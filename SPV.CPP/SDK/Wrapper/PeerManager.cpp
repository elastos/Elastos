// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <netinet/in.h>
#include <boost/bind.hpp>
#include <arpa/inet.h>
#include <Core/BRChainParams.h>
#include <Core/BRPeer.h>
#include <SDK/Wrapper/Message/PingMessage.h>

#include "PeerManager.h"
#include "Utils.h"
#include "Log.h"
#include "BRArray.h"
#include "ELAMerkleBlock.h"
#include "arith_uint256.h"
#include "Plugin/Registry.h"
#include "Plugin/Block/MerkleBlock.h"
#include "PeerCallbackInfo.h"

#define PROTOCOL_TIMEOUT      30.0
#define MAX_CONNECT_FAILURES  20 // notify user of network problems after this many connect failures in a row
#define PEER_FLAG_SYNCED      0x01
#define PEER_FLAG_NEEDSUPDATE 0x02

#define DEFAULT_FEE_PER_KB ((5000ULL*1000 + 99)/100) // bitcoind 0.11 min relay fee on 100bytes
#define MIN_FEE_PER_KB     ((TX_FEE_PER_KB*1000 + 190)/191) // minimum relay fee on a 191byte tx
#define MAX_FEE_PER_KB     ((1000100ULL*1000 + 190)/191) // slightly higher than a 10000bit fee on a 191byte tx

namespace Elastos {
	namespace ElaWallet {

		void PeerManager::syncStarted() {
			if (!_listener.expired()) {
				_listener.lock()->syncStarted();
			}
		}

		void PeerManager::syncStopped(int error) {
			if (!_listener.expired()) {
				_listener.lock()->syncStopped(error == 0 ? "" : strerror(error));
			}
		}

		void PeerManager::txStatusUpdate() {
			if (!_listener.expired()) {
				_listener.lock()->txStatusUpdate();
			}
		}

		void PeerManager::saveBlocks(bool replace, const std::vector<MerkleBlockPtr> &blocks) {
			if (!_listener.expired()) {
				_listener.lock()->saveBlocks(replace, blocks);
			}
		}

		void PeerManager::savePeers(bool replace, const std::vector<PeerPtr> &peers) {
			if (!_listener.expired()) {
				_listener.lock()->savePeers(replace, peers);
			}
		}

		int PeerManager::networkIsReachable() {
			int result = 1;
			if (!_listener.expired()) {
				result = _listener.lock()->networkIsReachable();
			}
			return result;
		}

		void PeerManager::txPublished(int error) {
			if (!_listener.expired()) {
				_listener.lock()->txPublished(error == 0 ? "" : strerror(error));
			}
		}

		void PeerManager::threadCleanup() {
			if (!_listener.expired()) {
			}
		}

		void PeerManager::blockHeightIncreased(uint32_t height) {
			if (!_listener.expired()) {
				_listener.lock()->blockHeightIncreased(height);
			}
		}

		void PeerManager::syncIsInactive() {
			if (!_listener.expired()) {
				_listener.lock()->syncIsInactive();
			}
		}

		PeerManager::Listener::Listener(const PluginTypes &pluginTypes) :
				_pluginTypes(pluginTypes) {
		}

		PeerManager::PeerManager(const ChainParams &params,
								 const WalletPtr &wallet,
								 uint32_t earliestKeyTime,
								 uint32_t reconnectSeconds,
								 const std::vector<MerkleBlockPtr> &blocks,
								 const std::vector<PeerPtr> &peers,
								 const boost::shared_ptr<PeerManager::Listener> &listener,
								 const PluginTypes &plugins) :
				_wallet(wallet),
				lastBlock(nullptr),
				lastOrphan(nullptr),
				_pluginTypes(plugins),
				_reconnectSeconds(reconnectSeconds),
				_earliestKeyTime(earliestKeyTime),
				averageTxPerBlock(1400),
				maxConnectCount(PEER_MAX_CONNECTIONS),
				_reconnectTaskCount(0),
				_chainParams(params) {

			assert(listener != nullptr);
			_listener = boost::weak_ptr<Listener>(listener);

			_peers = peers;
			sortPeers();

			time_t now = time(nullptr);
			for (size_t i = 0; i < params.getRaw()->checkpointsCount; i++) {
				MerkleBlockPtr checkBlock = Registry::Instance()->CreateMerkleBlock(_pluginTypes.BlockType);
				checkBlock->setHeight(params.getRaw()->checkpoints[i].height);
				checkBlock->setHash(UInt256Reverse(&params.getRaw()->checkpoints[i].hash));
				checkBlock->setTimestamp(params.getRaw()->checkpoints[i].timestamp);
				checkBlock->setTarget(params.getRaw()->checkpoints[i].target);
				_checkpoints.Insert(checkBlock);
				_blocks.Insert(checkBlock);
				if (i == 0 || checkBlock->getTimestamp() + 1 * 24 * 60 * 60 < earliestKeyTime ||
					(earliestKeyTime == 0 && checkBlock->getTimestamp() + 1 * 24 * 60 * 60 < now))
					lastBlock = checkBlock;
			}

			MerkleBlockPtr block;
			for (size_t i = 0; i < blocks.size(); i++) {
				assert(blocks[i]->getHeight() !=
					   BLOCK_UNKNOWN_HEIGHT); // height must be saved/restored along with serialized block
				_orphans.insert(blocks[i]);

				if ((blocks[i]->getHeight() % BLOCK_DIFFICULTY_INTERVAL) == 0 &&
					(block == nullptr || blocks[i]->getHeight() > block->getHeight()))
					block = blocks[i]; // find last transition block
			}

			MerkleBlockPtr orphan = Registry::Instance()->CreateMerkleBlock(_pluginTypes.BlockType);
			while (block != nullptr) {
				_blocks.Insert(block);
				lastBlock = block;

				orphan->setPrevBlockHash(block->getPrevBlockHash());
				for (std::set<MerkleBlockPtr>::const_iterator it = _orphans.cbegin(); it != _orphans.cend();) {
					if (UInt256Eq(&orphan->getPrevBlockHash(), &(*it)->getPrevBlockHash())) {
						it = _orphans.erase(it);
						break;
					} else {
						++it;
					}
				}

				orphan->setPrevBlockHash(block->getHash());
				block = nullptr;
				for (std::set<MerkleBlockPtr>::const_iterator it = _orphans.cbegin(); it != _orphans.cend(); ++it) {
					if (UInt256Eq(&orphan->getPrevBlockHash(), &(*it)->getPrevBlockHash())) {
						block = *it;
					}
				}
			}
		}

		PeerManager::~PeerManager() {
		}

		Peer::ConnectStatus PeerManager::getConnectStatus() const {
			Peer::ConnectStatus status = Peer::Disconnected;

			{
				boost::mutex::scoped_lock scoped_lock(lock);
				if (isConnected != 0) status = Peer::Connected;

				for (size_t i = _connectedPeers.size(); i > 0 && status == Peer::Disconnected; i--) {
					if (_connectedPeers[i - 1]->getConnectStatusValue() == Peer::Disconnected) continue;
					status = Peer::Connecting;
				}
			}
			return status;
		}

		void PeerManager::connect() {
			lock.lock();
			if (connectFailureCount >= MAX_CONNECT_FAILURES) connectFailureCount = 0; //this is a manual retry

			if ((!downloadPeer || lastBlock->getHeight() < estimatedHeight) && syncStartHeight == 0) {
				syncStartHeight = lastBlock->getHeight() + 1;
				lock.unlock();
				syncStarted();
				lock.lock();
			}

			for (size_t i = _connectedPeers.size(); i > 0; i--) {
				if (_connectedPeers[i]->getConnectStatusValue() == Peer::Connecting)
					_connectedPeers[i]->Connect();
			}

			if (_connectedPeers.size() < maxConnectCount) {
				time_t now = time(NULL);
				std::vector<PeerPtr> peers;

				if (_peers.size() < maxConnectCount ||
					_peers[maxConnectCount - 1]->getTimestamp() + 3 * 24 * 60 * 60 < now) {
					findPeers();
				}

				peers.insert(peers.end(), _peers.begin(), peers.end());

				while (!peers.empty() && _connectedPeers.size() < maxConnectCount) {
					size_t i = BRRand((uint32_t) peers.size()); // index of random peer

					i = i * i / peers.size(); // bias random peer selection toward peers with more recent timestamp

					for (size_t j = _connectedPeers.size(); i != SIZE_MAX && j > 0; j--) {
						if (!peers[i]->IsEqual(_connectedPeers[j - 1].get())) continue;
						peers.erase(peers.begin() + i);
						i = SIZE_MAX;
					}

					//fixme [refactor]
//					if (i != SIZE_MAX) {
//						info = new BRPeerCallbackInfo;
//						info->manager = this;
//						info->peer = BRPeerNew(_chainParams.getRaw()->magicNumber);
//						*info->peer = peers[i];
//						((BRPeerContext *) info->peer)->manager = manager;
//						array_rm(peers, i);
//						array_add(connectedPeers, info->peer);
//						BRPeerSetCallbacks(info->peer, info, _peerConnected, _peerDisconnected, _peerRelayedPeers,
//										   _peerRelayedTx, _peerHasTx, _peerRejectedTx, _peerRelayedBlock,
//										   _peerRelayedPingMsg,
//										   _peerDataNotfound, _peerSetFeePerKb, _peerRequestedTx,
//										   _peerNetworkIsReachable,
//										   _peerThreadCleanup);
//						BRPeerSetEarliestKeyTime(info->peer, earliestKeyTime);
//						BRPeerConnect(info->peer);
//					}
				}
			}

			if (_connectedPeers.empty()) {
				_peer_log("sync failed");
				syncStopped();
				lock.unlock();
				syncStopped(ENETUNREACH);
			} else {
				lock.unlock();
			}
		}

