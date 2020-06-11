// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef __ELASTOS_SDK_PEERMANAGER_H__
#define __ELASTOS_SDK_PEERMANAGER_H__

#include "Peer.h"
#include "TransactionPeerList.h"
#include "PublishedTransaction.h"

#include <Common/Lockable.h>
#include <WalletCore/BloomFilter.h>
#include <Plugin/Interface/IMerkleBlock.h>
#include <Plugin/Block/MerkleBlock.h>
#include <Plugin/Registry.h>

#include <string>
#include <vector>
#include <boost/weak_ptr.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/function.hpp>
#include <boost/filesystem.hpp>
#include <boost/asio.hpp>

#define PEER_MAX_CONNECTIONS 1

namespace Elastos {
	namespace ElaWallet {

		class Wallet;
		class ChainParams;

		typedef boost::shared_ptr<Wallet> WalletPtr;
		typedef boost::shared_ptr<ChainParams> ChainParamsPtr;
		typedef ElementSet<MerkleBlockPtr> BlockSet;

		class PeerManager :
				public Lockable,
				public Peer::Listener {
		public:

			class Listener {
			public:
				Listener();

				virtual ~Listener() {}

				virtual void syncStarted() = 0;

				virtual void syncProgress(uint32_t progress, time_t lastBlockTime, uint32_t bytesPerSecond, const std::string &downloadPeer) = 0;

				virtual void syncStopped(const std::string &error) = 0;

				virtual void txStatusUpdate() = 0;

				virtual void saveBlocks(bool replace, const std::vector<MerkleBlockPtr> &blocks) = 0;

				virtual void savePeers(bool replace, const std::vector<PeerInfo> &peers) = 0;

				virtual void saveBlackPeer(const PeerInfo &peer) = 0;

				virtual bool networkIsReachable() = 0;

				virtual void txPublished(const std::string &hash, const nlohmann::json &result) = 0;

				virtual void connectStatusChanged(const std::string &status) = 0;
			};

		public:
			PeerManager(const ChainParamsPtr &params,
						const WalletPtr &wallet,
						time_t earliestKeyTime,
						uint32_t reconnectSeconds,
						const std::vector<MerkleBlockPtr> &blocks,
						const std::vector<PeerInfo> &peers,
						const std::set<PeerInfo> &blackPeers,
						const boost::shared_ptr<Listener> &listener,
						const std::string &chainID,
						const std::string &netType);

			~PeerManager();

			void SetWallet(const WalletPtr &wallet);
			/**
			* Connect to bitcoin peer-to-peer network (also call this whenever networkIsReachable()
			* status changes)
			*/
			void Connect();

			void AsyncConnect(const boost::system::error_code &error);

			void ConnectLaster(time_t second);

			void CancelTimer();

			void ClearData();

			/**
			* Disconnect from bitcoin peer-to-peer network (may cause syncFailed(), saveBlocks() or
			* savePeers() callbacks to fire)
			*/
			void Disconnect();

			void Rescan();

			uint32_t GetSyncStartHeight() const;

			uint32_t GetEstimatedBlockHeight() const;

			uint32_t GetLastBlockHeight() const;

			uint32_t GetLastBlockTimestamp() const;

			time_t GetKeepAliveTimestamp() const;

			void SetKeepAliveTimestamp(time_t t);

			double GetSyncProgress(uint32_t startHeight);

			Peer::ConnectStatus GetConnectStatus() const;

			bool SyncSucceeded() const;

			void SetSyncSucceeded(bool succeeded);

			void SetReconnectEnableStatus(bool status);

			bool GetReconnectEnableStatus() const;

			bool SetFixedPeer(const std::string &address, uint16_t port);

			std::string GetCurrentPeerName() const;

			std::string GetDownloadPeerName() const;

			PeerPtr GetDownloadPeer() const;

			size_t GetPeerCount() const;

			void PublishTransaction(const TransactionPtr &transaction);

			void PublishTransaction(const TransactionPtr &transaction, const Peer::PeerPubTxCallback &callback);

			uint64_t GetRelayCount(const uint256 &txHash) const;

			const std::string &GetChainID() const;

			const std::vector<PeerInfo> &GetPeers() const;

			void SetPeers(const std::vector<PeerInfo> &peers);

			const std::string &GetID() const;

		public:
			virtual void OnConnected(const PeerPtr &peer);

			virtual void OnDisconnected(const PeerPtr &peer, int error);

			virtual void OnRelayedPeers(const PeerPtr &peer, const std::vector<PeerInfo> &peers);

			virtual void OnRelayedTx(const PeerPtr &peer, const TransactionPtr &tx);

			virtual void OnHasTx(const PeerPtr &peer, const uint256 &txHash);

			virtual void OnRejectedTx(const PeerPtr &peer, const uint256 &txHash, uint8_t code, const std::string &reason);

			virtual void OnRelayedBlock(const PeerPtr &peer, const MerkleBlockPtr &block);

			virtual void OnRelayedPing(const PeerPtr &peer);

			virtual void OnNotfound(const PeerPtr &peer, const std::vector<uint256> &txHashes,
									const std::vector<uint256> &blockHashes);

			virtual void OnSetFeePerKb(const PeerPtr &peer, uint64_t feePerKb);

			virtual TransactionPtr OnRequestedTx(const PeerPtr &peer, const uint256 &txHash);

			virtual bool OnNetworkIsReachable(const PeerPtr &peer);

			virtual void OnThreadCleanup(const PeerPtr &peer);

		private:
			void InitBlocks(const std::vector<MerkleBlockPtr> &blocks);

			void RemovePeer(const PeerPtr &peer);

			Peer::ConnectStatus GetConnectStatusInternal() const;

		private:
			void FireSyncStarted();

			void FireSyncProgress(double progress, const PeerPtr &peer, const MerkleBlockPtr &block);

			void FireSyncStopped(int error);

			void FireTxStatusUpdate();

			void FireSaveBlocks(bool replace, const std::vector<MerkleBlockPtr> &blocks);

			void FireSavePeers(bool replace, const std::vector<PeerInfo> &peers);

			void FireSaveBlackPeer(const PeerInfo &peer);

			bool FireNetworkIsReachable();

			void FireTxPublished(const uint256 &hash, int code, const std::string &reason);

			void FireConnectStatusChanged(Peer::ConnectStatus status);

			void FireThreadCleanup();

			int VerifyDifficulty(const ChainParamsPtr &params, const MerkleBlockPtr &block,
								 const BlockSet &blockSet);

			int VerifyDifficultyInner(const MerkleBlockPtr &block, const MerkleBlockPtr &previous,
									  uint32_t transitionTime, uint32_t targetTimeSpan, uint32_t targetTimePerBlock);

			void LoadBloomFilter(const PeerPtr &peer);

			void UpdateBloomFilter();

			void FindPeers();

			void SortPeers();

			void SyncStopped();

			void AddTxToPublishList(const TransactionPtr &tx, const Peer::PeerPubTxCallback &callback);

			size_t PublishPendingTx(const PeerPtr &peer);

			size_t
			AddPeerToList(const PeerPtr &peer, const uint256 &txHash, std::vector<TransactionPeerList> &peerList);

			bool
			RemovePeerFromList(const PeerPtr &peer, const uint256 &txHash, std::vector<TransactionPeerList> &peerList);

			void PeerMisbehaving(const PeerPtr &peer);

			std::vector<uint128> AddressLookup(const std::string &hostname);

			bool VerifyBlock(const MerkleBlockPtr &block, const MerkleBlockPtr &prev, const PeerPtr &peer);

			std::vector<uint256> GetBlockLocators();

			void LoadMempools();

			void RequestUnrelayedTx(const PeerPtr &peer);

			bool PeerListHasPeer(const std::vector<TransactionPeerList> &peerList, const uint256 &txhash,
								 const PeerPtr &peer);

			size_t PeerListCount(const std::vector<TransactionPeerList> &list, const uint256 &txhash);

			void UpdateAddressOnlyDone(const PeerPtr &peer, int success);

			void LoadBloomFilterDone(const PeerPtr &peer, int success);

			void UpdateFilterRerequestDone(const PeerPtr &peer, int success);

			void UpdateFilterLoadDone(const PeerPtr &peer, int success);

			void UpdateFilterPingDone(const PeerPtr &peer, int success);

			void MempoolDone(const PeerPtr &peer, int success);

			void RequestUnrelayedTxGetDataDone(const PeerPtr &peer, int success);

			void PublishTxInvDone(const PeerPtr &peer, int success);

			void FindPeersThreadRoutine(const std::string &hostname, uint64_t services);

			void ReconnectLaster(time_t seconds);

			double GetSyncProgressInternal(uint32_t startHeight);

		private:
			int _isConnected, _connectFailureCount, _misbehavinCount, _dnsThreadCount, _maxConnectCount;
			bool _syncSucceeded, _enableReconnect;

			Peer::ConnectStatus _connectStatus;
			std::vector<PeerInfo> _peers;
			std::set<PeerInfo> _blackPeers;
			PeerInfo _fixedPeer;

			std::vector<PeerPtr> _connectedPeers;
			PeerPtr _downloadPeer;

			mutable std::string _downloadPeerName;
			time_t _keepAliveTimestamp, _earliestKeyTime;
			uint32_t _reconnectSeconds, _syncStartHeight, _filterUpdateHeight, _estimatedHeight;
			BloomFilterPtr _bloomFilter;
			double _fpRate, _averageTxPerBlock;
			BlockSet _blocks;
			BlockSet _orphans;
			BlockSet _checkpoints;
			MerkleBlockPtr _lastBlock, _lastOrphan;
			std::vector<TransactionPeerList> _txRelays, _txRequests;
			std::vector<PublishedTransaction> _publishedTx;
			std::vector<uint256> _publishedTxHashes;

			std::string _chainID;
			std::string _netType;
			WalletPtr _wallet;
			ChainParamsPtr _chainParams;

			boost::asio::io_service _reconnectService;
			boost::shared_ptr<boost::asio::deadline_timer> _reconnectTimer;

			boost::weak_ptr<Listener> _listener;
		};

		typedef boost::shared_ptr<PeerManager> PeerManagerPtr;

	}
}

#endif //__ELASTOS_SDK_PEERMANAGER_H__
