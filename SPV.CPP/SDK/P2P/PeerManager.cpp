// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "PeerManager.h"
#include "Message/PingMessage.h"
#include "Message/GetBlocksMessage.h"
#include "Message/FilterLoadMessage.h"
#include "Message/MempoolMessage.h"
#include "Message/GetDataMessage.h"
#include "Message/InventoryMessage.h"

#include <Plugin/Transaction/TransactionOutput.h>
#include <Plugin/Block/ELAMerkleBlock.h>
#include <Plugin/Registry.h>
#include <Common/Utils.h>
#include <Common/Log.h>
#include <WalletCore/BloomFilter.h>
#include <WalletCore/HDKeychain.h>
#include <Wallet/Wallet.h>
#include <P2P/ChainParams.h>

#include <netdb.h>
#include <netinet/in.h>
#include <boost/bind.hpp>
#include <boost/thread.hpp>
#include <arpa/inet.h>

#define PROTOCOL_TIMEOUT      40.0
#define MAX_CONNECT_FAILURES  1000 // notify user of network problems after this many connect failures in a row
#define PEER_FLAG_SYNCED      0x01
#define PEER_FLAG_NEEDSUPDATE 0x02

namespace Elastos {
	namespace ElaWallet {

		void PeerManager::FireSyncStarted() {
			if (!_listener.expired()) {
				_listener.lock()->syncStarted();
			}
		}

		void PeerManager::FireSyncProgress(double progress, const PeerPtr &peer, const MerkleBlockPtr &block) {
			struct timeval tv;
			gettimeofday(&tv, NULL);

			uint64_t now = tv.tv_sec * 1000 + tv.tv_usec / 1000;
			uint64_t milliseconds = now - peer->GetDownloadStartTime();
			uint32_t bytesPerSecond = 0;

			if (milliseconds != 0)
				bytesPerSecond = peer->GetDownloadBytes() * 1000 / milliseconds;

			peer->ScheduleDownloadStartTime();
			peer->SetDownloadBytes(0);

			if (!_listener.expired()) {
				_listener.lock()->syncProgress((uint32_t)(progress * 100), block->GetTimestamp(), bytesPerSecond, peer->GetHost());
			}
		}

		void PeerManager::FireSyncStopped(int error) {
			if (!_listener.expired()) {
				_listener.lock()->syncStopped(error == 0 ? "" : strerror(error));
			}
		}

		void PeerManager::FireTxStatusUpdate() {
			if (!_listener.expired()) {
				_listener.lock()->txStatusUpdate();
			}
		}

		void PeerManager::FireSaveBlocks(bool replace, const std::vector<MerkleBlockPtr> &blocks) {
			if (!_listener.expired()) {
				_listener.lock()->saveBlocks(replace, blocks);
			}
		}

		void PeerManager::FireSavePeers(bool replace, const std::vector<PeerInfo> &peers) {
			if (!_listener.expired()) {
				_listener.lock()->savePeers(replace, peers);
			}
		}

		void PeerManager::FireSaveBlackPeer(const PeerInfo &peer) {
			if (!_listener.expired()) {
				_listener.lock()->saveBlackPeer(peer);
			}
		}

		bool PeerManager::FireNetworkIsReachable() {
			bool result = false;
			if (!_listener.expired()) {
				result = _listener.lock()->networkIsReachable();
			}
			return result;
		}

		void PeerManager::FireTxPublished(const uint256 &hash, int code, const std::string &reason) {
			nlohmann::json result;
			result["Code"] = code;
			result["Reason"] = reason;
			std::string txID = hash.GetHex();

			if (!_listener.expired()) {
				_listener.lock()->txPublished(txID, result);
			}
		}

		void PeerManager::FireConnectStatusChanged(Peer::ConnectStatus status) {
			if (!_listener.expired()) {
				std::string st = status == Peer::Connecting ? "Connecting" :
								 (status == Peer::Connected ? "Connected" : "Disconnected");
				_listener.lock()->connectStatusChanged(st);
			}
		}

		void PeerManager::FireThreadCleanup() {
			if (!_listener.expired()) {
			}
		}

		PeerManager::Listener::Listener() {
		}

		PeerManager::PeerManager(const ChainParamsPtr &params,
								 const WalletPtr &wallet,
								 time_t earliestKeyTime,
								 uint32_t reconnectSeconds,
								 const std::vector<MerkleBlockPtr> &blocks,
								 const std::vector<PeerInfo> &peers,
								 const std::set<PeerInfo> &blackPeers,
								 const boost::shared_ptr<PeerManager::Listener> &listener,
								 const std::string &chainID,
								 const std::string &netType) :
				_wallet(wallet),
				_lastBlock(nullptr),
				_lastOrphan(nullptr),
				_chainID(chainID),
				_netType(netType),
				_chainParams(params),

				_syncSucceeded(false),
				_enableReconnect(true),

				_isConnected(0),
				_connectFailureCount(0),
				_misbehavinCount(0),
				_dnsThreadCount(0),
				_maxConnectCount(PEER_MAX_CONNECTIONS),
				_connectStatus(Peer::Disconnected),

				_keepAliveTimestamp(0),
				_earliestKeyTime(earliestKeyTime),
				_reconnectSeconds(reconnectSeconds),
				_syncStartHeight(0),
				_filterUpdateHeight(0),
				_estimatedHeight(0),

				_fpRate(0),
				_averageTxPerBlock(1400) {

			assert(listener != nullptr);
			_listener = boost::weak_ptr<Listener>(listener);

			_peers = peers;
			_blackPeers.insert(blackPeers.begin(), blackPeers.end());
			SortPeers();

			InitBlocks(blocks);
		}

		void PeerManager::InitBlocks(const std::vector<MerkleBlockPtr> &blocks) {
			const std::vector<CheckPoint> &Checkpoints = _chainParams->Checkpoints();
			for (size_t i = 0; i < Checkpoints.size(); i++) {
				MerkleBlockPtr checkBlock = Registry::Instance()->CreateMerkleBlock(_chainID);
				checkBlock->SetHeight(Checkpoints[i].Height());
				checkBlock->SetHash(Checkpoints[i].Hash());
				checkBlock->SetTimestamp(Checkpoints[i].Timestamp());
				checkBlock->SetTarget(Checkpoints[i].Target());
				_checkpoints.Insert(checkBlock);
				_blocks.Insert(checkBlock);
				if (i == 0 || checkBlock->GetTimestamp() + 1 * 24 * 60 * 60 < _earliestKeyTime)
					_lastBlock = checkBlock;
			}

			MerkleBlockPtr block = nullptr, earlistBlock = nullptr;
			for (size_t i = 0; i < blocks.size(); i++) {
				assert(blocks[i]->GetHeight() !=
					   BLOCK_UNKNOWN_HEIGHT); // height must be saved/restored along with serialized block
				_orphans.Insert(blocks[i]);

				if ((blocks[i]->GetHeight() % BLOCK_DIFFICULTY_INTERVAL) == 0 &&
					(block == nullptr || blocks[i]->GetHeight() > block->GetHeight()))
					block = blocks[i]; // find last transition block

				if (earlistBlock == nullptr || blocks[i]->GetHeight() < earlistBlock->GetHeight())
					earlistBlock = blocks[i];
			}
			if (block == nullptr)
				block = earlistBlock;

			while (block != nullptr) {
				_blocks.Insert(block);
				_lastBlock = block;
				_orphans.Remove(block);
				block = _orphans.GetMatchPrevHash(block->GetHash());
			}
		}

		PeerManager::~PeerManager() {
		}

		void PeerManager::SetWallet(const WalletPtr &wallet) {
			_wallet = wallet;
		}

		Peer::ConnectStatus PeerManager::GetConnectStatusInternal() const {
			Peer::ConnectStatus status = Peer::Disconnected;
			if (_isConnected != 0) status = Peer::Connected;

			for (size_t i = _connectedPeers.size(); i > 0 && status == Peer::Disconnected; i--) {
				if (_connectedPeers[i - 1]->GetConnectStatus() == Peer::Disconnected) continue;
				status = Peer::Connecting;
			}

			return status;
		}

		Peer::ConnectStatus PeerManager::GetConnectStatus() const {
			boost::mutex::scoped_lock scoped_lock(lock);
			return GetConnectStatusInternal();
		}

		bool PeerManager::SyncSucceeded() const {
			boost::mutex::scoped_lock scopedLock(lock);
			return _syncSucceeded;
		}

		void PeerManager::SetSyncSucceeded(bool succeeded) {
			boost::mutex::scoped_lock scopedLock(lock);
			_syncSucceeded = succeeded;
		}

		void PeerManager::SetReconnectEnableStatus(bool status) {
			boost::mutex::scoped_lock scopedLock(lock);
			_enableReconnect = status;
		}

		bool PeerManager::GetReconnectEnableStatus() const {
			boost::mutex::scoped_lock scopedLock(lock);
			return _enableReconnect;
		}

		void PeerManager::Connect() {
			bool connectionStatusChanged = false;
			Peer::ConnectStatus status;
			lock.lock();
//			if (_connectFailureCount >= MAX_CONNECT_FAILURES) _connectFailureCount = 0; //this is a manual retry

			if ((!_downloadPeer || _lastBlock->GetHeight() < _estimatedHeight) && _syncStartHeight == 0) {
				_syncStartHeight = _lastBlock->GetHeight() + 1;
				lock.unlock();
				FireSyncStarted();
				lock.lock();
			}

			for (size_t i = _connectedPeers.size(); i > 0; i--) {
				if (_connectedPeers[i - 1]->GetConnectStatus() == Peer::Connecting)
					_connectedPeers[i - 1]->Connect();
			}

			if (_connectedPeers.size() < _maxConnectCount) {
				time_t now = time(NULL);
				std::vector<PeerInfo> peers;

				if (_peers.size() < _maxConnectCount ||
					_peers[_maxConnectCount - 1].Timestamp + 60 * 24 * 60 * 60 < now) {
					FindPeers();
				}

				peers.insert(peers.end(), _peers.begin(), _peers.end());

				while (!peers.empty() && _connectedPeers.size() < _maxConnectCount) {
					size_t i = BRRand((uint32_t) peers.size()); // index of random peer

					i = i * i / peers.size(); // bias random peer selection toward peers with more recent timestamp

					for (size_t j = _connectedPeers.size(); i != SIZE_MAX && j > 0; j--) {
						if (peers[i] != _connectedPeers[j - 1]->GetPeerInfo()) continue;
						peers.erase(peers.begin() + i);
						i = SIZE_MAX;
					}

					if (i != SIZE_MAX) {
						PeerPtr newPeer = PeerPtr(new Peer(this, _chainParams->MagicNumber()));
						newPeer->InitDefaultMessages();
						newPeer->SetPeerInfo(peers[i]);
						newPeer->setEarliestKeyTime(_earliestKeyTime);
						peers.erase(peers.begin() + i);

						_connectedPeers.push_back(newPeer);
						newPeer->Connect();
					}
				}
			}

			status = GetConnectStatusInternal();
			if (_connectStatus != status) {
				_connectStatus = status;
				connectionStatusChanged = true;
			}

			if (_connectedPeers.empty()) {
				Log::error("{} sync failed: {}", GetID(), std::string(strerror(ENETUNREACH)));
				SyncStopped();
				lock.unlock();
				FireSyncStopped(ENETUNREACH);
			} else {
				lock.unlock();
			}

			if (connectionStatusChanged) FireConnectStatusChanged(status);
		}