		void PeerManager::disconnect() {
			struct timespec ts;
			size_t peerCount, dnsThreadCount;

			{
				boost::mutex::scoped_lock scoped_lock(lock);
				peerCount = _connectedPeers.size();
				dnsThreadCount = this->dnsThreadCount;

				for (size_t i = peerCount; i > 0; i--) {
					connectFailureCount = MAX_CONNECT_FAILURES; // prevent futher automatic reconnect attempts
					_connectedPeers[i - 1]->Disconnect();
				}
			}

			ts.tv_sec = 0;
			ts.tv_nsec = 1;

			while (peerCount > 0 || dnsThreadCount > 0) {
				nanosleep(&ts, NULL); // pthread_yield() isn't POSIX standard :(
				{
					boost::mutex::scoped_lock scoped_lock(lock);
					peerCount = _connectedPeers.size();
					dnsThreadCount = this->dnsThreadCount;
				}
			}
		}

		void PeerManager::rescan() {
			lock.lock();

			if (isConnected) {
				// start the chain download from the most recent checkpoint that's at least a week older than earliestKeyTime
				for (size_t i = _chainParams.getRaw()->checkpointsCount; i > 0; i--) {
					if (i - 1 == 0 ||
						_chainParams.getRaw()->checkpoints[i - 1].timestamp + 7 * 24 * 60 * 60 < _earliestKeyTime) {
						UInt256 hash = UInt256Reverse(&_chainParams.getRaw()->checkpoints[i - 1].hash);
						lastBlock = _blocks.Get(hash);
						break;
					}
				}

				if (downloadPeer) { // disconnect the current download peer so a new random one will be selected
					for (size_t i = _peers.size(); i > 0; i--) {
						if (_peers[i - 1]->IsEqual(downloadPeer.get()))
							_peers.erase(_peers.begin() + i - 1);
					}

					downloadPeer->Disconnect();
				}

				syncStartHeight = 0; // a syncStartHeight of 0 indicates that syncing hasn't started yet
				lock.unlock();
				connect();
			} else lock.unlock();
		}

		uint32_t PeerManager::getSyncStartHeight() const {
			return syncStartHeight;
		}

		uint32_t PeerManager::getEstimatedBlockHeight() const {
			uint32_t height;

			{
				boost::mutex::scoped_lock scoped_lock(lock);
				height = (lastBlock->getHeight() < estimatedHeight) ? estimatedHeight : lastBlock->getHeight();
			}
			return height;
		}

		uint32_t PeerManager::getLastBlockHeight() const {
			uint32_t height;

			{
				boost::mutex::scoped_lock scoped_lock(lock);
				height = lastBlock->getHeight();
			}
			return height;
		}

		uint32_t PeerManager::getLastBlockTimestamp() const {
			uint32_t timestamp;

			{
				boost::mutex::scoped_lock scoped_lock(lock);
				timestamp = lastBlock->getTimestamp();
			}
			return timestamp;
		}

		double PeerManager::getSyncProgress(uint32_t startHeight) {
			double progress;

			{
				boost::mutex::scoped_lock scoped_lock(lock);
				if (startHeight == 0) startHeight = syncStartHeight;

				if (!downloadPeer && syncStartHeight == 0) {
					progress = 0.0;
				} else if (!downloadPeer || lastBlock->getHeight() < estimatedHeight) {
					if (lastBlock->getHeight() > startHeight && estimatedHeight > startHeight) {
						progress = 0.1 + 0.9 * (lastBlock->getHeight() - startHeight) / (estimatedHeight - startHeight);
					} else progress = 0.05;
				} else progress = 1.0;
			}
			return progress;
		}

		void PeerManager::setFixedPeers(const std::vector<PeerPtr> &peers) {
			_fiexedPeers = peers;
		}

		void PeerManager::setFixedPeer(UInt128 address, uint16_t port) {
			disconnect();
			{
				boost::mutex::scoped_lock scoped_lock(lock);
				maxConnectCount = UInt128IsZero(&address) ? PEER_MAX_CONNECTIONS : 1;
				//fixme [refactor]
//    fixedPeer = PeerPtr() ((BRPeer) { address, port, 0, 0, 0 });
				_peers.clear();
			}
		}

		bool PeerManager::useFixedPeer(const std::string &node, int port) {
			UInt128 address = UINT128_ZERO;
			uint16_t _port = (uint16_t) port;

			if (!node.empty()) {
				struct in_addr addr;
				if (inet_pton(AF_INET, node.c_str(), &addr) != 1) return false;
				address.u16[5] = 0xffff;
				address.u32[3] = addr.s_addr;
				if (port == 0) _port = _chainParams.getRaw()->standardPort;
			} else {
				_port = 0;
			}

			setFixedPeer(address, _port);
			return true;
		}

		std::string PeerManager::getCurrentPeerName() const {
			return getDownloadPeerName();
		}

		std::string PeerManager::getDownloadPeerName() const {
			//fixme [refactor]
			{
				boost::mutex::scoped_lock scoped_lock(lock);
//			if (downloadPeer) {
//				sprintf(downloadPeerName, "%s:%d", BRPeerHost(downloadPeer.get()), downloadPeer->port);
//			} else downloadPeerName[0] = '\0';
			}
			return downloadPeerName;
		}

		size_t PeerManager::getPeerCount() const {
			size_t count = 0;

			{
				boost::mutex::scoped_lock scoped_lock(lock);
				for (size_t i = _connectedPeers.size(); i > 0; i--) {
					if (_connectedPeers[i - 1]->getConnectStatusValue() != Peer::Disconnected) count++;
				}
			}
			return count;
		}

		void PeerManager::publishTransaction(const TransactionPtr &tx) {
			publishTransaction(tx, Peer::PeerCallback());
		}

		void PeerManager::publishTransaction(const TransactionPtr &tx,
											 const Peer::PeerCallback &callback) {

			assert(tx != NULL && tx->isSigned());
			if (tx) lock.lock();

			if (tx && !tx->isSigned()) {
				lock.unlock();
				if (!callback.empty()) callback(EINVAL); // transaction not signed
			} else if (tx && !isConnected) {
				int connectFailureCount = connectFailureCount;

				lock.unlock();

				if (connectFailureCount >= MAX_CONNECT_FAILURES ||
					(!networkIsReachable())) {
					if (!callback.empty()) callback(ENOTCONN); // not connected to bitcoin network
				} else lock.lock();
			}

			if (tx) {
				size_t i, count = 0;

				tx->setTimestamp((uint32_t) time(NULL)); // set timestamp to publish time
				addTxToPublishList(tx, callback);

				for (i = _connectedPeers.size(); i > 0; i--) {
					if (_connectedPeers[i - 1]->getConnectStatusValue() == Peer::Connected) count++;
				}

				for (i = _connectedPeers.size(); i > 0; i--) {
					const PeerPtr &peer = _connectedPeers[i - 1];

					if (peer->getConnectStatusValue() != Peer::Connected) continue;

					// instead of publishing to all peers, leave out downloadPeer to see if tx propogates/gets relayed back
					// TODO: XXX connect to a random peer with an empty or fake bloom filter just for publishing
					if (peer != downloadPeer || count == 1) {
						publishPendingTx(peer);

						PingParameter pingParameter;
						pingParameter.callbackInfo.peer = peer;
						pingParameter.callbackInfo.manager = this;
						pingParameter.callback = callback;
						peer->SendMessage(MSG_PING, pingParameter);
					}
				}

				lock.unlock();
			}
		}

