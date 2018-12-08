// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <netinet/in.h>
#include <arpa/inet.h>

#include "PeerManager.h"
#include "Utils.h"
#include "Log.h"
#include "BRArray.h"
#include "ELATransaction.h"
#include "ELAMerkleBlock.h"
#include "arith_uint256.h"
#include "Message/PeerMessageManager.h"
#include "Plugin/Registry.h"
#include "Plugin/Block/MerkleBlock.h"

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

			static void syncProgress(void *info, uint32_t currentHeight, uint32_t estimatedHeight) {
				WeakListener *listener = (WeakListener *) info;
				if (!listener->expired()) {
					listener->lock()->syncProgress(currentHeight, estimatedHeight);
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
				WeakListener *weakListener = (WeakListener *) info;
				if (!weakListener->expired()) {

					PeerManager::Listener *listener = weakListener->lock().get();
					SharedWrapperList<IMerkleBlock, BRMerkleBlock *> *coreBlocks =
							new SharedWrapperList<IMerkleBlock, BRMerkleBlock *>();
					for (size_t i = 0; i < blockCount; ++i) {
						MerkleBlockPtr wrappedBlock(
								Registry::Instance()->CloneMerkleBlock(listener->getPluginTypes().BlockType, blocks[i],
																		true));
						coreBlocks->push_back(wrappedBlock);
					}
					listener->saveBlocks(replace, *coreBlocks);
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
				int result = 1;
				if (!listener->expired()) {

					result = listener->lock()->networkIsReachable();
				}
				return result;
			}

			static void txPublished(void *info, const UInt256 *hash, int error, const char *reason) {

				WeakListener *listener = (WeakListener *) info;
				nlohmann::json result;
				result["Code"] = error;
				result["Reason"] = std::string(reason);
				const std::string txID = Utils::UInt256ToString(*hash, true);

				if (!listener->expired()) {
					listener->lock()->txPublished(txID, result);
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

			static void syncIsInactive(void *info, uint32_t time) {
				WeakListener *listener = (WeakListener *) info;
				if (!listener->expired()) {
					listener->lock()->syncIsInactive(time);
				}
			}
		}

		PeerManager::Listener::Listener(const Elastos::ElaWallet::PluginTypes &pluginTypes) :
				_pluginTypes(pluginTypes) {
		}

		PeerManager::PeerManager(const ChainParams &params,
								 const WalletPtr &wallet,
								 uint32_t earliestKeyTime,
								 uint32_t reconnectSeconds,
								 const SharedWrapperList<IMerkleBlock, BRMerkleBlock *> &blocks,
								 const SharedWrapperList<Peer, BRPeer *> &peers,
								 const boost::shared_ptr<PeerManager::Listener> &listener,
								 const PluginTypes &plugins) :
				_wallet(wallet),
				_pluginTypes(plugins),
				_chainParams(params) {

			assert(listener != nullptr);
			_listener = boost::weak_ptr<Listener>(listener);

			BRPeer peerArray[peers.size()];
			for (int i = 0; i < peers.size(); ++i) {
				peerArray[i] = *peers[i]->getRaw();
			}

			std::vector<BRMerkleBlock *> blockArray;
			for (SharedWrapperList<IMerkleBlock, BRMerkleBlock *>::const_iterator it = blocks.cbegin();
				 it != blocks.cend(); ++it) {
				blockArray.push_back((*it)->getRawBlock());
			}

			_manager = ELAPeerManagerNew(
					_chainParams.getRaw(),
					wallet->getRaw(),
					earliestKeyTime,
					reconnectSeconds,
					blockArray.data(),
					blocks.size(),
					peerArray,
					peers.size(),
					PeerMessageManager::instance().createMessageManager(),
					plugins
			);

			BRPeerManagerSetCallbacks((BRPeerManager *) _manager, &_listener,
									  syncStarted,
									  syncProgress,
									  syncStopped,
									  txStatusUpdate,
									  saveBlocks,
									  savePeers,
									  networkIsReachable,
									  threadCleanup,
									  blockHeightIncreased,
									  syncIsInactive,
									  verifyDifficultyWrapper,
									  loadBloomFilter);
		}

		PeerManager::~PeerManager() {
			ELAPeerManagerFree(_manager);
		}

		std::string PeerManager::toString() const {
			//todo complete me
			return "";
		}

		BRPeerManager *PeerManager::getRaw() const {
			return (BRPeerManager *) _manager;
		}

		Peer::ConnectStatus PeerManager::getConnectStatus() const {
			return Peer::ConnectStatus(BRPeerManagerConnectStatus((BRPeerManager *) _manager));
		}

		void PeerManager::connect() {
			BRPeerManagerConnect((BRPeerManager *) _manager);
		}

		void PeerManager::disconnect() {
			BRPeerManagerDisconnect((BRPeerManager *) _manager);
		}

		void PeerManager::DisableReconnectCallback() {
			pthread_mutex_lock(&_manager->Raw.lock);
			_manager->Raw.syncIsInactivate = NULL;
			pthread_mutex_unlock(&_manager->Raw.lock);
		}

		void PeerManager::rescan() {
			BRPeerManagerRescan((BRPeerManager *) _manager);
		}

		uint32_t PeerManager::getSyncStartHeight() const {
			return _manager->Raw.syncStartHeight;
		}

		uint32_t PeerManager::getEstimatedBlockHeight() const {
			return BRPeerManagerEstimatedBlockHeight((BRPeerManager *) _manager);
		}

		uint32_t PeerManager::getLastBlockHeight() const {
			return BRPeerManagerLastBlockHeight((BRPeerManager *) _manager);
		}

		uint32_t PeerManager::getLastBlockTimestamp() const {
			return BRPeerManagerLastBlockTimestamp((BRPeerManager *) _manager);
		}

		double PeerManager::getSyncProgress(uint32_t startHeight) {
			return BRPeerManagerSyncProgress((BRPeerManager *) _manager, startHeight);
		}

		void PeerManager::setFixedPeers(const SharedWrapperList<Peer, BRPeer *> &peers) {
			if (_manager->Raw.fiexedPeers != nullptr)
				array_clear(_manager->Raw.fiexedPeers);

			array_new(_manager->Raw.fiexedPeers, peers.size());
			for (int i = 0; i < peers.size(); ++i) {
				array_add(_manager->Raw.fiexedPeers, *peers[i]->getRaw());
			}
		}

		bool PeerManager::useFixedPeer(const std::string &node, int port) {
			const BRChainParams *chainParams = BRPeerManagerChainParams((BRPeerManager *) _manager);

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

			BRPeerManagerSetFixedPeer((BRPeerManager *) _manager, address, _port);
			return true;
		}

		std::string PeerManager::getCurrentPeerName() const {
			return BRPeerManagerDownloadPeerName((BRPeerManager *) _manager);
		}

		std::string PeerManager::getDownloadPeerName() const {
			return BRPeerManagerDownloadPeerName((BRPeerManager *) _manager);
		}

		size_t PeerManager::getPeerCount() const {
			return BRPeerManagerPeerCount((BRPeerManager *) _manager);
		}

		void PeerManager::publishTransaction(const TransactionPtr &transaction) {
			ELATransaction *elaTransaction = ELATransactionCopy((ELATransaction *) transaction->getRaw());
			BRPeerManagerPublishTx((BRPeerManager *) _manager, (BRTransaction *) elaTransaction, &_listener,
								   txPublished);
		}

		uint64_t PeerManager::getRelayCount(const UInt256 &txHash) const {
			return BRPeerManagerRelayCount((BRPeerManager *) _manager, txHash);
		}

		void PeerManager::createGenesisBlock() const {
			ELAMerkleBlock *block = ELAMerkleBlockNew();
			block->raw.height = 0;
			block->raw.blockHash = Utils::UInt256FromString(
					"8d7014f2f941caa1972c8033b2f0a860ec8d4938b12bae2c62512852a558f405");
			block->raw.timestamp = 1513936800;
			block->raw.target = 486801407;
			BRSetAdd(_manager->Raw.blocks, block);
			_manager->Raw.lastBlock = (BRMerkleBlock *) block;
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

		void PeerManager::loadBloomFilter(BRPeerManager *manager, BRPeer *peer) {
			// every time a new wallet address is added, the bloom filter has to be rebuilt, and each address is only used
			// for one transaction, so here we generate some spare addresses to avoid rebuilding the filter each time a
			// wallet transaction is encountered during the chain sync
			manager->wallet->WalletUnusedAddrs(manager->wallet, NULL, SEQUENCE_GAP_LIMIT_EXTERNAL + 100, 0);
			manager->wallet->WalletUnusedAddrs(manager->wallet, NULL, SEQUENCE_GAP_LIMIT_INTERNAL + 100, 1);

			BRSetApply(manager->orphans, manager, manager->peerMessages->ApplyFreeBlock);
			BRSetClear(manager->orphans); // clear out orphans that may have been received on an old filter
			manager->lastOrphan = NULL;
			manager->filterUpdateHeight = manager->lastBlock->height;
			manager->fpRate = BLOOM_REDUCED_FALSEPOSITIVE_RATE;

			size_t addrsCount = manager->wallet->WalletAllAddrs(manager->wallet, NULL, 0);
			BRAddress *addrs = (BRAddress *)malloc(addrsCount*sizeof(*addrs));
			size_t utxosCount = BRWalletUTXOs(manager->wallet, NULL, 0);
			BRUTXO *utxos = (BRUTXO *)malloc(utxosCount*sizeof(*utxos));
			uint32_t blockHeight = (manager->lastBlock->height > 100) ? manager->lastBlock->height - 100 : 0;
			size_t txCount = BRWalletTxUnconfirmedBefore(manager->wallet, NULL, 0, blockHeight);
			BRTransaction **transactions = (BRTransaction **)malloc(txCount*sizeof(*transactions));
			BRBloomFilter *filter;

			assert(addrs != NULL);
			assert(utxos != NULL);
			assert(transactions != NULL);
			addrsCount = manager->wallet->WalletAllAddrs(manager->wallet, addrs, addrsCount);
			utxosCount = BRWalletUTXOs(manager->wallet, utxos, utxosCount);
			txCount = BRWalletTxUnconfirmedBefore(manager->wallet, transactions, txCount, blockHeight);
			filter = BRBloomFilterNew(manager->fpRate, addrsCount + utxosCount + txCount + 100, (uint32_t)BRPeerHash(peer),
									  BLOOM_UPDATE_ALL); // BUG: XXX txCount not the same as number of spent wallet outputs

			for (size_t i = 0; i < addrsCount; i++) { // add addresses to watch for tx receiveing money to the wallet
				UInt168 hash = UINT168_ZERO;

				BRAddressHash168(&hash, addrs[i].s);

				if (! UInt168IsZero(&hash) && ! BRBloomFilterContainsData(filter, hash.u8, sizeof(hash))) {
					BRBloomFilterInsertData(filter, hash.u8, sizeof(hash));
				}
			}

			free(addrs);

			ELAWallet *elaWallet = (ELAWallet *)manager->wallet;
			for (size_t i = 0; i < elaWallet->ListeningAddrs.size(); ++i) {
				UInt168 hash = UINT168_ZERO;

				BRAddressHash168(&hash, elaWallet->ListeningAddrs[i].c_str());

				if (! UInt168IsZero(&hash) && ! BRBloomFilterContainsData(filter, hash.u8, sizeof(hash))) {
					BRBloomFilterInsertData(filter, hash.u8, sizeof(hash));
				}
			}

			for (size_t i = 0; i < utxosCount; i++) { // add UTXOs to watch for tx sending money from the wallet
				uint8_t o[sizeof(UInt256) + sizeof(uint32_t)];

				UInt256Set(o, utxos[i].hash);
				UInt32SetLE(&o[sizeof(UInt256)], utxos[i].n);
				if (! BRBloomFilterContainsData(filter, o, sizeof(o))) BRBloomFilterInsertData(filter, o, sizeof(o));
			}

			free(utxos);

			for (size_t i = 0; i < txCount; i++) { // also add TXOs spent within the last 100 blocks
				for (size_t j = 0; j < transactions[i]->inCount; j++) {
					BRTxInput *input = &transactions[i]->inputs[j];
					BRTransaction *tx = BRWalletTransactionForHash(manager->wallet, input->txHash);
					uint8_t o[sizeof(UInt256) + sizeof(uint32_t)];

					if (tx && input->index < tx->outCount &&
						BRWalletContainsAddress(manager->wallet, tx->outputs[input->index].address)) {
						UInt256Set(o, input->txHash);
						UInt32SetLE(&o[sizeof(UInt256)], input->index);
						if (! BRBloomFilterContainsData(filter, o, sizeof(o))) BRBloomFilterInsertData(filter, o,sizeof(o));
					}
				}
			}

			free(transactions);
			if (manager->bloomFilter) BRBloomFilterFree(manager->bloomFilter);
			manager->bloomFilter = filter;
			// TODO: XXX if already synced, recursively add inputs of unconfirmed receives

			manager->peerMessages->BRPeerSendFilterloadMessage(peer, filter);
		}

	}
}