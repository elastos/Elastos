// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <netinet/in.h>
#include <arpa/inet.h>

#include "PeerManager.h"
#include "Utils.h"
#include "Message/PeerMessageManager.h"

namespace Elastos {
	namespace SDK {

		namespace {

			typedef boost::weak_ptr<PeerManager::Listener> WeakListener;

			static void syncStarted(void *info) {

				WeakListener *listener = (WeakListener *) info;
				if (!listener->expired()) {
					listener->lock()->syncStarted();
				}
			}

			static void syncStopped(void *info, int error) {

				WeakListener *listener = (WeakListener *) info;
				if (!listener->expired()) {
					listener->lock()->syncStopped(error == 0 ? "" : strerror(error));
				}
			}

			static void txStatusUpdate(void *info) {

				WeakListener *listener = (WeakListener *) info;
				if (!listener->expired()) {
					listener->lock()->txStatusUpdate();
				}
			}

			static void saveBlocks(void *info, int replace, BRMerkleBlock *blocks[], size_t blockCount) {

				WeakListener *listener = (WeakListener *) info;
				if (!listener->expired()) {

					SharedWrapperList<MerkleBlock, BRMerkleBlock *> coreBlocks;
					for (size_t i = 0; i < blockCount; ++i) {
						coreBlocks.push_back(MerkleBlockPtr(new MerkleBlock(BRMerkleBlockCopy(blocks[i]))));
					}
					listener->lock()->saveBlocks(replace, coreBlocks);
				}
			}

			static void savePeers(void *info, int replace, const BRPeer peers[], size_t count) {

				WeakListener *listener = (WeakListener *) info;
				if (!listener->expired()) {

					WrapperList<Peer, BRPeer> corePeers;
					for (size_t i = 0; i < count; ++i) {
						corePeers.push_back(Peer(peers[i]));
					}
					listener->lock()->savePeers(replace, corePeers);
				}
			}

			static int networkIsReachable(void *info) {

				WeakListener *listener = (WeakListener *) info;
				int result = 0;
				if (!listener->expired()) {

					result = listener->lock()->networkIsReachable();
				}
				return result;
			}

			static void txPublished(void *info, int error) {

				WeakListener *listener = (WeakListener *) info;
				if (!listener->expired()) {

					listener->lock()->txPublished(error == 0 ? "" : strerror(error));
				}
			}

			static void threadCleanup(void *info) {

				WeakListener *listener = (WeakListener *) info;
				if (!listener->expired()) {

					//todo complete releaseEnv
					//releaseEnv();
				}
			}
		}

		PeerManager::PeerManager(const ChainParams &params,
								 const WalletPtr &wallet,
								 uint32_t earliestKeyTime,
								 const SharedWrapperList<MerkleBlock, BRMerkleBlock *> &blocks,
								 const WrapperList<Peer, BRPeer> &peers,
								 const boost::shared_ptr<PeerManager::Listener> &listener) :
				_wallet(wallet) {

			assert(listener != nullptr);
			_listener = boost::weak_ptr<Listener>(listener);

			_manager = BRPeerManagerNew(
					params.getRaw(),
					wallet->getRaw(),
					earliestKeyTime,
					blocks.getRawPointerArray().data(),
					blocks.size(),
					peers.getRawArray().data(),
					peers.size(),
					nullptr
			);
			PeerMessageManager::instance().initMessages(_manager);

			if(_manager->lastBlock == nullptr) {
				createDummyBlock();
			}

			BRPeerManagerSetCallbacks(_manager, &_listener,
									  syncStarted,
									  syncStopped,
									  txStatusUpdate,
									  saveBlocks,
									  savePeers,
									  networkIsReachable,
									  threadCleanup);
		}

		PeerManager::~PeerManager() {
			BRPeerManagerFree(_manager);
		}

		std::string PeerManager::toString() const {
			//todo complete me
			return "";
		}

		BRPeerManager *PeerManager::getRaw() const {
			return _manager;
		}

		Peer::ConnectStatus PeerManager::getConnectStatus() const {
			//todo complete me
			return Peer::Unknown;
		}

		void PeerManager::connect() {
			BRPeerManagerConnect(_manager);
		}

		void PeerManager::disconnect() {
			BRPeerManagerDisconnect(_manager);
		}

		void PeerManager::rescan() {
			BRPeerManagerRescan(_manager);
		}

		uint32_t PeerManager::getEstimatedBlockHeight() const {
			return BRPeerManagerEstimatedBlockHeight(_manager);
		}

		uint32_t PeerManager::getLastBlockHeight() const {
			return BRPeerManagerLastBlockHeight(_manager);
		}

		uint32_t PeerManager::getLastBlockTimestamp() const {
			return BRPeerManagerLastBlockTimestamp(_manager);
		}

		double PeerManager::getSyncProgress(uint32_t startHeight) {
			return BRPeerManagerSyncProgress(_manager, startHeight);;
		}

		bool PeerManager::useFixedPeer(const std::string &node, int port) {
			const BRChainParams *chainParams = BRPeerManagerChainParams(_manager);

			UInt128 address = UINT128_ZERO;
			uint16_t _port = (uint16_t) port;

			if (!node.empty()) {
				struct in_addr addr;
				if (inet_pton(AF_INET, node.c_str(), &addr) != 1) return false;
				address.u16[5] = 0xffff;
				address.u32[3] = addr.s_addr;
				if (port == 0) _port = chainParams->standardPort;
			} else {
				_port = 0;
			}

			BRPeerManagerSetFixedPeer(_manager, address, _port);
			return true;
		}

		std::string PeerManager::getCurrentPeerName() const {
			return BRPeerManagerDownloadPeerName(_manager);
		}

		std::string PeerManager::getDownloadPeerName() const {
			return BRPeerManagerDownloadPeerName(_manager);
		}

		size_t PeerManager::getPeerCount() const {
			return BRPeerManagerPeerCount(_manager);
		}

		void PeerManager::publishTransaction(const TransactionPtr &transaction) {
			BRPeerManagerPublishTx(_manager,
								   transaction->getRaw(), &_listener, txPublished);
		}

		uint64_t PeerManager::getRelayCount(const UInt256 &txHash) const {
			return BRPeerManagerRelayCount(_manager, txHash);
		}

		void PeerManager::createDummyBlock() const {
			BRMerkleBlock *block = BRMerkleBlockNew();
			block->height = 0;
			block->blockHash = Utils::UInt256FromString("8d7014f2f941caa1972c8033b2f0a860ec8d4938b12bae2c62512852a558f405");
			block->timestamp = 1513936800;
			block->target = 486801407;
			BRSetAdd(_manager->blocks, block);
			_manager->lastBlock = block;
		}
	}
}