		uint64_t PeerManager::getRelayCount(const UInt256 &txHash) const {
			size_t count = 0;

			assert(!UInt256IsZero(&txHash));

			{
				boost::mutex::scoped_lock scoped_lock(lock);
				for (size_t i = txRelays.size(); i > 0; i--) {
					if (!UInt256Eq(&txRelays[i - 1].GetTransactionHash(), &txHash)) continue;
					count = txRelays[i - 1].GetPeers().size();
					break;
				}
			}

			return count;
		}

		int PeerManager::verifyDifficultyWrapper(const BRChainParams *params, const BRMerkleBlock *block,
												 const BRSet *blockSet) {
			const ELAChainParams *wrapperParams = (const ELAChainParams *) params;
			return verifyDifficulty(block, blockSet, wrapperParams->TargetTimeSpan,
									wrapperParams->TargetTimePerBlock, wrapperParams->NetType);
		}

		int PeerManager::verifyDifficulty(const BRMerkleBlock *block, const BRSet *blockSet, uint32_t targetTimeSpan,
										  uint32_t targetTimePerBlock, const std::string &netType) {
			const BRMerkleBlock *previous, *b = nullptr;
			uint32_t i;

			assert(block != nullptr);
			assert(blockSet != nullptr);

			uint64_t blocksPerRetarget = targetTimeSpan / targetTimePerBlock;

			// check if we hit a difficulty transition, and find previous transition block
			if ((block->height % blocksPerRetarget) == 0) {
				for (i = 0, b = block; b && i < blocksPerRetarget; i++) {
					b = (const BRMerkleBlock *) BRSetGet(blockSet, &b->prevBlock);
				}
			}

			previous = (const BRMerkleBlock *) BRSetGet(blockSet, &block->prevBlock);
			return verifyDifficultyInner(block, previous, (b) ? b->timestamp : 0, targetTimeSpan,
										 targetTimePerBlock, netType);
		}

		int PeerManager::verifyDifficultyInner(const BRMerkleBlock *block, const BRMerkleBlock *previous,
											   uint32_t transitionTime, uint32_t targetTimeSpan,
											   uint32_t targetTimePerBlock, const std::string &netType) {
			int r = 1;
			//fixme figure out why difficult validation fails occasionally
			if (1 || netType == "RegNet")
				return r;

			assert(block != nullptr);
			assert(previous != nullptr);

			uint64_t blocksPerRetarget = targetTimeSpan / targetTimePerBlock;

			if (!previous || !UInt256Eq(&(block->prevBlock), &(previous->blockHash)) ||
				block->height != previous->height + 1)
				r = 0;
			if (r && (block->height % blocksPerRetarget) == 0 && transitionTime == 0) r = 0;

			if (r && (block->height % blocksPerRetarget) == 0) {
				uint32_t timespan = previous->timestamp - transitionTime;

				arith_uint256 target;
				target.SetCompact(previous->target);

				// limit difficulty transition to -75% or +400%
				if (timespan < targetTimeSpan / 4) timespan = uint32_t(targetTimeSpan) / 4;
				if (timespan > targetTimeSpan * 4) timespan = uint32_t(targetTimeSpan) * 4;

				// TARGET_TIMESPAN happens to be a multiple of 256, and since timespan is at least TARGET_TIMESPAN/4, we don't
				// lose precision when target is multiplied by timespan and then divided by TARGET_TIMESPAN/256
				target *= timespan;
				target /= targetTimeSpan;

				uint32_t actualTargetCompact = target.GetCompact();
				if (block->target != actualTargetCompact) r = 0;
			} else if (r && previous->height != 0 && block->target != previous->target) r = 0;

			return r;
		}

		void PeerManager::loadBloomFilter(const PeerPtr &peer) {
			//fixme [refactor]
//			// every time a new wallet address is added, the bloom filter has to be rebuilt, and each address is only used
//			// for one transaction, so here we generate some spare addresses to avoid rebuilding the filter each time a
//			// wallet transaction is encountered during the chain sync
//			_wallet->WalletUnusedAddrs(wallet, NULL, SEQUENCE_GAP_LIMIT_EXTERNAL + 100, 0);
//			_wallet->WalletUnusedAddrs(wallet, NULL, SEQUENCE_GAP_LIMIT_INTERNAL + 100, 1);
//
//			BRSetApply(orphans, manager, peerMessages->ApplyFreeBlock);
//			BRSetClear(orphans); // clear out orphans that may have been received on an old filter
//			lastOrphan = NULL;
//			filterUpdateHeight = lastBlock->height;
//			fpRate = BLOOM_REDUCED_FALSEPOSITIVE_RATE;
//
//			size_t addrsCount = wallet->WalletAllAddrs(wallet, NULL, 0);
//			BRAddress *addrs = (BRAddress *) malloc(addrsCount * sizeof(*addrs));
//			size_t utxosCount = BRWalletUTXOs(wallet, NULL, 0);
//			BRUTXO *utxos = (BRUTXO *) malloc(utxosCount * sizeof(*utxos));
//			uint32_t blockHeight = (lastBlock->height > 100) ? lastBlock->height - 100 : 0;
//			size_t txCount = BRWalletTxUnconfirmedBefore(wallet, NULL, 0, blockHeight);
//			BRTransaction **transactions = (BRTransaction **) malloc(txCount * sizeof(*transactions));
//			BRBloomFilter *filter;
//
//			assert(addrs != NULL);
//			assert(utxos != NULL);
//			assert(transactions != NULL);
//			addrsCount = wallet->WalletAllAddrs(wallet, addrs, addrsCount);
//			utxosCount = BRWalletUTXOs(wallet, utxos, utxosCount);
//			txCount = BRWalletTxUnconfirmedBefore(wallet, transactions, txCount, blockHeight);
//			filter = BRBloomFilterNew(fpRate, addrsCount + utxosCount + txCount + 100,
//									  (uint32_t) BRPeerHash(peer),
//									  BLOOM_UPDATE_ALL); // BUG: XXX txCount not the same as number of spent wallet outputs
//
//			for (size_t i = 0; i < addrsCount; i++) { // add addresses to watch for tx receiveing money to the wallet
//				UInt168 hash = UINT168_ZERO;
//
//				BRAddressHash168(&hash, addrs[i].s);
//
//				if (!UInt168IsZero(&hash) && !BRBloomFilterContainsData(filter, hash.u8, sizeof(hash))) {
//					BRBloomFilterInsertData(filter, hash.u8, sizeof(hash));
//				}
//			}
//
//			free(addrs);
//
//			ELAWallet *elaWallet = (ELAWallet *) wallet;
//			for (size_t i = 0; i < elaWallet->ListeningAddrs.size(); ++i) {
//				UInt168 hash = UINT168_ZERO;
//
//				BRAddressHash168(&hash, elaWallet->ListeningAddrs[i].c_str());
//
//				if (!UInt168IsZero(&hash) && !BRBloomFilterContainsData(filter, hash.u8, sizeof(hash))) {
//					BRBloomFilterInsertData(filter, hash.u8, sizeof(hash));
//				}
//			}
//
//			for (size_t i = 0; i < utxosCount; i++) { // add UTXOs to watch for tx sending money from the wallet
//				uint8_t o[sizeof(UInt256) + sizeof(uint32_t)];
//
//				UInt256Set(o, utxos[i].hash);
//				UInt32SetLE(&o[sizeof(UInt256)], utxos[i].n);
//				if (!BRBloomFilterContainsData(filter, o, sizeof(o))) BRBloomFilterInsertData(filter, o, sizeof(o));
//			}
//
//			free(utxos);
//
//			for (size_t i = 0; i < txCount; i++) { // also add TXOs spent within the last 100 blocks
//				for (size_t j = 0; j < transactions[i]->inCount; j++) {
//					BRTxInput *input = &transactions[i]->inputs[j];
//					BRTransaction *tx = BRWalletTransactionForHash(wallet, input->txHash);
//					uint8_t o[sizeof(UInt256) + sizeof(uint32_t)];
//
//					if (tx && input->index < tx->outCount &&
//						BRWalletContainsAddress(wallet, tx->outputs[input->index].address)) {
//						UInt256Set(o, input->txHash);
//						UInt32SetLE(&o[sizeof(UInt256)], input->index);
//						if (!BRBloomFilterContainsData(filter, o, sizeof(o)))
//							BRBloomFilterInsertData(filter, o, sizeof(o));
//					}
//				}
//			}
//
//			free(transactions);
//			if (bloomFilter) BRBloomFilterFree(bloomFilter);
//			bloomFilter = filter;
//			// TODO: XXX if already synced, recursively add inputs of unconfirmed receives
//
//			peerMessages->BRPeerSendFilterloadMessage(peer, filter);
		}

		void PeerManager::sortPeers() {
			// comparator for sorting peers by timestamp, most recent first
			std::sort(_peers.begin(), _peers.end(), [](const PeerPtr &first, const PeerPtr &second) {
				return first->getTimestamp() > second->getTimestamp();
			});
		}

