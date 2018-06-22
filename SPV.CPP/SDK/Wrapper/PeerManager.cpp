// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <netinet/in.h>
#include <arpa/inet.h>

#include "PeerManager.h"
#include "Utils.h"
#include "Message/PeerMessageManager.h"
#include "Log.h"
#include "BRArray.h"
#include "ELATransaction.h"
#include "ELAMerkleBlock.h"
#include "arith_uint256.h"

namespace Elastos {
	namespace ElaWallet {

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

					SharedWrapperList<MerkleBlock, BRMerkleBlock *> *coreBlocks = new SharedWrapperList<MerkleBlock, BRMerkleBlock *>();
					for (size_t i = 0; i < blockCount; ++i) {
						coreBlocks->push_back(
								MerkleBlockPtr(new MerkleBlock(ELAMerkleBlockCopy((ELAMerkleBlock *) blocks[i]))));
					}
					listener->lock()->saveBlocks(replace, *coreBlocks);
				}
			}

			static void savePeers(void *info, int replace, const BRPeer peers[], size_t count) {

				WeakListener *listener = (WeakListener *) info;
				if (!listener->expired()) {

					SharedWrapperList<Peer, BRPeer *> *corePeers = new SharedWrapperList<Peer, BRPeer *>();
					for (size_t i = 0; i < count; ++i) {
						corePeers->push_back(PeerPtr(new Peer(peers[i])));
					}
					listener->lock()->savePeers(replace, *corePeers);
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

			static void blockHeightIncreased(void *info, uint32_t height) {
				WeakListener *listener = (WeakListener *) info;
				if (!listener->expired()) {
					listener->lock()->blockHeightIncreased(height);
				}
			}
		}

		PeerManager::PeerManager(const ChainParams &params,
								 const WalletPtr &wallet,
								 uint32_t earliestKeyTime,
								 const SharedWrapperList<MerkleBlock, BRMerkleBlock *> &blocks,
								 const SharedWrapperList<Peer, BRPeer *> &peers,
								 const boost::shared_ptr<PeerManager::Listener> &listener) :
				_wallet(wallet),
				_chainParams(params) {

			assert(listener != nullptr);
			_listener = boost::weak_ptr<Listener>(listener);

			BRPeer peerArray[peers.size()];
			for (int i = 0; i < peers.size(); ++i) {
				peerArray[i] = *peers[i]->getRaw();
			}

			_manager = BRPeerManagerNew(
					_chainParams.getRaw(),
					wallet->getRaw(),
					earliestKeyTime,
					getRawMerkleBlocks(blocks).data(),
					blocks.size(),
					peerArray,
					peers.size(),
					PeerMessageManager::instance().createMessageManager()
			);

//			if (_manager->lastBlock == nullptr) {
//				createGenesisBlock();
//			}

			BRPeerManagerSetCallbacks(_manager, &_listener,
									  syncStarted,
									  syncStopped,
									  txStatusUpdate,
									  saveBlocks,
									  savePeers,
									  networkIsReachable,
									  threadCleanup,
									  blockHeightIncreased,
									  verifyDifficultyWrapper);
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
			ELATransaction *elaTransaction = ELATransactioCopy((ELATransaction *)transaction->getRaw());
			BRPeerManagerPublishTx(_manager, (BRTransaction *)elaTransaction, &_listener, txPublished);
		}

		uint64_t PeerManager::getRelayCount(const UInt256 &txHash) const {
			return BRPeerManagerRelayCount(_manager, txHash);
		}

		void PeerManager::createGenesisBlock() const {
			ELAMerkleBlock *block = ELAMerkleBlockNew();
			block->raw.height = 0;
			block->raw.blockHash = Utils::UInt256FromString(
					"8d7014f2f941caa1972c8033b2f0a860ec8d4938b12bae2c62512852a558f405");
			block->raw.timestamp = 1513936800;
			block->raw.target = 486801407;
			BRSetAdd(_manager->blocks, block);
			_manager->lastBlock = (BRMerkleBlock *) block;
		}

		std::vector<BRMerkleBlock *>
		PeerManager::getRawMerkleBlocks(const SharedWrapperList<MerkleBlock, BRMerkleBlock *> &blocks) {
			std::vector<BRMerkleBlock *> list;

			size_t len = blocks.size();
			for (size_t i = 0; i < len; ++i) {
				ELAMerkleBlock *temp = (ELAMerkleBlock *) blocks[i]->getRaw();
				list.push_back((BRMerkleBlock *) ELAMerkleBlockCopy(temp));
			}

			return list;
		}

		int PeerManager::verifyDifficultyWrapper(const BRChainParams *params, const BRMerkleBlock *block,
												 const BRSet *blockSet) {
			const ELAChainParams *wrapperParams = (const ELAChainParams *) params;
			return verifyDifficulty(block, blockSet, wrapperParams->TargetTimeSpan, wrapperParams->TargetTimePerBlock);
		}

		int PeerManager::verifyDifficulty(const BRMerkleBlock *block, const BRSet *blockSet, uint32_t targetTimeSpan,
										  uint32_t targetTimePerBlock) {
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
			return verifyDifficultyInner(block, previous, (b) ? b->timestamp : 0, targetTimeSpan, targetTimePerBlock);
		}

		int PeerManager::verifyDifficultyInner(const BRMerkleBlock *block, const BRMerkleBlock *previous,
											   uint32_t transitionTime, uint32_t targetTimeSpan,
											   uint32_t targetTimePerBlock) {
			int r = 1;

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
	}
}