		void PeerManager::AsyncConnect(const boost::system::error_code &e) {
			if (e != boost::asio::error::operation_aborted) {
				if (GetConnectStatus() != Peer::Connected)
					Connect();
			} else {
				Log::warn("{} async connect err.value: {} err.msg: {}", GetID(), e.value(), e.message());
			}
		}

		void PeerManager::ConnectLaster(time_t seconds) {
			lock.lock();
			_enableReconnect = true;
			lock.unlock();

			if (_reconnectTimer) {
				_reconnectTimer->expires_from_now(boost::posix_time::seconds(seconds));
				_reconnectService.restart();
			} else {
				_reconnectTimer = boost::shared_ptr<boost::asio::deadline_timer>(new boost::asio::deadline_timer(
					_reconnectService, boost::posix_time::seconds(seconds)));
			}

			Log::debug("{} connect {} seconds later", GetID(), seconds);
			_reconnectTimer->async_wait(boost::bind(&PeerManager::AsyncConnect, this, boost::asio::placeholders::error));
			_reconnectService.run();
		}

		void PeerManager::CancelTimer() {
			boost::mutex::scoped_lock scopedLock(lock);
			if (_reconnectTimer) {
				_reconnectTimer->cancel();
			}
		}

		void PeerManager::ClearData() {
			_isConnected = 0;
			_connectFailureCount = 0;
			_misbehavinCount = 0;
			_syncSucceeded = false;
			_connectedPeers.clear();
			_peers.clear();
			_blackPeers.clear();
			_connectedPeers.clear();
			_syncStartHeight = 0;
			_fpRate = 0;
			_averageTxPerBlock = 1400;
			_blocks.Clear();
			_orphans.Clear();
			_checkpoints.Clear();
			_txRelays.clear();
			_txRequests.clear();
			_publishedTx.clear();
			_publishedTxHashes.clear();

			InitBlocks({});
		}

		void PeerManager::ReconnectLaster(time_t seconds) {
			Disconnect();
			ConnectLaster(seconds);
		}

		double PeerManager::GetSyncProgressInternal(uint32_t startHeight) {
			double progress;

			if (startHeight == 0) startHeight = _syncStartHeight;

			if (!_downloadPeer && _syncStartHeight == 0) {
				progress = 0.0;
			} else if (!_downloadPeer || _lastBlock->GetHeight() < _estimatedHeight) {
				if (_lastBlock->GetHeight() > startHeight && _estimatedHeight > startHeight) {
					progress = 0.1 + 0.9 * (_lastBlock->GetHeight() - startHeight) / (_estimatedHeight - startHeight);
				} else {
					progress = 0.05;
				}
			} else {
				progress = 1.0;
			}

			return progress;
		}

		void PeerManager::Disconnect() {
			struct timespec ts;
			size_t peerCount = 0;
			int dnsThreadCount;

			lock.lock();
			_enableReconnect = false;
			lock.unlock();

			usleep(1000);

			lock.lock();
			peerCount = _connectedPeers.size();
			dnsThreadCount = _dnsThreadCount;

			for (size_t i = peerCount; i > 0; i--) {
				_connectedPeers[i - 1]->Disconnect();
			}
			lock.unlock();

			ts.tv_sec = 0;
			ts.tv_nsec = 1000;

			while (peerCount > 0 || dnsThreadCount > 0) {
				nanosleep(&ts, NULL); // pthread_yield() isn't POSIX standard :(
				lock.lock();
				peerCount = _connectedPeers.size();
				dnsThreadCount = _dnsThreadCount;
				lock.unlock();
			}
		}

		void PeerManager::Rescan() {
			lock.lock();

			if (_isConnected) {
				// start the chain download from the most recent checkpoint that's at least a week older than earliestKeyTime
				const std::vector<CheckPoint> &checkpoints = _chainParams->Checkpoints();
				for (size_t i = checkpoints.size(); i > 0; i--) {
					if (i - 1 == 0 ||
						checkpoints[i - 1].Timestamp() + 7 * 24 * 60 * 60 < _earliestKeyTime) {
						uint256 hash = checkpoints[i - 1].Hash();
						_lastBlock = _blocks.Get(hash);
						break;
					}
				}

				if (_downloadPeer) { // disconnect the current download peer so a new random one will be selected
					RemovePeer(_downloadPeer);

					_downloadPeer->Disconnect();
				}

				_syncStartHeight = 0; // a syncStartHeight of 0 indicates that syncing hasn't started yet
				lock.unlock();
				Connect();
			} else lock.unlock();
		}

		uint32_t PeerManager::GetSyncStartHeight() const {
			return _syncStartHeight;
		}

		uint32_t PeerManager::GetEstimatedBlockHeight() const {
			uint32_t height;

			{
				boost::mutex::scoped_lock scoped_lock(lock);
				height = (_lastBlock->GetHeight() < _estimatedHeight) ? _estimatedHeight : _lastBlock->GetHeight();
			}
			return height;
		}

		uint32_t PeerManager::GetLastBlockHeight() const {
			boost::mutex::scoped_lock scoped_lock(lock);
			return _lastBlock->GetHeight();
		}

		uint32_t PeerManager::GetLastBlockTimestamp() const {
			uint32_t timestamp;

			{
				boost::mutex::scoped_lock scoped_lock(lock);
				timestamp = _lastBlock->GetTimestamp();
			}
			return timestamp;
		}

		time_t PeerManager::GetKeepAliveTimestamp() const {
			time_t t;
			{
				boost::mutex::scoped_lock scoped_lock(lock);
				t = _keepAliveTimestamp;
			}
			return t;
		}

		void PeerManager::SetKeepAliveTimestamp(time_t t) {
			_keepAliveTimestamp = t;
		}

		double PeerManager::GetSyncProgress(uint32_t startHeight) {
			boost::mutex::scoped_lock scoped_lock(lock);
			return GetSyncProgressInternal(startHeight);
		}

		bool PeerManager::SetFixedPeer(const std::string &address, uint16_t port) {
			std::vector<uint128> addrList = AddressLookup(address);
			if (addrList.empty()) {
				Log::error("invalid domain name: {}", address);
				return false;
			}

			{
				boost::mutex::scoped_lock scoped_lock(lock);
				_maxConnectCount = (addrList[0] == 0) ? PEER_MAX_CONNECTIONS : 1;
				_fixedPeer = PeerInfo(addrList[0], port, 0, 0);
				_peers.clear();
			}

			boost::thread workThread(boost::bind(&PeerManager::ReconnectLaster, this, 1));
			return true;
		}

		std::string PeerManager::GetCurrentPeerName() const {
			return GetDownloadPeerName();
		}

		std::string PeerManager::GetDownloadPeerName() const {
			boost::mutex::scoped_lock scoped_lock(lock);
			if (_downloadPeer) {
				std::stringstream ss;
				ss << _downloadPeer->GetHost() << ":" << _downloadPeer->GetPort();
				_downloadPeerName = ss.str();
			} else {
				_downloadPeerName = "";
			}

			return _downloadPeerName;
		}

		PeerPtr PeerManager::GetDownloadPeer() const {
			boost::mutex::scoped_lock scopedLock(lock);
			return _downloadPeer;
		}

		size_t PeerManager::GetPeerCount() const {
			size_t count = 0;

			{
				boost::mutex::scoped_lock scoped_lock(lock);
				for (size_t i = _connectedPeers.size(); i > 0; i--) {
					if (_connectedPeers[i - 1]->GetConnectStatus() != Peer::Disconnected) count++;
				}
			}
			return count;
		}

		void PeerManager::PublishTransaction(const TransactionPtr &tx) {
			PublishTransaction(tx, boost::bind(&PeerManager::FireTxPublished, this, _1, _2, _3));
		}

		void PeerManager::PublishTransaction(const TransactionPtr &tx,
											 const Peer::PeerPubTxCallback &callback) {

			bool txValid = (tx != nullptr);
			if (tx) lock.lock();

			if (tx && !tx->IsSigned()) {
				lock.unlock();
				if (!callback.empty()) callback(tx->GetHash(), EINVAL, "tx not signed"); // transaction not signed
				txValid = false;
//			} else if (tx && !_isConnected) {
//				int connectFailureCount = connectFailureCount;
//				lock.unlock();
//
//				if (connectFailureCount >= MAX_CONNECT_FAILURES ||
//					(!fireNetworkIsReachable())) {
//					if (!callback.empty()) callback(ENOTCONN); // not connected to bitcoin network
//				} else lock.lock();
			}

			if (txValid) {
				size_t i, count = 0;

				tx->SetTimestamp((uint32_t) time(NULL)); // set timestamp to publish time
				AddTxToPublishList(tx, callback);

				for (i = _connectedPeers.size(); i > 0; i--) {
					if (_connectedPeers[i - 1]->GetConnectStatus() == Peer::Connected) count++;
				}

				for (i = _connectedPeers.size(); i > 0; i--) {
					const PeerPtr &peer = _connectedPeers[i - 1];

					if (peer->GetConnectStatus() != Peer::Connected) continue;

					// instead of publishing to all peers, leave out downloadPeer to see if tx propogates/gets relayed back
					// TODO: XXX connect to a random peer with an empty or fake bloom filter just for publishing
					if (peer != _downloadPeer || count == 1) {
						PublishPendingTx(peer);

						PingParameter pingParameter(_lastBlock->GetHeight(),
													boost::bind(&PeerManager::PublishTxInvDone, this, peer, _1));
						peer->SendMessage(MSG_PING, pingParameter);
					}
				}

				lock.unlock();
			}
		}