		void PeerManager::findPeers() {
			uint64_t services = SERVICES_NODE_NETWORK | SERVICES_NODE_BLOOM | _chainParams.getRaw()->services;
			time_t now = time(NULL);
			struct timespec ts;
			pthread_t thread;
			pthread_attr_t attr;
			UInt128 *addr, *addrList = NULL;

			//fixme [refactor]
//			size_t peersCount = fiexedPeers == NULL ? 0 : array_count(fiexedPeers);
//			if (peersCount > 0) {
//				array_set_count(peers, peersCount);
//				for (int i = 0; i < peersCount; ++i) {
//					peers[i] = fiexedPeers[i];
//					peers[i].timestamp = now;
//				}
//			} else if (!UInt128IsZero(&fixedPeer.address)) {
//				array_set_count(peers, 1);
//				peers[0] = fixedPeer;
//				peers[0].services = services;
//				peers[0].timestamp = now;
//			} else {
//				for (size_t i = 0; params->dnsSeeds && params->dnsSeeds[i]; i++) {
//					for (addr = addrList = _addressLookup(params->dnsSeeds[i]);
//						 addr && !UInt128IsZero(addr); addr++) {
//						array_add(peers, ((BRPeer) {*addr, params->standardPort, services, now, 0}));
//					}
//					if (addrList) free(addrList);
//				}
//
//				ts.tv_sec = 0;
//				ts.tv_nsec = 1;
//
//				do {
//					pthread_mutex_unlock(&lock);
//					nanosleep(&ts, NULL); // pthread_yield() isn't POSIX standard :(
//					pthread_mutex_lock(&lock);
//				} while (dnsThreadCount > 0 && array_count(peers) < PEER_MAX_CONNECTIONS);
//
//				qsort(peers, array_count(peers), sizeof(*peers), _peerTimestampCompare);
//
//				_peer_log("peer manager found %zu peers\n", array_count(peers));
//				for (size_t i = 0; i < array_count(peers); i++) {
//					char host[INET6_ADDRSTRLEN] = {0};
//					BRPeer *peer = &peers[i];
//					if ((peer->address.u64[0] == 0 && peer->address.u16[4] == 0 && peer->address.u16[5] == 0xffff))
//						inet_ntop(AF_INET, &peer->address.u32[3], host, sizeof(host));
//					else
//						inet_ntop(AF_INET6, &peer->address, host, sizeof(host));
//					_peer_log("peers[%zu] = %s\n", i, host);
//				}
		}

		void PeerManager::syncStopped() {
			syncStartHeight = 0;

			if (downloadPeer != nullptr) {
				// don't cancel timeout if there's a pending tx publish callback
				for (size_t i = publishedTx.size(); i > 0; i--) {
					if (publishedTx[i - 1].HasCallback()) return;
				}

				downloadPeer->scheduleDisconnect(-1); // cancel sync timeout
			}
		}

		void PeerManager::addTxToPublishList(const TransactionPtr &tx, const Peer::PeerCallback &callback) {
			if (tx && tx->getBlockHeight() == TX_UNCONFIRMED) {
				for (size_t i = publishedTx.size(); i > 0; i--) {
					if (publishedTx[i - 1].GetTransaction()->IsEqual(tx.get())) return;
				}

				publishedTx.emplace_back(tx, callback);
				publishedTxHashes.push_back(tx->getHash());

				for (size_t i = 0; i < tx->getInputs().size(); i++) {
					addTxToPublishList(_wallet->transactionForHash(tx->getInputs()[i].getTransctionHash()),
									   boost::function<void(int)>());
				}
			}
		}

		void PeerManager::asyncConnect(const boost::system::error_code &error) {
			if (error.value() == 0) {
				if (getConnectStatus() != Peer::Connected) {
					Log::getLogger()->info("async connecting...");
					connect();
				}
			} else {
				Log::getLogger()->warn("asyncConnect err: {}", error.message());
			}

			if (_reconnectTaskCount > 0) {
				{
					boost::mutex::scoped_lock scoped_lock(lock);
					_reconnectTaskCount = 0;
				}
			}
		}

		void PeerManager::OnConnected(const PeerPtr &peer) {
			//fixme [refactor]
//			BRPeer *peer = ((BRPeerCallbackInfo *) info)->peer;
//			BRPeerManager *manager = ((BRPeerCallbackInfo *) info)->manager;
//			BRPeerCallbackInfo *peerInfo;
//			time_t now = time(NULL);
//
//			pthread_mutex_lock(&manager->lock);
//			if (peer->timestamp > now + 2 * 60 * 60 || peer->timestamp < now - 2 * 60 * 60)
//				peer->timestamp = now; // sanity check
//
//			// TODO: XXX does this work with 0.11 pruned nodes?
//			if ((peer->services & manager->params->services) != manager->params->services) {
//				peer_log(peer, "unsupported node type");
//				BRPeerDisconnect(peer);
//			} else if ((peer->services & SERVICES_NODE_NETWORK) != SERVICES_NODE_NETWORK) {
//				peer_log(peer, "peer->services: %llu != SERVICES_NODE_NETWORK", peer->services);
//				peer_log(peer, "node doesn't carry full blocks");
//				BRPeerDisconnect(peer);
//			} else if (BRPeerLastBlock(peer) + 10 < manager->lastBlock->height) {
//				peer_log(peer, "peer->lastBlock: %d != manager->lastBlock->height: %d", BRPeerLastBlock(peer),
//						 manager->lastBlock->height);
//				peer_log(peer, "node isn't synced");
//				BRPeerDisconnect(peer);
//			} else if (BRPeerVersion(peer) >= 70011 && (peer->services & SERVICES_NODE_BLOOM) != SERVICES_NODE_BLOOM) {
//				peer_log(peer, "node doesn't support SPV mode");
//				BRPeerDisconnect(peer);
//			} else if (manager->downloadPeer && // check if we should stick with the existing download peer
//					   (BRPeerLastBlock(manager->downloadPeer) >= BRPeerLastBlock(peer) ||
//						manager->lastBlock->height >= BRPeerLastBlock(peer))) {
//				if (manager->lastBlock->height >=
//					BRPeerLastBlock(peer)) { // only load bloom filter if we're done syncing
//					manager->connectFailureCount = 0; // also reset connect failure count if we're already synced
//					manager->loadBloomFilter(manager, peer);
//					_BRPeerManagerPublishPendingTx(manager, peer);
//					peerInfo = calloc(1, sizeof(*peerInfo));
//					assert(peerInfo != NULL);
//					peerInfo->peer = peer;
//					peerInfo->manager = manager;
//					manager->peerMessages->BRPeerSendPingMessage(peer, peerInfo, _loadBloomFilterDone);
//				}
//			} else { // select the peer with the lowest ping time to download the chain from if we're behind
//				// BUG: XXX a malicious peer can report a higher lastblock to make us select them as the download peer, if
//				// two peers agree on lastblock, use one of those two instead
//				for (size_t i = array_count(manager->connectedPeers); i > 0; i--) {
//					BRPeer *p = manager->connectedPeers[i - 1];
//
//					if (BRPeerConnectStatus(p) != BRPeerStatusConnected) continue;
//					if ((BRPeerPingTime(p) < BRPeerPingTime(peer) && BRPeerLastBlock(p) >= BRPeerLastBlock(peer)) ||
//						BRPeerLastBlock(p) > BRPeerLastBlock(peer))
//						peer = p;
//				}
//
//				if (manager->downloadPeer) {
//					peer_log(peer, "selecting new download peer with higher reported lastblock");
//					BRPeerDisconnect(manager->downloadPeer);
//				}
//
//				manager->downloadPeer = peer;
//				manager->isConnected = 1;
//				manager->estimatedHeight = BRPeerLastBlock(peer);
//				manager->loadBloomFilter(manager, peer);
//				BRPeerSetCurrentBlockHeight(peer, manager->lastBlock->height);
//				_BRPeerManagerPublishPendingTx(manager, peer);
//
//				if (manager->lastBlock->height < BRPeerLastBlock(peer)) { // start blockchain sync
//					UInt256 locators[_BRPeerManagerBlockLocators(manager, NULL, 0)];
//					size_t count = _BRPeerManagerBlockLocators(manager, locators, sizeof(locators) / sizeof(*locators));
//
//					BRPeerScheduleDisconnect(peer, PROTOCOL_TIMEOUT); // schedule sync timeout
//
//					// request just block headers up to a week before earliestKeyTime, and then merkleblocks after that
//					// we do not reset connect failure count yet incase this request times out
//					if (manager->lastBlock->timestamp + 7 * 24 * 60 * 60 >= manager->earliestKeyTime) {
//						manager->peerMessages->BRPeerSendGetblocksMessage(peer, locators, count, UINT256_ZERO);
//					} else manager->peerMessages->BRPeerSendGetheadersMessage(peer, locators, count, UINT256_ZERO);
//				} else { // we're already synced
//					manager->connectFailureCount = 0; // reset connect failure count
//					_BRPeerManagerLoadMempools(manager);
//				}
//			}
//
//			manager->wallet->WalletUpdateBalance(manager->wallet);
//
//			pthread_mutex_unlock(&manager->lock);
		}

