// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef __ELASTOS_SDK_PEERMANAGER_H__
#define __ELASTOS_SDK_PEERMANAGER_H__

#include <string>
#include <vector>
#include <boost/weak_ptr.hpp>
#include <boost/thread/mutex.hpp>

#include "Peer.h"
#include "ChainParams.h"
#include "Wallet.h"
#include "SharedWrapperList.h"
#include "WrapperList.h"
#include "CMemBlock.h"
#include "PublishedTransaction.h"
#include "TransactionPeerList.h"
#include "Plugin/PluginTypes.h"
#include "Plugin/Interface/IMerkleBlock.h"
#include "Plugin/Block/MerkleBlock.h"

#define PEER_MAX_CONNECTIONS 3

namespace Elastos {
	namespace ElaWallet {

		class PeerManager {
		public:

			class Listener {
			public:
				Listener(const PluginTypes &pluginTypes);

				virtual ~Listener() {}

				// func syncStarted()
				virtual void syncStarted() = 0;

				// func syncStopped(_ error: BRPeerManagerError?)
				virtual void syncStopped(const std::string &error) = 0;

				// func txStatusUpdate()
				virtual void txStatusUpdate() = 0;

				// func saveBlocks(_ replace: Bool, _ blocks: [BRBlockRef?])
				virtual void saveBlocks(bool replace, const std::vector<MerkleBlockPtr> &blocks) = 0;

				// func savePeers(_ replace: Bool, _ peers: [BRPeer])
				virtual void savePeers(bool replace, const std::vector<PeerPtr> &peers) = 0;

				// func networkIsReachable() -> Bool}
				virtual bool networkIsReachable() = 0;

				// Called on publishTransaction
				virtual void txPublished(const std::string &error) = 0;

				virtual void blockHeightIncreased(uint32_t blockHeight) = 0;

				virtual void syncIsInactive(uint32_t time) = 0;

				const PluginTypes &getPluginTypes() const { return _pluginTypes; }

			protected:
				PluginTypes _pluginTypes;
			};

		public:
			PeerManager(const ChainParams &params,
						const WalletPtr &wallet,
						uint32_t earliestKeyTime,
						uint32_t reconnectSeconds,
						const std::vector<MerkleBlockPtr> &blocks,
						const std::vector<PeerPtr> &peers,
						const boost::shared_ptr<Listener> &listener,
						const PluginTypes &plugins);

			~PeerManager();

			/**
			* Connect to bitcoin peer-to-peer network (also call this whenever networkIsReachable()
			* status changes)
			*/
			void connect();

			/**
			* Disconnect from bitcoin peer-to-peer network (may cause syncFailed(), saveBlocks() or
			* savePeers() callbacks to fire)
			*/
			void disconnect();

			void rescan();

			uint32_t getSyncStartHeight() const;

			uint32_t getEstimatedBlockHeight() const;

			uint32_t getLastBlockHeight() const;

			uint32_t getLastBlockTimestamp() const;

			double getSyncProgress(uint32_t startHeight);

			Peer::ConnectStatus getConnectStatus() const;

			void setFixedPeer(UInt128 address, uint16_t port);

			void setFixedPeers(const std::vector<PeerPtr> &peers);

			bool useFixedPeer(const std::string &node, int port);

			std::string getCurrentPeerName() const;

			std::string getDownloadPeerName() const;

			size_t getPeerCount() const;

			void publishTransaction(const TransactionPtr &transaction);

			uint64_t getRelayCount(const UInt256 &txHash) const;

			void asyncConnect(const boost::system::error_code &error);

		private:
			void syncStarted();

			void syncStopped(int error);

			void txStatusUpdate();

			void saveBlocks(bool replace, const std::vector<MerkleBlockPtr> &blocks);

			void savePeers(bool replace, const std::vector<PeerPtr> &peers);

			int networkIsReachable();

			void txPublished(int error);

			void threadCleanup();

			void blockHeightIncreased(uint32_t height);

			void syncIsInactive();

			int verifyDifficultyWrapper(const BRChainParams *params, const BRMerkleBlock *block, const BRSet *blockSet);

			int verifyDifficulty(const BRMerkleBlock *block, const BRSet *blockSet, uint32_t targetTimeSpan,
								 uint32_t targetTimePerBlock, const std::string &netType);

			int
			verifyDifficultyInner(const BRMerkleBlock *block, const BRMerkleBlock *previous, uint32_t transitionTime,
								  uint32_t targetTimeSpan, uint32_t targetTimePerBlock, const std::string &netType);

			void loadBloomFilter(BRPeer *peer);

			void findPeers();

			void sortPeers();

			void syncStopped();

			void addTxToPublishList(const TransactionPtr &tx, void (*callback)(void *, int));

		private:
			int isConnected, connectFailureCount, misbehavinCount, dnsThreadCount, maxConnectCount, reconnectTaskCount;
			std::vector<PeerPtr> _peers;
			std::vector<PeerPtr> _connectedPeers;
			std::vector<PeerPtr> _fiexedPeers;
			PeerPtr downloadPeer, fixedPeer;
			std::string downloadPeerName;
			uint32_t _earliestKeyTime, _reconnectSeconds, syncStartHeight, filterUpdateHeight, estimatedHeight;
			BRBloomFilter *bloomFilter;
			double fpRate, averageTxPerBlock;
			BlockSet _blocks;
			std::set<MerkleBlockPtr> _orphans;
			BlockSet _checkpoints;
			MerkleBlockPtr lastBlock, lastOrphan;
			std::vector<TransactionPeerList> txRelays, txRequests;
			std::vector<PublishedTransaction> publishedTx;
			std::vector<UInt256> publishedTxHashes;

			PluginTypes _pluginTypes;
			WalletPtr _wallet;
			ChainParams _chainParams;
			boost::weak_ptr<Listener> _listener;

			mutable boost::mutex lock;
		};

		typedef boost::shared_ptr<PeerManager> PeerManagerPtr;

	}
}

#endif //__ELASTOS_SDK_PEERMANAGER_H__