		uint64_t PeerManager::GetRelayCount(const uint256 &txHash) const {
			size_t count = 0;

			assert(txHash != 0);

			{
				boost::mutex::scoped_lock scoped_lock(lock);
				for (size_t i = _txRelays.size(); i > 0; i--) {
					if (_txRelays[i - 1].GetTransactionHash() != txHash)
						continue;
					count = _txRelays[i - 1].GetPeers().size();
					break;
				}
			}

			return count;
		}

		int PeerManager::VerifyDifficulty(const ChainParamsPtr &params, const MerkleBlockPtr &block,
										  const BlockSet &blockSet) {
			MerkleBlockPtr previous, b = nullptr;
			uint32_t i;
			const uint32_t &targetTimeSpan = params->TargetTimeSpan();
			const uint32_t &targetTimePerBlock = params->TargetTimePerBlock();

			assert(block != nullptr);

			uint64_t blocksPerRetarget = targetTimeSpan / targetTimePerBlock;

			// check if we hit a difficulty transition, and find previous transition block
			if ((block->GetHeight() % blocksPerRetarget) == 0) {
				for (i = 0, b = block; b && i < blocksPerRetarget; i++) {
					b = blockSet.Get(b->GetPrevBlockHash());
				}
			}

			previous = blockSet.Get(block->GetPrevBlockHash());
			return VerifyDifficultyInner(block, previous, (b != nullptr) ? b->GetTimestamp() : 0, targetTimeSpan,
										 targetTimePerBlock);
		}

		int PeerManager::VerifyDifficultyInner(const MerkleBlockPtr &block, const MerkleBlockPtr &previous,
											   uint32_t transitionTime, uint32_t targetTimeSpan,
											   uint32_t targetTimePerBlock) {
			int r = 1;

//			assert(block != nullptr);
//			assert(previous != nullptr);
//
//			uint64_t blocksPerRetarget = targetTimeSpan / targetTimePerBlock;
//
//			if (!previous || !uint256Eq(&(block->GetPrevBlockHash()), &(previous->GetHash())) ||
//				block->GetHeight() != previous->GetHeight() + 1)
//				r = 0;
//			if (r && (block->GetHeight() % blocksPerRetarget) == 0 && transitionTime == 0) r = 0;
//
//			if (r && (block->GetHeight() % blocksPerRetarget) == 0) {
//				uint32_t timespan = previous->GetTimestamp() - transitionTime;
//
//				arith_uint256 target;
//				target.SetCompact(previous->GetTarget());
//
				// limit difficulty transition to -75% or +400%
//				if (timespan < targetTimeSpan / 4) timespan = uint32_t(targetTimeSpan) / 4;
//				if (timespan > targetTimeSpan * 4) timespan = uint32_t(targetTimeSpan) * 4;

				// TARGET_TIMESPAN happens to be a multiple of 256, and since timespan is at least TARGET_TIMESPAN/4, we don't
				// lose precision when target is multiplied by timespan and then divided by TARGET_TIMESPAN/256
//				target *= timespan;
//				target /= targetTimeSpan;

//				uint32_t actualTargetCompact = target.GetCompact();
//				if (block->GetTarget() != actualTargetCompact) r = 0;
//			} else if (r && previous->GetHeight() != 0 && block->GetTarget() != previous->GetTarget()) r = 0;
//
			return r;
		}

		void PeerManager::LoadBloomFilter(const PeerPtr &peer) {
			// every time a new wallet address is added, the bloom filter has to be rebuilt, and each address is only used
			// for one transaction, so here we generate some spare addresses to avoid rebuilding the filter each time a
			// wallet transaction is encountered during the chain sync
			_wallet->UnusedAddresses(SEQUENCE_GAP_LIMIT_EXTERNAL + 100, 0);
			_wallet->UnusedAddresses(SEQUENCE_GAP_LIMIT_INTERNAL + 100, 1);

			_orphans.Clear(); // clear out orphans that may have been received on an old filter
			_lastOrphan = nullptr;
			_filterUpdateHeight = _lastBlock->GetHeight();
			_fpRate = BLOOM_REDUCED_FALSEPOSITIVE_RATE;

			AddressArray specialAddresses = _wallet->GetAllSpecialAddresses();

			AddressArray addrs, addrInternal, allCID;
			_wallet->GetAllAddresses(addrs, 0, UINT32_MAX, false);
			_wallet->GetAllAddresses(addrInternal, 0, UINT32_MAX, true);
			addrs.insert(addrs.end(), addrInternal.begin(), addrInternal.end());
			_wallet->GetAllCID(allCID, 0, UINT32_MAX);

			UTXOArray utxos = _wallet->GetAllUTXO("");
			uint32_t blockHeight = (_lastBlock->GetHeight() > 100) ? _lastBlock->GetHeight() - 100 : 0;

			std::vector<TransactionPtr> transactions = _wallet->TxUnconfirmedBefore(blockHeight);

			size_t elementCount = specialAddresses.size() + addrs.size() + allCID.size() +
				utxos.size() + transactions.size();

			bool is_side_wallet = addrs.size() == 1 && addrs[0]->ProgramHash().prefix() == PrefixCrossChain;
			uint32_t tweak = is_side_wallet ? UINT32_MAX : (uint32_t) peer->GetPeerInfo().GetHash();
			BloomFilterPtr filter = BloomFilterPtr(new BloomFilter(_fpRate, elementCount + 100, tweak,
																   BLOOM_UPDATE_ALL)); // BUG: XXX txCount not the same as number of spent wallet outputs

			bytes_t hash;

			for (size_t i = 0; i < specialAddresses.size(); ++i) {
				if (specialAddresses[i]->Valid()) {
					hash = specialAddresses[i]->ProgramHash().bytes();
					if (!filter->ContainsData(hash))
						filter->InsertData(hash);
				}
			}

			for (size_t i = 0; i < addrs.size(); i++) { // add addresses to watch for tx receiveing money to the wallet
				if (addrs[i]->Valid()) {
					hash = addrs[i]->ProgramHash().bytes();
					if (!filter->ContainsData(hash))
						filter->InsertData(hash);
				}
			}

			for (size_t i = 0; i < allCID.size(); ++i) {
				hash = allCID[i]->ProgramHash().bytes();
				if (!filter->ContainsData(hash)) {
					filter->InsertData(hash);
				}
			}

			for (size_t i = 0; i < utxos.size(); i++) { // add UTXOs to watch for tx sending money from the wallet
				bytes_t o = utxos[i]->Hash().bytes();
				o.append(utxos[i]->Index());

				if (!filter->ContainsData(o))
					filter->InsertData(o);
			}

			for (size_t i = 0; i < transactions.size(); i++) { // also add TXOs spent within the last 100 blocks
				const InputArray &inputs = transactions[i]->GetInputs();
				for (InputArray::const_iterator in = inputs.cbegin(); in != inputs.cend(); ++in) {
					const TransactionPtr &tx = _wallet->TransactionForHash((*in)->TxHash());
					if (tx) {
						OutputPtr output = tx->OutputOfIndex((*in)->Index());
						if (output && _wallet->ContainsAddress(output->Addr())) {
							bytes_t o = (*in)->TxHash().bytes();
							o.append((*in)->Index());
							if (!filter->ContainsData(o))
								filter->InsertData(o);
						}
					}
				}
			}

			_bloomFilter = filter;
			// TODO: XXX if already synced, recursively add inputs of unconfirmed receives
			FilterLoadParameter bloomFilterParameter;
			bloomFilterParameter.Filter = filter;
			peer->SendMessage(MSG_FILTERLOAD, bloomFilterParameter);
		}

		void PeerManager::SortPeers() {
			// comparator for sorting peers by timestamp, most recent first
			std::sort(_peers.begin(), _peers.end(), [](const PeerInfo &first, const PeerInfo &second) {
				return first.Timestamp > second.Timestamp;
			});
		}

		void PeerManager::FindPeers() {
			uint64_t services = SERVICES_NODE_NETWORK | SERVICES_NODE_BLOOM | _chainParams->Services();
			time_t now = time(NULL);
			struct timespec ts;

			_peers.clear();

			if (_fixedPeer.Address != 0) {
				_peers.push_back(_fixedPeer);
				_peers[0].Services = services;
				_peers[0].Timestamp = now;
			} else {
				const std::vector<std::string> &dnsSeeds = _chainParams->DNSSeeds();
				for (size_t i = 1; i < dnsSeeds.size(); i++) {
					boost::thread workThread(boost::bind(&PeerManager::FindPeersThreadRoutine, this, dnsSeeds[i], services));
					_dnsThreadCount++;
				}

				std::vector<uint128> addrList = AddressLookup(dnsSeeds[0]);
				for (std::vector<uint128>::iterator addr = addrList.begin();
					 addr != addrList.end() && (*addr) != 0; addr++) {
					_peers.emplace_back(*addr, _chainParams->StandardPort(), now, services);
				}

				ts.tv_sec = 0;
				ts.tv_nsec = 1;

				do {
					Unlock();
					nanosleep(&ts, NULL); // pthread_yield() isn't POSIX standard :(
					Lock();
				} while (_dnsThreadCount > 0 && _peers.size() < PEER_MAX_CONNECTIONS);

				SortPeers();

				Log::debug("{} found {} peers", GetID(), _peers.size());
			}
		}

		void PeerManager::SyncStopped() {
			_syncStartHeight = 0;

			if (_downloadPeer != nullptr) {
				// don't cancel timeout if there's a pending tx publish callback
				for (size_t i = _publishedTx.size(); i > 0; i--) {
					if (_publishedTx[i - 1].HasCallback()) return;
				}

				_downloadPeer->ScheduleDisconnect(-1); // cancel sync timeout
			}
		}