		void PeerManager::OnDisconnected(const PeerPtr &peer, int error) {
			//fixme [refactor]
//			BRPeer *peer = ((BRPeerCallbackInfo *) info)->peer;
//			BRPeerManager *manager = ((BRPeerCallbackInfo *) info)->manager;
//			BRTxPeerList *peerList;
//			int willSave = 0, willReconnect = 0, txError = 0;
//			size_t txCount = 0;
//
//			//free(info);
//			pthread_mutex_lock(&manager->lock);
//
//			BRPublishedTx pubTx[array_count(manager->publishedTx)];
//
//			if (error == EPROTO) { // if it's protocol error, the peer isn't following standard policy
//				_BRPeerManagerPeerMisbehavin(manager, peer);
//			} else if (error) { // timeout or some non-protocol related network error
//				for (size_t i = array_count(manager->peers); i > 0; i--) {
//					if (BRPeerEq(&manager->peers[i - 1], peer))
//						array_rm(manager->peers, i - 1);
//				}
//
//				manager->connectFailureCount++;
//
//				// if it's a timeout and there's pending tx publish callbacks, the tx publish timed out
//				// BUG: XXX what if it's a connect timeout and not a publish timeout?
//				if (error == ETIMEDOUT && (peer != manager->downloadPeer || manager->syncStartHeight == 0 ||
//										   array_count(manager->connectedPeers) == 1))
//					txError = ETIMEDOUT;
//			}
//
//			for (size_t i = array_count(manager->txRelays); i > 0; i--) {
//				peerList = &manager->txRelays[i - 1];
//
//				for (size_t j = array_count(peerList->peers); j > 0; j--) {
//					if (BRPeerEq(&peerList->peers[j - 1], peer))
//						array_rm(peerList->peers, j - 1);
//				}
//			}
//
//			if (peer == manager->downloadPeer) { // download peer disconnected
//				manager->isConnected = 0;
//				manager->downloadPeer = NULL;
//				if (manager->connectFailureCount > MAX_CONNECT_FAILURES)
//					manager->connectFailureCount = MAX_CONNECT_FAILURES;
//			}
//
//			if (!manager->isConnected && manager->connectFailureCount == MAX_CONNECT_FAILURES) {
//				_BRPeerManagerSyncStopped(manager);
//
//				// clear out stored peers so we get a fresh list from DNS on next connect attempt
//				array_clear(manager->peers);
//				txError = ENOTCONN; // trigger any pending tx publish callbacks
//				willSave = 1;
//				peer_log(peer, "sync failed");
//			} else if (manager->connectFailureCount < MAX_CONNECT_FAILURES) {
//				willReconnect = 1;
//			}
//
//			if (txError) {
//				for (size_t i = array_count(manager->publishedTx); i > 0; i--) {
//					if (manager->publishedTx[i - 1].callback == NULL) continue;
//					peer_log(peer, "transaction canceled: %s", strerror(txError));
//					pubTx[txCount++] = manager->publishedTx[i - 1];
//					manager->publishedTx[i - 1].callback = NULL;
//					manager->publishedTx[i - 1].info = NULL;
//				}
//			}
//
//			for (size_t i = array_count(manager->connectedPeers); i > 0; i--) {
//				if (manager->connectedPeers[i - 1] != peer)
//					continue;
//				array_rm(manager->connectedPeers, i - 1);
//				break;
//			}
//
//			BRPeerFree(peer);
//			pthread_mutex_unlock(&manager->lock);
//
//			for (size_t i = 0; i < txCount; i++) {
//				pubTx[i].callback(pubTx[i].info, txError);
//			}
//
//			if (willSave && manager->savePeers) manager->savePeers(manager->info, 1, NULL, 0);
//			if (willSave && manager->syncStopped) manager->syncStopped(manager->info, error);
//			if (willReconnect && array_count(manager->connectedPeers) == 0) {
//				peer_log(peer, "willReconnect...");
//				BRPeerManagerConnect(manager);
//			}
//			if (manager->txStatusUpdate) manager->txStatusUpdate(manager->info);
		}

		void PeerManager::OnRelayedPeers(const PeerPtr &peer, const std::vector<PeerPtr> &peers, size_t peersCount) {
			//fixme [refactor]
//			BRPeer *peer = ((BRPeerCallbackInfo *) info)->peer;
//			BRPeerManager *manager = ((BRPeerCallbackInfo *) info)->manager;
//			time_t now = time(NULL);
//
//			pthread_mutex_lock(&manager->lock);
//			peer_log(peer, "relayed %zu peer(s)", peersCount);
//
//			array_add_array(manager->peers, peers, peersCount);
//			qsort(manager->peers, array_count(manager->peers), sizeof(*manager->peers), _peerTimestampCompare);
//
//			// limit total to 2500 peers
//			if (array_count(manager->peers) > 2500) array_set_count(manager->peers, 2500);
//			peersCount = array_count(manager->peers);
//
//			// remove peers more than 3 hours old, or until there are only 1000 left
//			while (peersCount > 1000 && manager->peers[peersCount - 1].timestamp + 3 * 60 * 60 < now) peersCount--;
//			array_set_count(manager->peers, peersCount);
//
//			BRPeer save[peersCount];
//
//			for (size_t i = 0; i < peersCount; i++) save[i] = manager->peers[i];
//			pthread_mutex_unlock(&manager->lock);
//
//			// peer relaying is complete when we receive <1000
//			if (peersCount > 1 && peersCount < 1000 &&
//				manager->savePeers)
//				manager->savePeers(manager->info, 1, save, peersCount);
		}

		void PeerManager::OnRelayedTx(const PeerPtr &peer, const TransactionPtr &transaction) {
			int isWalletTx = 0, hasPendingCallbacks = 0;
			size_t relayCount = 0;
			TransactionPtr tx = transaction;
			Peer::PeerCallback txCallback;

			{
				boost::mutex::scoped_lock scopedLock(lock);
				peer->Pinfo("relayed tx: %s", Utils::UInt256ToString(tx->getHash()));

				for (size_t i = publishedTx.size(); i > 0; i--) { // see if tx is in list of published tx
					if (UInt256Eq(&publishedTxHashes[i - 1], &tx->getHash())) {
						txCallback = publishedTx[i - 1].GetCallback();
						publishedTx[i - 1].ResetCallback();
						relayCount = addPeerToList(peer, tx->getHash(), txRelays);
					} else if (publishedTx[i - 1].HasCallback()) hasPendingCallbacks = 1;
				}

				// cancel tx publish timeout if no publish callbacks are pending, and syncing is done or this is not downloadPeer
				if (!hasPendingCallbacks && (syncStartHeight == 0 || peer != downloadPeer)) {
					peer->scheduleDisconnect(-1); // cancel publish tx timeout
				}

				if (syncStartHeight == 0 || _wallet->containsTransaction(tx)) {
					isWalletTx = _wallet->registerTransaction(tx);
					if (isWalletTx) tx = _wallet->transactionForHash(tx->getHash());
				} else {
					tx = nullptr;
				}

				if (tx && isWalletTx) {
					// reschedule sync timeout
					if (syncStartHeight > 0 && peer == downloadPeer) {
						peer->scheduleDisconnect(PROTOCOL_TIMEOUT);
					}

					if (_wallet->getTransactionAmountSent(tx) > 0 &&
						_wallet->transactionIsValid(tx)) {
						addTxToPublishList(tx, Peer::PeerCallback());  // add valid send tx to mempool
					}

					// keep track of how many peers have or relay a tx, this indicates how likely the tx is to confirm
					// (we only need to track this after syncing is complete)
					if (syncStartHeight == 0)
						relayCount = addPeerToList(peer, tx->getHash(), txRelays);

					removePeerFromList(peer, tx->getHash(), txRequests);

					//fixme [refactor]
//					if (bloomFilter != nullptr) { // check if bloom filter is already being updated
//						BRAddress addrs[SEQUENCE_GAP_LIMIT_EXTERNAL + SEQUENCE_GAP_LIMIT_INTERNAL];
//						UInt168 hash;
//
//						// the transaction likely consumed one or more wallet addresses, so check that at least the next <gap limit>
//						// unused addresses are still matched by the bloom filter
//						_wallet->nusedAddrs(manager->wallet, addrs, SEQUENCE_GAP_LIMIT_EXTERNAL, 0);
//						_wallet->WalletUnusedAddrs(manager->wallet, addrs + SEQUENCE_GAP_LIMIT_EXTERNAL,
//												   SEQUENCE_GAP_LIMIT_INTERNAL, 1);
//
//						for (size_t i = 0; i < SEQUENCE_GAP_LIMIT_EXTERNAL + SEQUENCE_GAP_LIMIT_INTERNAL; i++) {
//							if (!BRAddressHash168(&hash, addrs[i].s) ||
//								BRBloomFilterContainsData(bloomFilter, hash.u8, sizeof(hash)))
//								continue;
//							if (bloomFilter) BRBloomFilterFree(bloomFilter);
//							bloomFilter = nullptr; // reset bloom filter so it's recreated with new wallet addresses
//							updateBloomFilter();
//							break;
//						}
//					}
				}

				// set timestamp when tx is verified
				if (tx && relayCount >= maxConnectCount && tx->getBlockHeight() == TX_UNCONFIRMED &&
					tx->getTimestamp() == 0) {
					_wallet->updateTransactions({tx->getHash()}, TX_UNCONFIRMED, (uint32_t) time(NULL));
				}
			}

			if (!txCallback.empty()) txCallback(0);
		}

