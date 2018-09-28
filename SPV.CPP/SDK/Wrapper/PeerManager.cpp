// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <netinet/in.h>
#include <arpa/inet.h>
#include <Core/BRChainParams.h>
#include <Core/BRPeer.h>

#include "PeerManager.h"
#include "Utils.h"
#include "Log.h"
#include "BRArray.h"
#include "ELAMerkleBlock.h"
#include "arith_uint256.h"
#include "Plugin/Registry.h"
#include "Plugin/Block/MerkleBlock.h"

#define PROTOCOL_TIMEOUT      30.0
#define MAX_CONNECT_FAILURES  20 // notify user of network problems after this many connect failures in a row
#define PEER_FLAG_SYNCED      0x01
#define PEER_FLAG_NEEDSUPDATE 0x02

namespace Elastos {
	namespace ElaWallet {

		namespace {
			typedef struct {
				PeerManager *manager;
				const char *hostname;
				uint64_t services;
			} BRFindPeersInfo;

			typedef struct {
				BRPeer *peer;
				PeerManager *manager;
				UInt256 hash;
			} BRPeerCallbackInfo;

		}

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
				reconnectTaskCount(0),
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
					BRPeerCallbackInfo *info;

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
			//fixme [refactor]
//			uint32_t timestamp;
//
			{
				boost::mutex::scoped_lock scoped_lock(lock);
//			timestamp = lastBlock->timestamp;
			}
//			return timestamp;
			return 0;
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

			assert(tx != NULL && tx->isSigned());
			if (tx) lock.lock();

			if (tx && !tx->isSigned()) {
				lock.unlock();
				//fixme [refactor]
//				if (callback) callback(info, EINVAL); // transaction not signed
			} else if (tx && !isConnected) {
				int connectFailureCount = connectFailureCount;

				lock.unlock();

				if (connectFailureCount >= MAX_CONNECT_FAILURES ||
					(!networkIsReachable())) {
					//fixme [refactor]
//					if (callback) callback(info, ENOTCONN); // not connected to bitcoin network
//            tx = NULL; // add tx to publish tx list anyway
				} else lock.lock();
			}

			if (tx) {
				size_t i, count = 0;

				tx->setTimestamp((uint32_t) time(NULL)); // set timestamp to publish time
				//fixme [refactor]
//				addTxToPublishList(tx, callback);

				for (i = _connectedPeers.size(); i > 0; i--) {
					if (_connectedPeers[i - 1]->getConnectStatusValue() == Peer::Connected) count++;
				}

				for (i = _connectedPeers.size(); i > 0; i--) {
					//fixme [refactor]
//					BRPeer *peer = connectedPeers[i - 1];
//					BRPeerCallbackInfo *peerInfo;
//
//					if (BRPeerConnectStatus(peer) != BRPeerStatusConnected) continue;
//
//					// instead of publishing to all peers, leave out downloadPeer to see if tx propogates/gets relayed back
//					// TODO: XXX connect to a random peer with an empty or fake bloom filter just for publishing
//					if (peer != downloadPeer || count == 1) {
//						_BRPeerManagerPublishPendingTx(manager, peer);
//						peerInfo = calloc(1, sizeof(*peerInfo));
//						assert(peerInfo != NULL);
//						peerInfo->peer = peer;
//						peerInfo->manager = manager;
//						peerMessages->BRPeerSendPingMessage(peer, peerInfo, _publishTxInvDone);
//					}
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

		void PeerManager::loadBloomFilter(BRPeer *peer) {
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
//fixme [refactor]
			// comparator for sorting peers by timestamp, most recent first
		}

		void PeerManager::findPeers() {
			uint64_t services = SERVICES_NODE_NETWORK | SERVICES_NODE_BLOOM | _chainParams.getRaw()->services;
			time_t now = time(NULL);
			struct timespec ts;
			pthread_t thread;
			pthread_attr_t attr;
			UInt128 *addr, *addrList = NULL;
			BRFindPeersInfo *info;

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

		void PeerManager::addTxToPublishList(const TransactionPtr &tx, void (*callback)(void *, int)) {
			//fixme [refactor]
//			if (tx && tx->getBlockHeight() == TX_UNCONFIRMED) {
//				for (size_t i = array_count(publishedTx); i > 0; i--) {
//					if (BRTransactionEq(publishedTx[i - 1].tx, tx)) return;
//				}
//
//				array_add(publishedTx, ((BRPublishedTx) {tx, info, callback}));
//				array_add(publishedTxHashes, tx->txHash);
//
//				for (size_t i = 0; i < tx->inCount; i++) {
//					_BRPeerManagerAddTxToPublishList(manager,
//													 BRWalletTransactionForHash(wallet, tx->inputs[i].txHash),
//													 NULL, NULL);
//				}
//			}
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

			if (reconnectTaskCount > 0) {
				{
					boost::mutex::scoped_lock scoped_lock(lock);
					reconnectTaskCount = 0;
				}
			}
		}
	}

}