		void PeerManager::AddTxToPublishList(const TransactionPtr &tx, const Peer::PeerPubTxCallback &callback) {
			if (tx && tx->GetBlockHeight() == TX_UNCONFIRMED) {
				for (size_t i = _publishedTx.size(); i > 0; i--) {
					if (_publishedTx[i - 1].GetTransaction()->IsEqual(*tx)) return;
				}

				_publishedTx.emplace_back(tx, callback);
				_publishedTxHashes.push_back(tx->GetHash());

				for (size_t i = 0; i < tx->GetInputs().size(); i++) {
					AddTxToPublishList(_wallet->TransactionForHash(tx->GetInputs()[i]->TxHash()),
									   Peer::PeerPubTxCallback());
				}
			}
		}

		void PeerManager::OnConnected(const PeerPtr &peerPtr) {
			time_t now = time(nullptr);

			Peer::ConnectStatus status = Peer::Disconnected;
			bool connectionStatusChanged = false;
			PeerPtr peer = peerPtr;

			lock.lock();


			// TODO: XXX does this work with 0.11 pruned nodes?
			if ((peer->GetServices() & _chainParams->Services()) != _chainParams->Services()) {
				peer->warn("unsupported node type");
				peer->Disconnect();
			} else if (peer->GetServices() == 0 && peer->GetLastBlock() == 0) {
				// Get address from address server
				peer->SendMessage(MSG_GETADDR, Message::DefaultParam);
				peer->ScheduleDisconnect(PROTOCOL_TIMEOUT); // schedule sync timeout

				PingParameter pingParameter(_lastBlock->GetHeight(),
											boost::bind(&PeerManager::UpdateAddressOnlyDone, this, peer, _1));
				peer->SendMessage(MSG_PING, pingParameter);

			} else if ((peer->GetServices() & SERVICES_NODE_NETWORK) != SERVICES_NODE_NETWORK) {
				peer->warn("peer->services: {} != SERVICES_NODE_NETWORK", peer->GetServices());
				peer->warn("node doesn't carry full blocks");
				peer->Disconnect();
				_blackPeers.insert(peer->GetPeerInfo());
			} else if (peer->GetLastBlock() + 10 < _lastBlock->GetHeight()) {
				peer->warn("peer->lastBlock: {} !=  lastBlock->height: {}", peer->GetLastBlock(),
						   _lastBlock->GetHeight());
				peer->warn("node isn't synced");
				peer->Disconnect();
//			} else if (peer->GetVersion() >= 70011 &&
//					   (peer->GetServices() & SERVICES_NODE_BLOOM) != SERVICES_NODE_BLOOM) {
//				peer->warn("node doesn't support SPV mode");
//				peer->Disconnect();
			} else if (_downloadPeer && // check if we should stick with the existing download peer
					   (_downloadPeer->GetLastBlock() >= peer->GetLastBlock() ||
						_lastBlock->GetHeight() >= peer->GetLastBlock())) {
				if (_lastBlock->GetHeight() >= peer->GetLastBlock()) { // only load bloom filter if we're done syncing
					_connectFailureCount = 0; // also reset connect failure count if we're already synced
					LoadBloomFilter(peer);
					PublishPendingTx(peer);
					PingParameter pingParameter(_lastBlock->GetHeight(),
												boost::bind(&PeerManager::LoadBloomFilterDone, this, peer, _1));
					peer->SendMessage(MSG_PING, pingParameter);
				}

				if (peer->GetTimestamp() > now + 2 * 60 * 60 || peer->GetTimestamp() < now - 2 * 60 * 60)
					peer->SetTimestamp(now); // sanity check
			} else { // select the peer with the lowest ping time to download the chain from if we're behind
				// BUG: XXX a malicious peer can report a higher lastblock to make us select them as the download peer, if
				// two peers agree on lastblock, use one of those two instead
				for (size_t i = _connectedPeers.size(); i > 0; i--) {
					const PeerPtr &p = _connectedPeers[i - 1];

					if (p->GetConnectStatus() != Peer::Connected) continue;
					if ((p->GetPingTime() < peer->GetPingTime() && p->GetLastBlock() >= peer->GetLastBlock()) ||
						p->GetLastBlock() > peer->GetLastBlock())
						peer = p;
				}

				if (peer->GetTimestamp() > now + 2 * 60 * 60 || peer->GetTimestamp() < now - 2 * 60 * 60)
					peer->SetTimestamp(now); // sanity check

				if (_downloadPeer) {
					peer->info("selecting new download peer with higher reported lastblock");
					_downloadPeer->Disconnect();
				}

				peer->SetWaitingBlocks(false);
				_downloadPeer = peer;
				_syncSucceeded = false;
				_keepAliveTimestamp = time(nullptr);
				_isConnected = 1;
				_estimatedHeight = peer->GetLastBlock();
				_connectFailureCount = 0; // reset connect failure count
				//peer->SendMessage(MSG_GETADDR, Message::DefaultParam);

				peer->ScheduleDownloadStartTime();
				LoadBloomFilter(peer);
				peer->SetCurrentBlockHeight(_lastBlock->GetHeight());
				PublishPendingTx(peer);

				if (_lastBlock->GetHeight() < peer->GetLastBlock()) { // start blockchain sync
					peer->ScheduleDisconnect(PROTOCOL_TIMEOUT); // schedule sync timeout
					// request just block headers up to a week before earliestKeyTime, and then merkleblocks after that
					// we do not reset connect failure count yet incase this request times out
					peer->SendMessage(MSG_GETBLOCKS, GetBlocksParameter(GetBlockLocators(), uint256()));
				} else { // we're already synced
					LoadMempools();
				}
			}

			status = GetConnectStatusInternal();
			if (_connectStatus != status) {
				_connectStatus = status;
				connectionStatusChanged = true;
			}

			lock.unlock();
			if (connectionStatusChanged) FireConnectStatusChanged(status);
		}

		void PeerManager::OnDisconnected(const PeerPtr &peer, int error) {
			int willSave = 0, txError = 0;
			bool willReconnect = false, isBlack = false, connectionStatusChanged = false;
			Peer::ConnectStatus status = Peer::Disconnected;

			{
				boost::mutex::scoped_lock scopedLock(lock);

				if (error == EPROTO) { // if it's protocol error, the peer isn't following standard policy
					_connectFailureCount++;
					PeerMisbehaving(peer);
				} else if (error) { // timeout or some non-protocol related network error
					RemovePeer(peer);
					_connectFailureCount++;

					// if it's a timeout and there's pending tx publish callbacks, the tx publish timed out
					// BUG: XXX what if it's a connect timeout and not a publish timeout?
					if (error == ETIMEDOUT &&
						(peer != _downloadPeer || _syncStartHeight == 0 || _connectedPeers.size() == 1))
						txError = ETIMEDOUT;
				}

				for (size_t i = _txRelays.size(); i > 0; i--) {
					_txRelays[i - 1].RemovePeer(peer);
				}

				if (_blackPeers.find(peer->GetPeerInfo()) != _blackPeers.end()) {
					RemovePeer(peer);
					isBlack = true;
				}

				if (peer == _downloadPeer) { // download peer disconnected
					_isConnected = 0;
					_downloadPeer = NULL;
					if (_connectFailureCount > MAX_CONNECT_FAILURES)
						_connectFailureCount = MAX_CONNECT_FAILURES;
				}

				if (!_isConnected && _connectFailureCount >= MAX_CONNECT_FAILURES) {
					SyncStopped();

					// clear out stored peers so we get a fresh list from DNS on next connect attempt
					_peers.clear();
					txError = ENOTCONN; // trigger any pending tx publish callbacks
					willSave = 1;
					peer->warn("sync failed too many times");
				} else if (_enableReconnect && _connectFailureCount < MAX_CONNECT_FAILURES) {
					peer->info("will reconnect");
					willReconnect = true;
				}

				for (std::vector<PeerPtr>::iterator p = _connectedPeers.begin(); p != _connectedPeers.end();) {
					if ((*p) == peer) {
						p = _connectedPeers.erase(p);
						break;
					} else {
						++p;
					}
				}

				status = GetConnectStatusInternal();
				if (_connectStatus != status) {
					_connectStatus = status;
					connectionStatusChanged	= true;
				}
				PEER_INFO(peer, "connected peer size: {}", _connectedPeers.size());
			}

			if (connectionStatusChanged) FireConnectStatusChanged(status);
			if (willSave) FireSavePeers(true, {});
			if (willSave) FireSyncStopped(error);
			if (isBlack) FireSaveBlackPeer(peer->GetPeerInfo());
			if (willReconnect) ConnectLaster(1);
			FireTxStatusUpdate();
		}

		void PeerManager::OnRelayedPeers(const PeerPtr &peer, const std::vector<PeerInfo> &peers) {
			time_t now = time(NULL);
			size_t peersCount;
			std::vector<PeerInfo> save;

			if (_chainID == CHAINID_IDCHAIN && _netType == "MainNet") {
				peer->info("do not relay IDChain's peers for now");
				return;
			}

			{
				boost::mutex::scoped_lock scopedLock(lock);
				peer->info("relayed {} peer(s), {} peer(s) before save", peers.size(), _peers.size());

				std::set<PeerInfo> uniquePeers;
				uniquePeers.insert(_peers.begin(), _peers.end());
				uniquePeers.insert(peers.begin(), peers.end());

				for (const PeerInfo &p : uniquePeers) {
					if (_blackPeers.find(p) == _blackPeers.end()) {
						save.push_back(p);
					}
				}

				std::sort(save.begin(), save.end(), [](const PeerInfo &first, const PeerInfo &second) {
					return first.Timestamp > second.Timestamp;
				});

				// limit total to 2500 peers
				if (save.size() > 2500) save.resize(2500);
				peersCount = save.size();

//				while (peersCount > 200 && save[peersCount - 1].Timestamp + 30 * 24 * 3600 < now) peersCount--;
//				save.resize(peersCount);

				// remove peers more than 3 hours old, or until there are only 1000 left
				while (peersCount > 500 && save[peersCount - 1].Timestamp + 3 * 60 * 60 < now) peersCount--;
				save.resize(peersCount);

				_peers = save;
			}

			if (!save.empty()) {
				PEER_DEBUG(peer, "save {} peer(s)", save.size());
				FireSavePeers(true, save);
			}
		}