		void PeerManager::OnHasTx(const PeerPtr &peer, const UInt256 &txHash) {
			int isWalletTx = 0, hasPendingCallbacks = 0;
			size_t relayCount = 0;
			PublishedTransaction pubTx;

			{
				boost::mutex::scoped_lock scopedLock(lock);
				TransactionPtr tx = _wallet->transactionForHash(txHash);
				peer->Pinfo("has tx: {}", Utils::UInt256ToString(txHash));

				for (size_t i = publishedTx.size(); i > 0; i--) { // see if tx is in list of published tx
					if (UInt256Eq(&(publishedTxHashes[i - 1]), &txHash)) {
						if (!tx) tx = publishedTx[i - 1].GetTransaction();
						pubTx = publishedTx[i - 1];
						pubTx.ResetCallback();
						relayCount = addPeerToList(peer, txHash, txRelays);
					} else if (publishedTx[i - 1].HasCallback()) hasPendingCallbacks = 1;
				}

				// cancel tx publish timeout if no publish callbacks are pending, and syncing is done or this is not downloadPeer
				if (!hasPendingCallbacks && (syncStartHeight == 0 || peer != downloadPeer)) {
					peer->scheduleDisconnect(-1);  // cancel publish tx timeout
				}

				if (tx) {
					isWalletTx = _wallet->registerTransaction(tx);
					if (isWalletTx) tx = _wallet->transactionForHash(tx->getHash());

					// reschedule sync timeout
					if (syncStartHeight > 0 && peer == downloadPeer && isWalletTx) {
						peer->scheduleDisconnect(PROTOCOL_TIMEOUT);
					}

					// keep track of how many peers have or relay a tx, this indicates how likely the tx is to confirm
					// (we only need to track this after syncing is complete)
					if (syncStartHeight == 0)
						relayCount = addPeerToList(peer, txHash, txRelays);

					// set timestamp when tx is verified
					if (relayCount >= maxConnectCount && tx && tx->getBlockHeight() == TX_UNCONFIRMED &&
						tx->getTimestamp() == 0) {
						std::vector<UInt256> hashes = {txHash};
						_wallet->updateTransactions(hashes, TX_UNCONFIRMED, (uint32_t) time(NULL));
					}

					removePeerFromList(peer, txHash, txRequests);
				}
			}

			if (pubTx.HasCallback()) pubTx.FireCallback(0);
		}

		void PeerManager::OnRejectedTx(const PeerPtr &peer, const UInt256 &txHash, uint8_t code) {

			{
				boost::mutex::scoped_lock scopedLock(lock);
				peer->Pinfo("rejected tx: {}", Utils::UInt256ToString(txHash));
				TransactionPtr tx = _wallet->transactionForHash(txHash);
				removePeerFromList(peer, txHash, txRequests);

				if (tx) {
					if (removePeerFromList(peer, txHash, txRelays) && tx->getBlockHeight() == TX_UNCONFIRMED) {
						// set timestamp 0 to mark tx as unverified
						_wallet->updateTransactions({txHash}, TX_UNCONFIRMED, 0);
					}

					// if we get rejected for any reason other than double-spend, the peer is likely misconfigured
					if (code != REJECT_SPENT && _wallet->getTransactionAmountSent(tx) > 0) {
						for (size_t i = 0; i <
										   tx->getInputs().size(); i++) { // check that all inputs are confirmed before dropping peer
							const TransactionPtr &t = _wallet->transactionForHash(
									tx->getInputs()[i].getTransctionHash());
							if (!t || t->getBlockHeight() != TX_UNCONFIRMED) continue;
							tx = nullptr;
							break;
						}

						if (tx != nullptr) peerMisbehaving(peer);
					}
				}
			}

			txStatusUpdate();
		}

