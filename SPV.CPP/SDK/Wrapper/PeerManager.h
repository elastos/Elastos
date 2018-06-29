// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef __ELASTOS_SDK_PEERMANAGER_H__
#define __ELASTOS_SDK_PEERMANAGER_H__

#include <string>
#include <vector>
#include <boost/weak_ptr.hpp>

#include "BRPeerManager.h"

#include "Peer.h"
#include "ChainParams.h"
#include "Wallet.h"
#include "SharedWrapperList.h"
#include "WrapperList.h"
#include "CMemBlock.h"
#include "ELACoreExt/ELAPeerManager.h"
#include "Plugin/PluginTypes.h"
#include "Plugin/Interface/IMerkleBlock.h"
#include "Plugin/Block/MerkleBlock.h"

namespace Elastos {
	namespace ElaWallet {

		class PeerManager :
				public Wrapper<BRPeerManager> {
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
#ifdef MERKLE_BLOCK_PLUGIN
				virtual void
				saveBlocks(bool replace, const SharedWrapperList<IMerkleBlock, BRMerkleBlock *> &blocks) = 0;
#else
				virtual void
				saveBlocks(bool replace, const SharedWrapperList<MerkleBlock, BRMerkleBlock *> &blocks) = 0;
#endif

				// func savePeers(_ replace: Bool, _ peers: [BRPeer])
				virtual void savePeers(bool replace, const SharedWrapperList<Peer, BRPeer *> &peers) = 0;

				// func networkIsReachable() -> Bool}
				virtual bool networkIsReachable() = 0;

				// Called on publishTransaction
				virtual void txPublished(const std::string &error) = 0;

				virtual void blockHeightIncreased(uint32_t blockHeight) = 0;

				const PluginTypes &getPluginTypes() const { return _pluginTypes;}

			protected:
				PluginTypes _pluginTypes;
			};

		public:
			PeerManager(const ChainParams &params,
						const WalletPtr &wallet,
						uint32_t earliestKeyTime,
#ifdef MERKLE_BLOCK_PLUGIN
						const SharedWrapperList<IMerkleBlock, BRMerkleBlock *> &blocks,
#else
						const SharedWrapperList<MerkleBlock, BRMerkleBlock *> &blocks,
#endif
						const SharedWrapperList<Peer, BRPeer *> &peers,
						const boost::shared_ptr<Listener> &listener,
						const PluginTypes &plugins);

			~PeerManager();

			virtual std::string toString() const;

			virtual BRPeerManager *getRaw() const;

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

			uint32_t getEstimatedBlockHeight() const;

			uint32_t getLastBlockHeight() const;

			uint32_t getLastBlockTimestamp() const;

			double getSyncProgress(uint32_t startHeight);

			Peer::ConnectStatus getConnectStatus() const;

			void setFixedPeers(const SharedWrapperList<Peer, BRPeer *> &peers);

			bool useFixedPeer(const std::string &node, int port);

			std::string getCurrentPeerName() const;

			std::string getDownloadPeerName() const;

			size_t getPeerCount() const;

			void publishTransaction(const TransactionPtr &transaction);

			uint64_t getRelayCount(const UInt256 &txHash) const;

		private:
			void createGenesisBlock() const;

			static int verifyDifficultyWrapper(const BRChainParams *params, const BRMerkleBlock *block,
											   const BRSet *blockSet);

			static int verifyDifficulty(const BRMerkleBlock *block, const BRSet *blockSet, uint32_t targetTimeSpan,
										uint32_t targetTimePerBlock);

			static int
			verifyDifficultyInner(const BRMerkleBlock *block, const BRMerkleBlock *previous, uint32_t transitionTime,
								  uint32_t targetTimeSpan, uint32_t targetTimePerBlock);

		private:
			ELAPeerManager *_manager;

			PluginTypes _pluginTypes;
			WalletPtr _wallet;
			ChainParams _chainParams;
			boost::weak_ptr<Listener> _listener;
		};

		typedef boost::shared_ptr<PeerManager> PeerManagerPtr;

	}
}

#endif //__ELASTOS_SDK_PEERMANAGER_H__