		void PeerManager::OnRelayedTx(const PeerPtr &peer, const TransactionPtr &transaction) {
			int isWalletTx = 0, hasPendingCallbacks = 0;
			size_t relayCount = 0;
			TransactionPtr tx = transaction;
			UTXOPtr coinBase;
			PublishedTransaction pubTx;

			{
				_wallet->StripTransaction(tx);
				boost::mutex::scoped_lock scopedLock(lock);
				peer->info("relayed tx");

				for (size_t i = _publishedTx.size(); i > 0; i--) { // see if tx is in list of published tx
					if (_publishedTxHashes[i - 1] == tx->GetHash()) {
						pubTx = _publishedTx[i - 1];
						_publishedTx[i - 1].ResetCallback();
						relayCount = AddPeerToList(peer, tx->GetHash(), _txRelays);
					} else if (_publishedTx[i - 1].HasCallback()) hasPendingCallbacks = 1;
				}

				// cancel tx publish timeout if no publish callbacks are pending, and syncing is done or this is not downloadPeer
				if (!hasPendingCallbacks && (_syncStartHeight == 0 || peer != _downloadPeer)) {
					peer->ScheduleDisconnect(-1); // cancel publish tx timeout
				}

				if (_syncStartHeight == 0 || _wallet->ContainsTransaction(tx)) {
					isWalletTx = _wallet->RegisterTransaction(tx);
					if (isWalletTx)
						tx = _wallet->TransactionForHash(tx->GetHash());
				} else {
					tx = nullptr;
				}

				if (tx && isWalletTx) {
					// reschedule sync timeout
					if (_syncStartHeight > 0 && peer == _downloadPeer) {
						peer->ScheduleDisconnect(PROTOCOL_TIMEOUT);
					}

					if (_syncSucceeded && _wallet->AmountSentByTx(tx) > 0 &&
						_wallet->TransactionIsValid(tx)) {
						AddTxToPublishList(tx, Peer::PeerPubTxCallback());  // add valid send tx to mempool
					}

					// keep track of how many peers have or relay a tx, this indicates how likely the tx is to confirm
					// (we only need to track this after syncing is complete)
					if (_syncStartHeight == 0)
						relayCount = AddPeerToList(peer, tx->GetHash(), _txRelays);

					RemovePeerFromList(peer, tx->GetHash(), _txRequests);

					if (_bloomFilter != nullptr) { // check if bloom filter is already being updated

						// the transaction likely consumed one or more wallet addresses, so check that at least the next <gap limit>
						// unused addresses are still matched by the bloom filter
						AddressArray unusedAddrs = _wallet->UnusedAddresses(SEQUENCE_GAP_LIMIT_EXTERNAL, 0);
						AddressArray internalAddrs = _wallet->UnusedAddresses(SEQUENCE_GAP_LIMIT_INTERNAL, 1);
						unusedAddrs.insert(unusedAddrs.end(), internalAddrs.begin(), internalAddrs.end());

						bytes_t hash;

						for (AddressArray::iterator it = unusedAddrs.begin(); it != unusedAddrs.end(); ++it) {
							hash = (*it)->ProgramHash().bytes();
							if (!_bloomFilter->ContainsData(hash)) {
								_bloomFilter.reset();
								UpdateBloomFilter();
								break;
							}
						}
					}
				}

				// set timestamp when tx is verified
				if (tx && relayCount >= _maxConnectCount && tx->GetBlockHeight() == TX_UNCONFIRMED &&
					tx->GetTimestamp() == 0) {
					_wallet->UpdateTransactions({tx->GetHash()}, TX_UNCONFIRMED, time(NULL));
				} else if (coinBase && coinBase->BlockHeight() == TX_UNCONFIRMED && coinBase->Timestamp() == 0) {
					_wallet->UpdateTransactions({coinBase->Hash()}, TX_UNCONFIRMED, time(NULL));
				}
			}

			if (pubTx.HasCallback()) pubTx.FireCallback(0, "success");
		}

		void PeerManager::OnHasTx(const PeerPtr &peer, const uint256 &txHash) {
			int isWalletTx = 0, hasPendingCallbacks = 0;
			size_t relayCount = 0;
			PublishedTransaction pubTx;

			{
				boost::mutex::scoped_lock scopedLock(lock);
				TransactionPtr tx = _wallet->TransactionForHash(txHash);
				peer->info("has tx");

				for (size_t i = _publishedTx.size(); i > 0; i--) { // see if tx is in list of published tx
					if (_publishedTxHashes[i - 1] == txHash) {
						if (!tx) tx = _publishedTx[i - 1].GetTransaction();
						pubTx = _publishedTx[i - 1];
						_publishedTx[i - 1].ResetCallback();
						relayCount = AddPeerToList(peer, txHash, _txRelays);
					} else if (_publishedTx[i - 1].HasCallback()) hasPendingCallbacks = 1;
				}

				// cancel tx publish timeout if no publish callbacks are pending, and syncing is done or this is not downloadPeer
				if (!hasPendingCallbacks && (_syncStartHeight == 0 || peer != _downloadPeer)) {
					peer->ScheduleDisconnect(-1);  // cancel publish tx timeout
				}

				if (tx) {
					isWalletTx = _wallet->RegisterTransaction(tx);
					if (isWalletTx) tx = _wallet->TransactionForHash(tx->GetHash());

					// reschedule sync timeout
					if (_syncStartHeight > 0 && peer == _downloadPeer && isWalletTx) {
						peer->ScheduleDisconnect(PROTOCOL_TIMEOUT);
					}

					// keep track of how many peers have or relay a tx, this indicates how likely the tx is to confirm
					// (we only need to track this after syncing is complete)
					if (_syncStartHeight == 0)
						relayCount = AddPeerToList(peer, txHash, _txRelays);

					// set timestamp when tx is verified
					if (relayCount >= _maxConnectCount && tx && tx->GetBlockHeight() == TX_UNCONFIRMED &&
						tx->GetTimestamp() == 0) {
						std::vector<uint256> hashes = {txHash};
						_wallet->UpdateTransactions(hashes, TX_UNCONFIRMED, (uint32_t) time(NULL));
					}

					RemovePeerFromList(peer, txHash, _txRequests);
				}
			}

			if (pubTx.HasCallback()) pubTx.FireCallback(0, "has tx");
		}

		void PeerManager::OnRejectedTx(const PeerPtr &peer, const uint256 &txHash, uint8_t code, const std::string &reason) {
			TransactionPtr tx;
			PublishedTransaction pubTx;
			{
				boost::mutex::scoped_lock scopedLock(lock);
				peer->info("rejected tx: code {}, reason {}", code, reason);
				tx = _wallet->TransactionForHash(txHash);
				RemovePeerFromList(peer, txHash, _txRequests);

				for (size_t i = _publishedTx.size(); i > 0; --i) { // see if tx is in list of published tx
					if (_publishedTxHashes[i - 1] == txHash) {
						pubTx = _publishedTx[i - 1];
						_publishedTx[i - 1].ResetCallback();
						_publishedTx.erase(_publishedTx.begin() + (i - 1));
						_publishedTxHashes.erase(_publishedTxHashes.begin() + (i - 1));
						break;
					}
				}

				if (tx) {
					if (RemovePeerFromList(peer, txHash, _txRelays) && tx->GetBlockHeight() == TX_UNCONFIRMED) {
						// set timestamp 0 to mark tx as unverified
						_wallet->UpdateTransactions({txHash}, TX_UNCONFIRMED, 0);
					}

					// if we get rejected for any reason other than double-spend, the peer is likely misconfigured
#if 0 // TODO enable this after node error code refactored.
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
#endif
				}
			}

			FireTxStatusUpdate();
			if (pubTx.HasCallback()) pubTx.FireCallback(code, reason);
		}