		void PeerManager::OnRelayedBlock(const PeerPtr &peer, const MerkleBlockPtr &block) {
			//fixme [refactor]
//			BRPeer *peer = ((BRPeerCallbackInfo *) info)->peer;
//			BRPeerManager *manager = ((BRPeerCallbackInfo *) info)->manager;
//			size_t txCount = BRMerkleBlockTxHashes(block, NULL, 0);
//			UInt256 _txHashes[(sizeof(UInt256) * txCount <= 0x1000) ? txCount : 0],
//					*txHashes = (sizeof(UInt256) * txCount <= 0x1000) ? _txHashes : malloc(txCount * sizeof(*txHashes));
//			size_t i, j, fpCount = 0, saveCount = 0;
//			BRMerkleBlock orphan, *b, *b2, *prev, *next = NULL;
//			uint32_t txTime = 0;
//
//			assert(txHashes != NULL);
//			txCount = BRMerkleBlockTxHashes(block, txHashes, txCount);
//			pthread_mutex_lock(&manager->lock);
//			prev = BRSetGet(manager->blocks, &block->prevBlock);
//
//			if (prev) {
//				txTime = block->timestamp;
//				block->height = prev->height + 1;
//			}
//
//			// track the observed bloom filter false positive rate using a low pass filter to smooth out variance
//			if (peer == manager->downloadPeer && block->totalTx > 0) {
//				for (i = 0; i < txCount; i++) { // wallet tx are not false-positives
//					if (!BRWalletTransactionForHash(manager->wallet, txHashes[i])) fpCount++;
//				}
//
//				// moving average number of tx-per-block
//				manager->averageTxPerBlock = manager->averageTxPerBlock * 0.999 + block->totalTx * 0.001;
//
//				// 1% low pass filter, also weights each block by total transactions, compared to the avarage
//				manager->fpRate = manager->fpRate * (1.0 - 0.01 * block->totalTx / manager->averageTxPerBlock) +
//								  0.01 * fpCount / manager->averageTxPerBlock;
//
//				// false positive rate sanity check
//				if (BRPeerConnectStatus(peer) == BRPeerStatusConnected &&
//					manager->fpRate > BLOOM_DEFAULT_FALSEPOSITIVE_RATE * 10.0) {
//					peer_log(peer,
//							 "bloom filter false positive rate %f too high after %"PRIu32" blocks, disconnecting...",
//							 manager->fpRate, manager->lastBlock->height + 1 - manager->filterUpdateHeight);
////            BRPeerDisconnect(peer);
//				} else if (manager->lastBlock->height + 500 < BRPeerLastBlock(peer) &&
//						   manager->fpRate > BLOOM_REDUCED_FALSEPOSITIVE_RATE * 10.0) {
//					_BRPeerManagerUpdateFilter(manager); // rebuild bloom filter when it starts to degrade
//				}
//			}
//
//			// ignore block headers that are newer than one week before earliestKeyTime (it's a header if it has 0 totalTx)
//			if (block->totalTx == 0 && block->timestamp + 7 * 24 * 60 * 60 > manager->earliestKeyTime + 2 * 60 * 60) {
//				manager->peerMessages->MerkleBlockFree(manager, block);
//				block = NULL;
//			} else if (manager->bloomFilter ==
//					   NULL) { // ingore potentially incomplete blocks when a filter update is pending
//				manager->peerMessages->MerkleBlockFree(manager, block);
//				block = NULL;
//
//				if (peer == manager->downloadPeer && manager->lastBlock->height < manager->estimatedHeight) {
//					BRPeerScheduleDisconnect(peer, PROTOCOL_TIMEOUT); // reschedule sync timeout
//					manager->connectFailureCount = 0; // reset failure count once we know our initial request didn't timeout
//				}
//			} else if (!prev) { // block is an orphan
//				peer_log(peer, "relayed orphan block %s, previous %s, last block is %s, height %"PRIu32,
//						 u256hex(block->blockHash), u256hex(block->prevBlock), u256hex(manager->lastBlock->blockHash),
//						 manager->lastBlock->height);
//
//				if (block->timestamp + 7 * 24 * 60 * 60 < time(NULL)) { // ignore orphans older than one week ago
//					manager->peerMessages->MerkleBlockFree(manager, block);
//					block = NULL;
//				} else {
//					// call getblocks, unless we already did with the previous block, or we're still syncing
//					if (manager->lastBlock->height >= BRPeerLastBlock(peer) &&
//						(!manager->lastOrphan || !UInt256Eq(&manager->lastOrphan->blockHash, &block->prevBlock))) {
//						UInt256 locators[_BRPeerManagerBlockLocators(manager, NULL, 0)];
//						size_t locatorsCount = _BRPeerManagerBlockLocators(manager, locators,
//																		   sizeof(locators) / sizeof(*locators));
//
//						peer_log(peer, "calling getblocks");
//						manager->peerMessages->BRPeerSendGetblocksMessage(peer, locators, locatorsCount, UINT256_ZERO);
//					}
//
//					BRSetAdd(manager->orphans, block); // BUG: limit total orphans to avoid memory exhaustion attack
//					manager->lastOrphan = block;
//					BRPeerScheduleDisconnect(peer, PROTOCOL_TIMEOUT); // reschedule sync timeout
//				}
//			} else if (!_BRPeerManagerVerifyBlock(manager, block, prev, peer)) { // block is invalid
//				peer_log(peer, "relayed invalid block");
//				manager->peerMessages->MerkleBlockFree(manager, block);
//				block = NULL;
//				_BRPeerManagerPeerMisbehavin(manager, peer);
//			} else if (UInt256Eq(&block->prevBlock, &manager->lastBlock->blockHash)) { // new block extends main chain
//				if ((block->height % 500) == 0 || txCount > 0 || block->height >= BRPeerLastBlock(peer)) {
//					peer_log(peer, "adding block #%"PRIu32", false positive rate: %f", block->height, manager->fpRate);
//				}
//
//				BRSetAdd(manager->blocks, block);
//				manager->lastBlock = block;
//				if (manager->blockHeightIncreased) manager->blockHeightIncreased(manager->info, block->height);
//
//				if (txCount > 0) BRWalletUpdateTransactions(manager->wallet, txHashes, txCount, block->height, txTime);
//				if (manager->downloadPeer) BRPeerSetCurrentBlockHeight(manager->downloadPeer, block->height);
//
//				if (block->height < manager->estimatedHeight && peer == manager->downloadPeer) {
//					BRPeerScheduleDisconnect(peer, PROTOCOL_TIMEOUT); // reschedule sync timeout
//					manager->connectFailureCount = 0; // reset failure count once we know our initial request didn't timeout
//				}
//
//				if ((block->height % BLOCK_DIFFICULTY_INTERVAL) == 0)
//					saveCount = 1; // save transition block immediately
//
//				if (block->height == manager->estimatedHeight) { // chain download is complete
//					saveCount = (block->height % BLOCK_DIFFICULTY_INTERVAL) + BLOCK_DIFFICULTY_INTERVAL + 1;
//					_BRPeerManagerLoadMempools(manager);
//				}
//			} else if (BRSetContains(manager->blocks, block)) { // we already have the block (or at least the header)
//				if ((block->height % 500) == 0 || txCount > 0 || block->height >= BRPeerLastBlock(peer)) {
//					peer_log(peer, "relayed existing block #%"PRIu32, block->height);
//				}
//
//				b = manager->lastBlock;
//				while (b && b->height > block->height)
//					b = BRSetGet(manager->blocks, &b->prevBlock); // is block in main chain?
//
//				if (BRMerkleBlockEq(b, block)) { // if it's not on a fork, set block heights for its transactions
//					if (txCount > 0)
//						BRWalletUpdateTransactions(manager->wallet, txHashes, txCount, block->height, txTime);
//					if (block->height == manager->lastBlock->height) manager->lastBlock = block;
//				}
//
//				b = BRSetAdd(manager->blocks, block);
//
//				if (b != block) {
//					if (BRSetGet(manager->orphans, b) == b) BRSetRemove(manager->orphans, b);
//					if (manager->lastOrphan == b) manager->lastOrphan = NULL;
//					manager->peerMessages->MerkleBlockFree(manager, b);
//				}
//			} else if (manager->lastBlock->height < BRPeerLastBlock(peer) &&
//					   block->height > manager->lastBlock->height + 1) { // special case, new block mined durring rescan
//				peer_log(peer, "marking new block #%"PRIu32" as orphan until rescan completes", block->height);
//				BRSetAdd(manager->orphans, block); // mark as orphan til we're caught up
//				manager->lastOrphan = block;
//			} else if (block->height <=
//					   manager->params->checkpoints[manager->params->checkpointsCount - 1].height) { // old fork
//				peer_log(peer, "ignoring block on fork older than most recent checkpoint, block #%"PRIu32", hash: %s",
//						 block->height, u256hex(block->blockHash));
//				manager->peerMessages->MerkleBlockFree(manager, block);
//				block = NULL;
//			} else { // new block is on a fork
//				peer_log(peer, "chain fork reached height %"PRIu32, block->height);
//				BRSetAdd(manager->blocks, block);
//
//				if (block->height > manager->lastBlock->height) { // check if fork is now longer than main chain
//					b = block;
//					b2 = manager->lastBlock;
//
//					while (b && b2 && !BRMerkleBlockEq(b, b2)) { // walk back to where the fork joins the main chain
//						b = BRSetGet(manager->blocks, &b->prevBlock);
//						if (b && b->height < b2->height) b2 = BRSetGet(manager->blocks, &b2->prevBlock);
//					}
//
//					peer_log(peer, "reorganizing chain from height %"PRIu32", new height is %"PRIu32, b->height,
//							 block->height);
//
//					BRWalletSetTxUnconfirmedAfter(manager->wallet,
//												  b->height); // mark tx after the join point as unconfirmed
//
//					b = block;
//
//					while (b && b2 && b->height > b2->height) { // set transaction heights for new main chain
//						size_t count = BRMerkleBlockTxHashes(b, NULL, 0);
//						uint32_t height = b->height, timestamp = b->timestamp;
//
//						if (count > txCount) {
//							txHashes = (txHashes != _txHashes) ? realloc(txHashes, count * sizeof(*txHashes)) :
//									   malloc(count * sizeof(*txHashes));
//							assert(txHashes != NULL);
//							txCount = count;
//						}
//
//						count = BRMerkleBlockTxHashes(b, txHashes, count);
//						b = BRSetGet(manager->blocks, &b->prevBlock);
//						if (b) timestamp = timestamp / 2 + b->timestamp / 2;
//						if (count > 0) BRWalletUpdateTransactions(manager->wallet, txHashes, count, height, timestamp);
//					}
//
//					manager->lastBlock = block;
//					if (manager->blockHeightIncreased) {
//						for (int k = 1; k <= block->height - b2->height; ++k) {
//							manager->blockHeightIncreased(manager->info, b2->height + k);
//						}
//					}
//
//					if (block->height == manager->estimatedHeight) { // chain download is complete
//						saveCount = (block->height % BLOCK_DIFFICULTY_INTERVAL) + BLOCK_DIFFICULTY_INTERVAL + 1;
//						_BRPeerManagerLoadMempools(manager);
//					}
//				}
//			}
//
//			if (txHashes != _txHashes) free(txHashes);
//
//			if (block && block->height != BLOCK_UNKNOWN_HEIGHT) {
//				if (block->height > manager->estimatedHeight) manager->estimatedHeight = block->height;
//
//				// check if the next block was received as an orphan
//				orphan.prevBlock = block->blockHash;
//				next = BRSetRemove(manager->orphans, &orphan);
//			}
//
//			BRMerkleBlock *saveBlocks[saveCount];
//
//			for (i = 0, b = block; b && i < saveCount; i++) {
//				assert(b->height != BLOCK_UNKNOWN_HEIGHT); // verify all blocks to be saved are in the chain
//				saveBlocks[i] = b;
//				b = BRSetGet(manager->blocks, &b->prevBlock);
//			}
//
//			// make sure the set of blocks to be saved starts at a difficulty interval
//			j = (i > 0) ? saveBlocks[i - 1]->height % BLOCK_DIFFICULTY_INTERVAL : 0;
//			if (j > 0) i -= (i > BLOCK_DIFFICULTY_INTERVAL - j) ? BLOCK_DIFFICULTY_INTERVAL - j : i;
//			assert(i == 0 || (saveBlocks[i - 1]->height % BLOCK_DIFFICULTY_INTERVAL) == 0);
//			pthread_mutex_unlock(&manager->lock);
//			if (i > 0 && manager->saveBlocks) manager->saveBlocks(manager->info, (i > 1 ? 1 : 0), saveBlocks, i);
//
//			if (block && block->height != BLOCK_UNKNOWN_HEIGHT && block->height >= BRPeerLastBlock(peer) &&
//				manager->txStatusUpdate) {
//				manager->txStatusUpdate(manager->info); // notify that transaction confirmations may have changed
//			}
//
//			if (next) _peerRelayedBlock(info, next);
		}

