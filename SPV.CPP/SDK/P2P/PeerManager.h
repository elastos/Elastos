// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef __ELASTOS_SDK_PEERMANAGER_H__
#define __ELASTOS_SDK_PEERMANAGER_H__

#include "Peer.h"
#include "TransactionPeerList.h"
#include "PublishedTransaction.h"

#include <SDK/Common/Lockable.h>
#include <SDK/WalletCore/BIPs/BloomFilter.h>
#include <SDK/Plugin/Interface/IMerkleBlock.h>
#include <SDK/Plugin/Block/MerkleBlock.h>
#include <SDK/Plugin/Registry.h>

#include <string>
#include <vector>
#include <boost/weak_ptr.hpp>
#include <boost/thread/mutex.hpp>

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

				// func syncStarted()
				virtual void syncStarted() = 0;

				virtual void syncProgress(uint32_t currentHeight, uint32_t estimateHeight, time_t lastBlockTime) = 0;

				// func syncStopped(_ error: BRPeerManagerError?)
				virtual void syncStopped(const std::string &error) = 0;

				// func txStatusUpdate()
				virtual void txStatusUpdate() = 0;

				// func saveBlocks(_ replace: Bool, _ blocks: [BRBlockRef?])
				virtual void saveBlocks(bool replace, const std::vector<MerkleBlockPtr> &blocks) = 0;

				// func savePeers(_ replace: Bool, _ peers: [BRPeer])
				virtual void savePeers(bool replace, const std::vector<PeerInfo> &peers) = 0;

				// func networkIsReachable() -> Bool}
				virtual bool networkIsReachable() = 0;

				// Called on publishTransaction
				virtual void txPublished(const std::string &hash, const nlohmann::json &result) = 0;

				virtual void syncIsInactive(uint32_t time) = 0;

				virtual void connectStatusChanged(const std::string &status) = 0;
			};

		public:
			PeerManager(const ChainParamsPtr &params,
						const WalletPtr &wallet,
						time_t earliestKeyTime,
						uint32_t reconnectSeconds,
						const std::vector<MerkleBlockPtr> &blocks,
						const std::vector<PeerInfo> &peers,
						const boost::shared_ptr<Listener> &listener,
						const PluginType &plugin);

			~PeerManager();

			void SetWallet(const WalletPtr &wallet);
			/**
			* Connect to bitcoin peer-to-peer network (also call this whenever networkIsReachable()
			* status changes)
			*/
			void Connect();

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

			void ResetReconnectStep();

			bool SyncSucceeded() const;

			void SetSyncSucceeded(bool succeeded);

			void SetReconnectEnableStatus(bool status);

			bool GetReconnectEnableStatus() const;

			void SetFixedPeer(uint128 address, uint16_t port);

			void SetFixedPeers(const std::vector<PeerInfo> &peers);

			bool UseFixedPeer(const std::string &node, int port);

			std::string GetCurrentPeerName() const;

			std::string GetDownloadPeerName() const;

			const PeerPtr GetDownloadPeer() const;

			size_t GetPeerCount() const;

			void PublishTransaction(const TransactionPtr &transaction);

			void PublishTransaction(const TransactionPtr &transaction, const Peer::PeerPubTxCallback &callback);

			uint64_t GetRelayCount(const uint256 &txHash) const;

			void AsyncConnect(const boost::system::error_code &error);

			const std::vector<PublishedTransaction> GetPublishedTransaction() const;

			const std::vector<uint256> GetPublishedTransactionHashes() const;

			int ReconnectTaskCount() const;

			void SetReconnectTaskCount(int count);

			const PluginType &GetPluginType() const;

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

			virtual void OnRelayedPingMsg(const PeerPtr &peer);

			virtual void OnNotfound(const PeerPtr &peer, const std::vector<uint256> &txHashes,
									const std::vector<uint256> &blockHashes);

			virtual void OnSetFeePerKb(const PeerPtr &peer, uint64_t feePerKb);

			virtual TransactionPtr OnRequestedTx(const PeerPtr &peer, const uint256 &txHash);

			virtual bool OnNetworkIsReachable(const PeerPtr &peer);

			virtual void OnThreadCleanup(const PeerPtr &peer);

		private:
			void FireSyncStarted();

			void FireSyncProgress(uint32_t currentHeight, uint32_t estimatedHeight, time_t lastBlockTime);

			void FireSyncStopped(int error);

			void FireTxStatusUpdate();

			void FireSaveBlocks(bool replace, const std::vector<MerkleBlockPtr> &blocks);

			void FireSavePeers(bool replace, const std::vector<PeerInfo> &peers);

			bool FireNetworkIsReachable();

			void FireTxPublished(const uint256 &hash, int code, const std::string &reason);

			void FireConnectStatusChanged(Peer::ConnectStatus status);

			void FireThreadCleanup();

			void FireBlockHeightIncreased(uint32_t height);

			void FireSyncIsInactive(uint32_t time);

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

			void PublishPendingTx(const PeerPtr &peer);

			size_t
			AddPeerToList(const PeerPtr &peer, const uint256 &txHash, std::vector<TransactionPeerList> &peerList);

			bool
			RemovePeerFromList(const PeerPtr &peer, const uint256 &txHash, std::vector<TransactionPeerList> &peerList);

			void PeerMisbehaving(const PeerPtr &peer);

			std::vector<uint128> AddressLookup(const std::string &hostname);

			bool VerifyBlock(const MerkleBlockPtr &block, const MerkleBlockPtr &prev, const PeerPtr &peer);

			std::vector<uint256> GetBlockLocators();

			void LoadMempools();

			void ResendUnconfirmedTx(const PeerPtr &peer);

			void RequestUnrelayedTx(const PeerPtr &peer);

			bool PeerListHasPeer(const std::vector<TransactionPeerList> &peerList, const uint256 &txhash,
								 const PeerPtr &peer);

			size_t PeerListCount(const std::vector<TransactionPeerList> &list, const uint256 &txhash);

			void LoadBloomFilterDone(const PeerPtr &peer, int success);

			void UpdateFilterRerequestDone(const PeerPtr &peer, int success);

			void UpdateFilterLoadDone(const PeerPtr &peer, int success);

			void UpdateFilterPingDone(const PeerPtr &peer, int success);

			void MempoolDone(const PeerPtr &peer, int success);

			void RequestUnrelayedTxGetDataDone(const PeerPtr &peer, int success);

			void PublishTxInvDone(const PeerPtr &peer, int success);

			void FindPeersThreadRoutine(const std::string &hostname, uint64_t services);

		private:
			int _isConnected, _connectFailureCount, _misbehavinCount, _dnsThreadCount, _maxConnectCount, _reconnectTaskCount;
			bool _syncSucceeded, _needGetAddr, _enableReconnectTask;

			std::vector<PeerInfo> _peers;
			std::vector<PeerInfo> _fiexedPeers;
			PeerInfo _fixedPeer;

			std::vector<PeerPtr> _connectedPeers;
			PeerPtr _downloadPeer;

			mutable std::string _downloadPeerName;
			time_t _keepAliveTimestamp, _earliestKeyTime;
			uint32_t _reconnectSeconds, _syncStartHeight, _filterUpdateHeight, _estimatedHeight;
			uint32_t _reconnectStep;
			BloomFilterPtr _bloomFilter;
			double _fpRate, _averageTxPerBlock;
			BlockSet _blocks;
			std::set<MerkleBlockPtr> _orphans;
			BlockSet _checkpoints;
			MerkleBlockPtr _lastBlock, _lastOrphan;
			std::vector<TransactionPeerList> _txRelays, _txRequests;
			std::vector<PublishedTransaction> _publishedTx;
			std::vector<uint256> _publishedTxHashes;

			PluginType _pluginType;
			WalletPtr _wallet;
			ChainParamsPtr _chainParams;
			boost::weak_ptr<Listener> _listener;
		};

		typedef boost::shared_ptr<PeerManager> PeerManagerPtr;

	}
}

#endif //__ELASTOS_SDK_PEERMANAGER_H__