		void PeerManager::OnRelayedBlock(const PeerPtr &peer, const MerkleBlockPtr &block) {
			size_t i, j, fpCount = 0, saveCount = 0;
			MerkleBlockPtr b, b2, prev, next;
			std::vector<MerkleBlockPtr> saveBlocks;
			std::vector<uint256> txHashes;
			static uint32_t txTotal = 0; // for test
			block->MerkleBlockTxHashes(txHashes);

			{
				boost::mutex::scoped_lock scopedLock(lock);
				prev = _blocks.Get(block->GetPrevBlockHash());

				if (prev) {
					block->SetHeight(prev->GetHeight() + 1);
				}

				// track the observed bloom filter false positive rate using a low pass filter to smooth out variance
				if (peer == _downloadPeer && block->GetTransactionCount() > 0) {
					for (i = 0; i < txHashes.size(); i++) { // wallet tx are not false-positives
						if (!_wallet->ContainsTransaction(txHashes[i]))
							fpCount++;
					}

					// moving average number of tx-per-block
					_averageTxPerBlock = _averageTxPerBlock * 0.999 + block->GetTransactionCount() * 0.001;

					// 1% low pass filter, also weights each block by total transactions, compared to the avarage
					_fpRate = _fpRate * (1.0 - 0.01 * block->GetTransactionCount() / _averageTxPerBlock) +
							 0.01 * fpCount / _averageTxPerBlock;

					// false positive rate sanity check
					if (peer->GetConnectStatus() == Peer::Connected &&
						_fpRate > BLOOM_DEFAULT_FALSEPOSITIVE_RATE * 10.0) {
						peer->warn(
							"bloom filter false positive rate {} too high after {} blocks, disconnecting...",
							_fpRate, _lastBlock->GetHeight() + 1 - _filterUpdateHeight);
						_fpRate = 0;
						_averageTxPerBlock = 1400;
						peer->Disconnect();
						return;
					} else if (_lastBlock->GetHeight() + 500 < peer->GetLastBlock() &&
							   _fpRate > BLOOM_REDUCED_FALSEPOSITIVE_RATE * 10.0) {
						UpdateBloomFilter(); // rebuild bloom filter when it starts to degrade
					}
				}

				// ignore block headers that are newer than one week before earliestKeyTime (it's a header if it has 0 totalTx)
				if (block->GetTransactionCount() == 0 &&
					block->GetTimestamp() + 7 * 24 * 60 * 60 > _earliestKeyTime + 2 * 60 * 60) {
				} else if (_bloomFilter ==
						   nullptr) { // ingore potentially incomplete blocks when a filter update is pending

					if (peer == _downloadPeer && _lastBlock->GetHeight() < _estimatedHeight) {
						peer->ScheduleDisconnect(PROTOCOL_TIMEOUT); // reschedule sync timeout
						_connectFailureCount = 0; // reset failure count once we know our initial request didn't timeout
					}
				} else if (!prev) { // block is an orphan
					PEER_DEBUG(peer, "relayed orphan block {}:{}, previous {}, last block is {}:{}",
							   block->GetHash().GetHex(),
							   block->GetHeight(),
							   block->GetPrevBlockHash().GetHex(),
							   _lastBlock->GetHash().GetHex(),
							   _lastBlock->GetHeight());

					if (block->GetTimestamp() + 7 * 24 * 60 * 60 < time(nullptr)) { // ignore orphans older than one week ago
						peer->info("ignore orphan #{}", block->GetHeight());
					} else {
						// call getblocks, unless we already did with the previous block, or we're still syncing
						if (_lastBlock->GetHeight() >= peer->GetLastBlock() &&
							(!_lastOrphan || !_lastOrphan->IsEqual(block.get()))) {
							peer->info("calling getblocks");
							GetBlocksParameter getBlocksParameter(GetBlockLocators(), uint256());
							peer->SendMessage(MSG_GETBLOCKS, getBlocksParameter);
						}

						_orphans.Insert(block); // BUG: limit total orphans to avoid memory exhaustion attack
						_lastOrphan = block;
						peer->ScheduleDisconnect(PROTOCOL_TIMEOUT); // reschedule sync timeout
					}
				} else if (!VerifyBlock(block, prev, peer)) { // block is invalid
					peer->warn("relayed invalid block");
					PeerMisbehaving(peer);
				} else if (block->GetPrevBlockHash() == _lastBlock->GetHash()) { // new block extends main chain
					_blocks.Insert(block);
					_lastBlock = block;
					_wallet->SetBlockHeight(_lastBlock->GetHeight());

					txTotal += block->GetTotalTx();
					if ((block->GetHeight() % 500) == 0 || txHashes.size() > 0 ||
						block->GetHeight() >= peer->GetLastBlock()) {
						peer->info("adding block #{}, {}, false positive rate: {}",
								   block->GetHeight(), txTotal, _fpRate);
						FireSyncProgress(GetSyncProgressInternal(0), peer, block);
						txTotal = 0;
					}

					if (txHashes.size() > 0)
						_wallet->UpdateTransactions(txHashes, block->GetHeight(), block->GetTimestamp());
					if (_downloadPeer) _downloadPeer->SetCurrentBlockHeight(block->GetHeight());

					if (block->GetHeight() < _estimatedHeight && peer == _downloadPeer) {
						peer->ScheduleDisconnect(PROTOCOL_TIMEOUT); // reschedule sync timeout
						_connectFailureCount = 0; // reset failure count once we know our initial request didn't timeout
					}

					if ((block->GetHeight() % BLOCK_DIFFICULTY_INTERVAL) == 0)
						saveCount = 1; // save transition block immediately

					if (block->GetHeight() == _estimatedHeight) { // chain download is complete
						saveCount = (block->GetHeight() % BLOCK_DIFFICULTY_INTERVAL) + BLOCK_DIFFICULTY_INTERVAL + 1;
						LoadMempools();
					}
				} else if (_blocks.Contains(block)) { // we already have the block (or at least the header)
					if ((block->GetHeight() % 500) == 0 || txHashes.size() > 0 ||
						block->GetHeight() >= peer->GetLastBlock()) {
						peer->info("relayed existing block #{}", block->GetHeight());
					}

					b = _lastBlock;
					while (b && b->GetHeight() > block->GetHeight())
						b = _blocks.Get(b->GetPrevBlockHash()); // is block in main chain?

					if (b->IsEqual(block.get())) { // if it's not on a fork, set block heights for its transactions
						if (txHashes.size() > 0)
							_wallet->UpdateTransactions(txHashes, block->GetHeight(), block->GetTimestamp());
						if (block->GetHeight() == _lastBlock->GetHeight()) _lastBlock = block;
					}

					b = _blocks.Get(block->GetHash());
					if (b != nullptr) _blocks.Remove(b);
					_blocks.Insert(block);

					if (b != nullptr && b != block) {
						_orphans.Remove(b);
						if (_lastOrphan == b) _lastOrphan = nullptr;
					}
				} else if (_lastBlock->GetHeight() < peer->GetLastBlock() &&
					block->GetHeight() >
							   _lastBlock->GetHeight() + 1) { // special case, new block mined durring rescan
					peer->info("marking new block #{} as orphan until rescan completes", block->GetHeight());
					_orphans.Insert(block); // mark as orphan til we're caught up
					_lastOrphan = block;
				} else if (block->GetHeight() <= _chainParams->LastCheckpoint().Height()) { // old fork
					peer->info("ignoring block on fork older than most recent checkpoint, block #{}, hash: {}",
							   block->GetHeight(), block->GetHash().GetHex());
				} else { // new block is on a fork
					peer->warn("chain fork reached height {}", block->GetHeight());
					_blocks.Insert(block);

					if (block->GetHeight() > _lastBlock->GetHeight()) { // check if fork is now longer than main chain
						b = block;
						b2 = _lastBlock;
						std::vector<MerkleBlockPtr> longerChain;
						while (b && b2 && !b->IsEqual(b2.get())) { // walk back to where the fork joins the main chain
							longerChain.insert(longerChain.begin(), b);
							b = _blocks.Get(b->GetPrevBlockHash());
							if (b && b->GetHeight() < b2->GetHeight()) b2 = _blocks.Get(b2->GetPrevBlockHash());
						}

						peer->info("reorganizing chain from height {}, new height is {}", b->GetHeight(),
								   block->GetHeight());

						_wallet->SetTxUnconfirmedAfter(b->GetHeight());  // mark tx after the join point as unconfirmed

						for (std::vector<MerkleBlockPtr>::iterator it = longerChain.begin(); it != longerChain.end(); ++it) {
							b = *it;
							if (b2->GetHash() == b->GetPrevBlockHash()) {
								uint32_t height = b->GetHeight();
								uint32_t timestamp = b->GetTimestamp();
								txHashes.clear();
								b->MerkleBlockTxHashes(txHashes);
								if (!txHashes.empty())
									_wallet->UpdateTransactions(txHashes, height, timestamp);
								b2 = b;
							}
						}

						_lastBlock = block;
						_wallet->SetBlockHeight(_lastBlock->GetHeight());

						if (block->GetHeight() == _estimatedHeight) { // chain download is complete
							saveCount =
									(block->GetHeight() % BLOCK_DIFFICULTY_INTERVAL) + BLOCK_DIFFICULTY_INTERVAL + 1;
							LoadMempools();
						}
					}
				}

				if (block && block->GetHeight() != BLOCK_UNKNOWN_HEIGHT) {
					if (block->GetHeight() > _estimatedHeight) _estimatedHeight = block->GetHeight();

					// check if the next block was received as an orphan
					MerkleBlockPtr nextBlock = _orphans.GetMatchPrevHash(block->GetHash());
					if (nextBlock) {
						next = nextBlock;
						_orphans.Remove(nextBlock);
					}
				}

				saveBlocks.clear();

				for (i = 0, b = block; b && i < saveCount; i++) {
					assert(b->GetHeight() != BLOCK_UNKNOWN_HEIGHT); // verify all blocks to be saved are in the chain
					MerkleBlockPtr prvBlock = _blocks.Get(b->GetPrevBlockHash());
					if (prvBlock)
						saveBlocks.push_back(b);
					b = prvBlock;
				}

				// make sure the set of blocks to be saved starts at a difficulty interval
				j = (saveBlocks.size() > 0) ? saveBlocks.back()->GetHeight() % BLOCK_DIFFICULTY_INTERVAL : 0;
				if (j > 0) {
					if (saveBlocks.size() > BLOCK_DIFFICULTY_INTERVAL - j) {
						saveBlocks.resize(saveBlocks.size() - (BLOCK_DIFFICULTY_INTERVAL - j));
//					} else {
//						saveBlocks.clear();
					}
				}
//				assert(saveBlocks.size() == 0 || (saveBlocks.back()->GetHeight() % BLOCK_DIFFICULTY_INTERVAL) == 0);
			}

			if (saveBlocks.size() > 0)
				FireSaveBlocks(saveBlocks.size() > 1, saveBlocks);

			if (block && block->GetHeight() != BLOCK_UNKNOWN_HEIGHT) {
				_wallet->UpdateLockedBalance();
			}

			if (next) OnRelayedBlock(peer, next);
		}

		void PeerManager::OnRelayedPing(const PeerPtr &peer) {
			bool needReconnect = false;
			time_t seconds = 0;

			lock.lock();
			if (_syncSucceeded && time_after(time(NULL), _keepAliveTimestamp + 30) && 0 == PublishPendingTx(peer)) {
				seconds = _reconnectSeconds;
				needReconnect = true;
			}
			lock.unlock();

			if (needReconnect) {
				boost::thread workThread(boost::bind(&PeerManager::ReconnectLaster, this, seconds));
			}
		}

		void PeerManager::OnNotfound(const PeerPtr &peer, const std::vector<uint256> &txHashes,
									 const std::vector<uint256> &blockHashes) {
			boost::mutex::scoped_lock scopedLock(lock);
			for (size_t i = 0; i < txHashes.size(); i++) {
				RemovePeerFromList(peer, txHashes[i], _txRelays);
				RemovePeerFromList(peer, txHashes[i], _txRequests);
			}
		}