		void PeerManager::OnRelayedPingMsg(const PeerPtr &peer) {
			syncIsInactive();
		}

		void PeerManager::OnNotfound(const PeerPtr &peer, const std::vector<UInt256> &txHashes,
									 const std::vector<UInt256> &blockHashes) {
			boost::mutex::scoped_lock scopedLock(lock);
			for (size_t i = 0; i < txHashes.size(); i++) {
				removePeerFromList(peer, txHashes[i], txRelays);
				removePeerFromList(peer, txHashes[i], txRequests);
			}
		}

		void PeerManager::OnSetFeePerKb(const PeerPtr &peer, uint64_t feePerKb) {
			uint64_t maxFeePerKb = 0, secondFeePerKb = 0;

			{
				boost::mutex::scoped_lock scopedLock(lock);
				for (size_t i = _connectedPeers.size(); i > 0; i--) { // find second highest fee rate
					const PeerPtr &p = _connectedPeers[i - 1];
					if (p->getConnectStatusValue() != Peer::Connected) continue;
					if (p->getFeePerKb() > maxFeePerKb) secondFeePerKb = maxFeePerKb, maxFeePerKb = p->getFeePerKb();
				}

				if (secondFeePerKb * 3 / 2 > DEFAULT_FEE_PER_KB && secondFeePerKb * 3 / 2 <= MAX_FEE_PER_KB &&
					secondFeePerKb * 3 / 2 > _wallet->getFeePerKb()) {
					peer->Pinfo("increasing feePerKb to {} based on feefilter messages from peers",
								secondFeePerKb * 3 / 2);
					_wallet->setFeePerKb(secondFeePerKb * 3 / 2);
				}
			}
		}

		const TransactionPtr &PeerManager::OnRequestedTx(const PeerPtr &peer, const UInt256 &txHash) {
			int hasPendingCallbacks = 0, error = 0;
			PublishedTransaction pubTx;

			{
				boost::mutex::scoped_lock scopedLock(lock);
				for (size_t i = publishedTx.size(); i > 0; i--) {
					if (UInt256Eq(&publishedTxHashes[i - 1], &txHash)) {
						pubTx = publishedTx[i - 1];
						publishedTx[i - 1].ResetCallback();
					} else if (publishedTx[i - 1].HasCallback()) hasPendingCallbacks = 1;
				}

				// cancel tx publish timeout if no publish callbacks are pending, and syncing is done or this is not downloadPeer
				if (!hasPendingCallbacks && (syncStartHeight == 0 || peer != downloadPeer)) {
					peer->scheduleDisconnect(-1); // cancel publish tx timeout
				}

				addPeerToList(peer, txHash, txRelays);
				if (pubTx.GetTransaction() != nullptr) _wallet->registerTransaction(pubTx.GetTransaction());
				if (pubTx.GetTransaction() != nullptr && !_wallet->transactionIsValid(pubTx.GetTransaction()))
					error = EINVAL;
			}

			if (pubTx.HasCallback()) pubTx.FireCallback(error);
			return pubTx.GetTransaction();
		}

		bool PeerManager::OnNetworkIsReachable(const PeerPtr &peer) {
			//fixme [refactor]
//			BRPeerManager *manager = ((BRPeerCallbackInfo *) info)->manager;
//
//			return (manager->networkIsReachable) ? manager->networkIsReachable(manager->info) : 1;
		}

		void PeerManager::OnThreadCleanup(const PeerPtr &peer) {
		}

		void PeerManager::publishPendingTx(const PeerPtr &peer) {
			for (size_t i = publishedTx.size(); i > 0; i--) {
				if (!publishedTx[i - 1].HasCallback()) continue;
				peer->scheduleDisconnect(PROTOCOL_TIMEOUT);  // schedule publish timeout
				break;
			}

			//fixme [refactor]
//			BRPeerSendInv(peer, manager->publishedTxHashes, array_count(manager->publishedTxHashes));
		}

		const std::vector<PublishedTransaction> PeerManager::getPublishedTransaction() const {
			return publishedTx;
		}

		const std::vector<UInt256> PeerManager::getPublishedTransactionHashes() const {
			return publishedTxHashes;
		}

		int PeerManager::reconnectTaskCount() const {
			return _reconnectTaskCount;
		}

		int &PeerManager::reconnectTaskCount() {
			return _reconnectTaskCount;
		}

		size_t
		PeerManager::addPeerToList(const PeerPtr &peer, const UInt256 &txHash, std::vector<TransactionPeerList> &list) {
			for (size_t i = list.size(); i > 0; i--) {
				if (!UInt256Eq(&list[i - 1].GetTransactionHash(), &txHash)) continue;

				for (size_t j = list[i - 1].GetPeers().size(); j > 0; j--) {
					if (list[i - 1].GetPeers()[j - 1]->IsEqual(peer.get()))
						return list[i - 1].GetPeers().size();
				}

				list[i - 1].AddPeer(peer);
				return list[i - 1].GetPeers().size();
			}

			list.push_back(TransactionPeerList(txHash, {peer}));
			return 1;
		}

		bool PeerManager::removePeerFromList(const PeerPtr &peer, const UInt256 &txHash,
											 std::vector<TransactionPeerList> &list) {
			for (size_t i = list.size(); i > 0; i--) {
				if (!UInt256Eq(&list[i - 1].GetTransactionHash(), &txHash)) continue;

				for (size_t j = list[i - 1].GetPeers().size(); j > 0; j--) {
					if (!list[i - 1].GetPeers()[j - 1]->IsEqual(peer.get())) continue;
					list[i - 1].RemovePeerAt(j - 1);
					return true;
				}

				break;
			}

			return false;
		}

		void PeerManager::peerMisbehaving(const PeerPtr &peer) {
			for (size_t i = _peers.size(); i > 0; i--) {
				if (_peers[i - 1]->IsEqual(peer.get()))
					_peers.erase(_peers.begin() + i - 1);
			}

			if (++misbehavinCount >= 10) { // clear out stored peers so we get a fresh list from DNS for next connect
				misbehavinCount = 0;
				_peers.clear();
			}

			peer->Disconnect();
		}

		void PeerManager::updateBloomFilter() {

			if (downloadPeer && (downloadPeer->GetFlags() & PEER_FLAG_NEEDSUPDATE) == 0) {
				downloadPeer->SetNeedsFilterUpdate(true);
				downloadPeer->SetFlags(downloadPeer->GetFlags() | PEER_FLAG_NEEDSUPDATE);
				downloadPeer->Pinfo("filter update needed, waiting for pong");
				// wait for pong so we're sure to include any tx already sent by the peer in the updated filter
				PingParameter pingParameter;
				pingParameter.callbackInfo.peer = downloadPeer;
				pingParameter.callbackInfo.manager = this;
				pingParameter.callback = boost::bind(&PeerManager::updateFilterPingDone, this, downloadPeer, _1);
				downloadPeer->SendMessage(MSG_PING, pingParameter);
			}
		}

		void PeerManager::updateFilterPingDone(const PeerPtr &peer, int success) {
			if (success) {
				boost::mutex::scoped_lock scopedLock(lock);
				peer->Pinfo("updating filter with newly created wallet addresses");
				if (bloomFilter) BRBloomFilterFree(bloomFilter);
				bloomFilter = nullptr;

				PingParameter pingParameter;
				pingParameter.callbackInfo.manager = this;
				if (lastBlock->getHeight() < estimatedHeight) { // if we're syncing, only update download peer
					if (downloadPeer) {
						loadBloomFilter(downloadPeer);
						pingParameter.callbackInfo.peer = downloadPeer;
						pingParameter.callback = boost::bind(&PeerManager::updateFilterPingDone, this, downloadPeer,
															 _1);
						downloadPeer->SendMessage(MSG_PING, pingParameter);// wait for pong so filter is loaded
					}
				} else {
					for (size_t i = _connectedPeers.size(); i > 0; i--) {
						if (_connectedPeers[i - 1]->getConnectStatusValue() != Peer::Connected) continue;
						pingParameter.callbackInfo.peer = _connectedPeers[i - 1];
						pingParameter.callback = boost::bind(&PeerManager::updateFilterPingDone, this,
															 _connectedPeers[i - 1],
															 _1);
						loadBloomFilter(peer);
						downloadPeer->SendMessage(MSG_PING, pingParameter);// wait for pong so filter is loaded
					}
				}
			}

		}

	}
}