		void PeerManager::OnSetFeePerKb(const PeerPtr &peer, uint64_t feePerKb) {
			uint64_t maxFeePerKb = 0, secondFeePerKb = 0;

			{
				boost::mutex::scoped_lock scopedLock(lock);
				for (size_t i = _connectedPeers.size(); i > 0; i--) { // find second highest fee rate
					const PeerPtr &p = _connectedPeers[i - 1];
					if (p->GetConnectStatus() != Peer::Connected) continue;
					if (p->GetFeePerKb() > maxFeePerKb) secondFeePerKb = maxFeePerKb, maxFeePerKb = p->GetFeePerKb();
				}

				if (secondFeePerKb * 3 / 2 > DEFAULT_FEE_PER_KB && secondFeePerKb * 3 / 2 <= MAX_FEE_PER_KB &&
					secondFeePerKb * 3 / 2 > _wallet->GetFeePerKb()) {
					peer->info("increasing feePerKb to {} based on feefilter messages from peers",
							   secondFeePerKb * 3 / 2);
					_wallet->SetFeePerKb(secondFeePerKb * 3 / 2);
				}
			}
		}

		TransactionPtr PeerManager::OnRequestedTx(const PeerPtr &peer, const uint256 &txHash) {
			int hasPendingCallbacks = 0, error = 0;
			PublishedTransaction pubTx;

			{
				boost::mutex::scoped_lock scopedLock(lock);
				for (size_t i = _publishedTx.size(); i > 0; i--) {
					if (_publishedTx[i - 1].GetTransaction()->GetHash() == txHash) {
						pubTx = _publishedTx[i - 1];
						//_publishedTx[i - 1].ResetCallback();
					} else if (_publishedTx[i - 1].HasCallback())
						hasPendingCallbacks = 1;
				}

				// cancel tx publish timeout if no publish callbacks are pending, and syncing is done or this is not downloadPeer
				if (!hasPendingCallbacks && (_syncStartHeight == 0 || peer != _downloadPeer)) {
					peer->ScheduleDisconnect(-1); // cancel publish tx timeout
				}

				//AddPeerToList(peer, txHash, _txRelays);
				if (pubTx.GetTransaction() != nullptr)
					_wallet->RegisterTransaction(pubTx.GetTransaction());
				if (pubTx.GetTransaction() != nullptr && !_wallet->TransactionIsValid(pubTx.GetTransaction()))
					error = 0x10; // RejectInvalid by node
			}

			if (pubTx.HasCallback()) pubTx.FireCallback(error, "tx is requested");
			return pubTx.GetTransaction();
		}

		bool PeerManager::OnNetworkIsReachable(const PeerPtr &peer) {
			return FireNetworkIsReachable();
		}

		void PeerManager::OnThreadCleanup(const PeerPtr &peer) {
		}

		void PeerManager::RemovePeer(const PeerPtr &peer) {
			for (std::vector<PeerInfo>::iterator p = _peers.begin(); p != _peers.end();) {
				if ((*p) == peer->GetPeerInfo()) {
					p = _peers.erase(p);
					break;
				} else {
					++p;
				}
			}
		}

		size_t PeerManager::PublishPendingTx(const PeerPtr &peer) {
			std::vector<uint256> pendingHashes;

			for (size_t i = _publishedTx.size(); i > 0; i--) {
				if (!_publishedTx[i - 1].HasCallback() ||
					_publishedTx[i - 1].GetTransaction()->GetBlockHeight() != TX_UNCONFIRMED)
					continue;
				peer->ScheduleDisconnect(PROTOCOL_TIMEOUT);  // schedule publish timeout
				pendingHashes.push_back(_publishedTx[i - 1].GetTransaction()->GetHash());
			}

			InventoryParameter inventoryParameter;
			inventoryParameter.txHashes = pendingHashes;
			peer->SendMessage(MSG_INV, inventoryParameter);

			return pendingHashes.size();
		}

		size_t
		PeerManager::AddPeerToList(const PeerPtr &peer, const uint256 &txHash, std::vector<TransactionPeerList> &list) {
			for (size_t i = list.size(); i > 0; i--) {
				if (list[i - 1].GetTransactionHash() != txHash)
					continue;

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

		bool PeerManager::RemovePeerFromList(const PeerPtr &peer, const uint256 &txHash,
											 std::vector<TransactionPeerList> &list) {
			bool removed = false;
			for (size_t i = list.size(); i > 0; i--) {
				if (list[i - 1].GetTransactionHash() == txHash) {
					removed = list[i - 1].RemovePeer(peer);
					break;
				}
			}

			return removed;
		}

		void PeerManager::PeerMisbehaving(const PeerPtr &peer) {
			RemovePeer(peer);

			if (++_misbehavinCount >= 10) { // clear out stored peers so we get a fresh list from DNS for next connect
				_misbehavinCount = 0;
				_peers.clear();
				FireSavePeers(true, {});
			}

			peer->Disconnect();
		}

		const std::string &PeerManager::GetChainID() const {
			return _chainID;
		}

		const std::vector<PeerInfo> &PeerManager::GetPeers() const {
			return _peers;
		}

		void PeerManager::SetPeers(const std::vector<PeerInfo> &peers) {
			_peers = peers;
		}

		const std::string &PeerManager::GetID() const {
			return _wallet->GetWalletID();
		}

		void PeerManager::UpdateBloomFilter() {

			if (_downloadPeer && (_downloadPeer->GetFlags() & PEER_FLAG_NEEDSUPDATE) == 0) {
				_downloadPeer->SetNeedsFilterUpdate(true);
				_downloadPeer->SetFlags(_downloadPeer->GetFlags() | PEER_FLAG_NEEDSUPDATE);
				_downloadPeer->info("filter update needed, waiting for pong");
				// wait for pong so we're sure to include any tx already sent by the peer in the updated filter
				PingParameter pingParam(_lastBlock->GetHeight(),
										boost::bind(&PeerManager::UpdateFilterPingDone, this, _downloadPeer, _1));
				_downloadPeer->SendMessage(MSG_PING, pingParam);
			}
		}

		void PeerManager::UpdateFilterRerequestDone(const PeerPtr &peer, int success) {
			if (!success) return;

			boost::mutex::scoped_lock scopedLock(lock);
			if ((peer->GetFlags() & PEER_FLAG_NEEDSUPDATE) == 0) {
				peer->SendMessage(MSG_GETBLOCKS, GetBlocksParameter(GetBlockLocators(), uint256()));
			}
		}

		void PeerManager::UpdateFilterLoadDone(const PeerPtr &peer, int success) {
			if (!success) return;

			peer->info("update filter load done");
			boost::mutex::scoped_lock scopedLock(lock);
			peer->SetNeedsFilterUpdate(false);
			peer->SetFlags(peer->GetFlags() & (uint8_t)(~PEER_FLAG_NEEDSUPDATE));

			if (_lastBlock->GetHeight() < _estimatedHeight) { // if syncing, rerequest blocks
				_downloadPeer->RerequestBlocks(_lastBlock->GetHash());
				PingParameter pingParam(_lastBlock->GetHeight(),
										boost::bind(&PeerManager::UpdateFilterRerequestDone, this, _downloadPeer, _1));
				_downloadPeer->SendMessage(MSG_PING, pingParam);
			} else {
				MempoolParameter mempoolParameter;
				mempoolParameter.KnownTxHashes = {};
				mempoolParameter.CompletionCallback = boost::function<void(int)>();
				peer->SendMessage(MSG_MEMPOOL, mempoolParameter);
			}
		}

		void PeerManager::UpdateFilterPingDone(const PeerPtr &peer, int success) {
			if (!success) return;

			boost::mutex::scoped_lock scopedLock(lock);
			peer->info("updating filter with newly created wallet addresses");
			_bloomFilter = nullptr;

			if (_lastBlock->GetHeight() < _estimatedHeight) { // if we're syncing, only update download peer
				if (_downloadPeer) {
					LoadBloomFilter(_downloadPeer);
					PingParameter pingParam(_lastBlock->GetHeight(),
											boost::bind(&PeerManager::UpdateFilterLoadDone, this, _downloadPeer, _1));
					_downloadPeer->SendMessage(MSG_PING, pingParam);// wait for pong so filter is loaded
				}
			} else {
				for (size_t i = _connectedPeers.size(); i > 0; i--) {
					if (_connectedPeers[i - 1]->GetConnectStatus() != Peer::Connected)
						continue;

					PingParameter pingParam(_lastBlock->GetHeight(),
											boost::bind(&PeerManager::UpdateFilterLoadDone, this, _connectedPeers[i - 1], _1));
					LoadBloomFilter(peer);
					_downloadPeer->SendMessage(MSG_PING, pingParam);// wait for pong so filter is loaded
				}
			}
		}

		void PeerManager::UpdateAddressOnlyDone(const PeerPtr &peer, int success) {
			PEER_INFO(peer, "use new addresses to reconnect");
			peer->Disconnect();
		}

		void PeerManager::LoadBloomFilterDone(const PeerPtr &peer, int success) {

			lock.lock();
			if (success) {
				MempoolParameter mempoolParameter;
				mempoolParameter.KnownTxHashes = _publishedTxHashes;
				mempoolParameter.CompletionCallback = boost::bind(&PeerManager::MempoolDone, this, peer, _1);
				peer->SendMessage(MSG_MEMPOOL, mempoolParameter);
				lock.unlock();
			} else {

				if (peer == _downloadPeer) {
					peer->info("sync succeeded");
					_keepAliveTimestamp = time(nullptr);
					_syncSucceeded = true;
					SyncStopped();
					lock.unlock();
					FireSyncStopped(0);
				} else lock.unlock();
			}
		}

		std::vector<uint128> PeerManager::AddressLookup(const std::string &hostname) {
			struct addrinfo hints, *servinfo, *p;
			std::vector<uint128> addrList;

			memset(&hints, 0, sizeof(hints));
			hints.ai_socktype = SOCK_STREAM;
			hints.ai_family = PF_UNSPEC;
			if (getaddrinfo(hostname.c_str(), NULL, &hints, &servinfo) == 0) {
				for (p = servinfo; p != NULL; p = p->ai_next) {
					uint128 addr;
					char host[INET6_ADDRSTRLEN];
					if (p->ai_family == AF_INET) {
						*(uint16_t *)&addr.begin()[10] = 0xffff;
						*(uint32_t *)&addr.begin()[12] = ((struct sockaddr_in *) p->ai_addr)->sin_addr.s_addr;
						inet_ntop(AF_INET, &addr.begin()[12], host, sizeof(host));
					} else if (p->ai_family == AF_INET6) {
						memcpy(addr.begin(), &((struct sockaddr_in6 *) p->ai_addr)->sin6_addr, addr.size());
						inet_ntop(AF_INET6, addr.begin(), host, sizeof(host));
					}
					Log::debug("{} {} -> {}", GetID(), hostname, host);
					addrList.push_back(addr);
				}

				freeaddrinfo(servinfo);
			}

			addrList.shrink_to_fit();
			return addrList;
		}

		bool PeerManager::VerifyBlock(const MerkleBlockPtr &block, const MerkleBlockPtr &prev, const PeerPtr &peer) {
			bool r = true;

			if (!prev || block->GetPrevBlockHash() != prev->GetHash() ||
				block->GetHeight() != prev->GetHeight() + 1)
				r = false;

			// check if we hit a difficulty transition, and find previous transition time
			if (r && (block->GetHeight() % BLOCK_DIFFICULTY_INTERVAL) == 0) {
				MerkleBlockPtr b = block;
				uint256 prevBlock;

				for (uint32_t i = 0; b && i < BLOCK_DIFFICULTY_INTERVAL; i++) {
					b = _blocks.Get(b->GetPrevBlockHash());
				}

				if (!b) {
					peer->warn("missing previous difficulty tansition, block height: {}", block->GetHeight());
//					peer->warn("missing previous difficulty tansition, can't verify block: {}",
//							   block->GetHash().GetHex());
					//r = false;
				} else prevBlock = b->GetPrevBlockHash();

				while (b) { // free up some memory
					b = _blocks.Get(prevBlock);
					if (b) prevBlock = b->GetPrevBlockHash();

					if (b && (b->GetHeight() % BLOCK_DIFFICULTY_INTERVAL) != 0) {
						_blocks.Remove(b);
					}
				}
			}

			//fixme figure out why difficult validation fails occasionally
			if (0 && VerifyDifficulty(_chainParams, block, _blocks)) {
				peer->error("relayed block with invalid difficulty target {}, blockHash: {}", block->GetTarget(),
							block->GetHash().GetHex());
				r = false;
			}

			if (r) {
				const MerkleBlockPtr &checkpoint = _checkpoints.Get(block->GetHash());

				// verify blockchain checkpoints
				if (checkpoint && !block->IsEqual(checkpoint.get())) {
					peer->warn("relayed a block that differs from the checkpoint at height {}, blockHash: {}, "
							   "expected: {}", block->GetHeight(), block->GetHash().GetHex(),
							   checkpoint->GetHash().GetHex());
					r = false;
				}
			}

			return r;
		}

		std::vector<uint256> PeerManager::GetBlockLocators() {
			// append 10 most recent block hashes, decending, then continue appending, doubling the step back each time,
			// finishing with the genesis block (top, -1, -2, -3, -4, -5, -6, -7, -8, -9, -11, -15, -23, -39, -71, -135, ..., 0)
			MerkleBlockPtr block = _lastBlock;
			int32_t step = 1, i = 0, j;

			std::vector<uint256> locators;
			while (block != nullptr && block->GetHeight() > 0) {
				locators.push_back(block->GetHash());
				if (++i >= 10) step *= 2;

				for (j = 0; block && j < step; j++) {
					block = _blocks.Get(block->GetPrevBlockHash());
				}
			}

			locators.push_back(_chainParams->FirstCheckpoint().Hash());
			return locators;
		}

		void PeerManager::LoadMempools() {
			// after syncing, load filters and get mempools from other peers
			for (size_t i = _connectedPeers.size(); i > 0; i--) {
				const PeerPtr &peer = _connectedPeers[i - 1];

				if (peer->GetConnectStatus() != Peer::Connected) continue;

				if (peer != _downloadPeer || _fpRate > BLOOM_REDUCED_FALSEPOSITIVE_RATE * 5.0) {
					LoadBloomFilter(peer);
					PublishPendingTx(peer);
					PingParameter pingParameter(_lastBlock->GetHeight(),
												boost::bind(&PeerManager::LoadBloomFilterDone, this, peer, _1));
					peer->SendMessage(MSG_PING, pingParameter);
				} else {
					MempoolParameter mempoolParameter;
					mempoolParameter.KnownTxHashes = _publishedTxHashes;
					mempoolParameter.CompletionCallback = boost::bind(&PeerManager::MempoolDone, this, peer, _1);
					peer->SendMessage(MSG_MEMPOOL, mempoolParameter);
				}
			}
		}

		void PeerManager::MempoolDone(const PeerPtr &peer, int success) {
			bool syncFinished = false;

			if (success) {
				peer->info("mempool request finished");
				MerkleBlockPtr block;

				{
					boost::mutex::scoped_lock scopedLock(lock);
					if (_syncStartHeight > 0) {
						peer->info("sync succeeded");
						_keepAliveTimestamp = time(nullptr);
						_syncSucceeded = true;
						syncFinished = true;
						SyncStopped();
					}

					block = _lastBlock;
					RequestUnrelayedTx(peer);
					peer->SendMessage(MSG_GETADDR, Message::DefaultParam);
				}

				FireTxStatusUpdate();
				FireSyncProgress(GetSyncProgressInternal(0), peer, block);
				if (syncFinished) FireSyncStopped(0);
			} else peer->info("mempool request failed");
		}

		void PeerManager::RequestUnrelayedTx(const PeerPtr &peer) {
			std::vector<TransactionPtr> tx = _wallet->TxUnconfirmedBefore(TX_UNCONFIRMED);
			std::vector<uint256> txHashes;

			for (size_t i = 0; i < tx.size(); i++) {
				if (!PeerListHasPeer(_txRelays, tx[i]->GetHash(), peer) &&
					!PeerListHasPeer(_txRequests, tx[i]->GetHash(), peer)) {
					txHashes.push_back(tx[i]->GetHash());
					AddPeerToList(peer, tx[i]->GetHash(), _txRequests);
				}
			}

			if (!txHashes.empty()) {
				GetDataParameter getDataParameter(txHashes, {});
				peer->info("request unrelayed tx {}", txHashes.size());
				peer->SendMessage(MSG_GETDATA, getDataParameter);

				if ((peer->GetPeerInfo().Flags & PEER_FLAG_SYNCED) == 0) {
					PingParameter pingParameter(_lastBlock->GetHeight(),
												boost::bind(&PeerManager::RequestUnrelayedTxGetDataDone, this, peer, _1));
					peer->SendMessage(MSG_PING, pingParameter);
				}
			} else peer->SetFlags(peer->GetFlags() | PEER_FLAG_SYNCED);
		}

		bool PeerManager::PeerListHasPeer(const std::vector<TransactionPeerList> &peerList, const uint256 &txhash,
										  const PeerPtr &peer) {
			for (size_t i = peerList.size(); i > 0; i--) {
				if (peerList[i - 1].GetTransactionHash() != txhash) continue;

				for (size_t j = peerList[i - 1].GetPeers().size(); j > 0; j--) {
					if (peerList[i - 1].GetPeers()[j - 1]->IsEqual(peer.get()))
						return true;
				}

				break;
			}

			return false;
		}

		void PeerManager::RequestUnrelayedTxGetDataDone(const PeerPtr &callbackPeer, int success) {
			bool isPublishing;
			size_t count = 0;
			PeerPtr peer = callbackPeer;

			boost::mutex::scoped_lock scopedLock(lock);
			if (success) peer->SetFlags(peer->GetFlags() | PEER_FLAG_SYNCED);

			for (size_t i = _connectedPeers.size(); i > 0; i--) {
				peer = _connectedPeers[i - 1];
				if (peer->GetConnectStatus() == Peer::Connected) count++;
				if ((peer->GetPeerInfo().Flags & PEER_FLAG_SYNCED) != 0) continue;
				count = 0;
				break;
			}

			// don't remove transactions until we're connected to maxConnectCount peers, and all peers have finished
			// relaying their mempools
			if (count >= _maxConnectCount) {
				uint256 hash;
				std::vector<TransactionPtr> tx = _wallet->TxUnconfirmedBefore(TX_UNCONFIRMED);

				for (size_t i = tx.size(); i > 0; i--) {
					hash = tx[i - 1]->GetHash();
					isPublishing = false;

					for (size_t j = _publishedTx.size(); !isPublishing && j > 0; j--) {
						if (_publishedTx[j - 1].GetTransaction()->IsEqual(*tx[i - 1]) &&
							_publishedTx[j - 1].HasCallback())
							isPublishing = true;
						break;
					}

					if (!isPublishing && PeerListCount(_txRelays, hash) == 0 &&
						PeerListCount(_txRequests, hash) == 0) {
						peer->info("removing tx unconfirmed at: {}, txHash: {}", _lastBlock->GetHeight(), hash.GetHex());
						_wallet->RemoveTransaction(hash);
					} else if (!isPublishing && PeerListCount(_txRelays, hash) < _maxConnectCount) {
						// set timestamp 0 to mark as unverified
						_wallet->UpdateTransactions({hash}, TX_UNCONFIRMED, 0);
					}
				}
			}
		}

		size_t PeerManager::PeerListCount(const std::vector<TransactionPeerList> &list, const uint256 &txhash) {
			for (size_t i = list.size(); i > 0; i--) {
				if (list[i - 1].GetTransactionHash() == txhash)
					return list[i - 1].GetPeers().size();
			}

			return 0;
		}

		void PeerManager::PublishTxInvDone(const PeerPtr &peer, int success) {
			boost::mutex::scoped_lock scopedLock(lock);
			RequestUnrelayedTx(peer);
		}

		void PeerManager::FindPeersThreadRoutine(const std::string &hostname, uint64_t services) {
			std::vector<uint128> addrList = AddressLookup(hostname);
			time_t now = time(NULL);

			boost::mutex::scoped_lock scopedLock(lock);
			for (std::vector<uint128>::iterator addr = addrList.begin(); addr != addrList.end() && (*addr) != 0; addr++) {
				_peers.emplace_back(*addr, _chainParams->StandardPort(), now, services);
			}
			_dnsThreadCount--;
		}

	}
}