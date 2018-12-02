// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <time.h>
#include <Core/BRMerkleBlock.h>

#include "Core/BRArray.h"

#include "ELAPeerManager.h"

namespace Elastos {
	namespace ElaWallet {

		namespace {
			static void _dummyThreadCleanup(void *info) {

			}
		}

		ELAPeerManager *ELAPeerManagerNew(const BRChainParams *params, BRWallet *wallet, uint32_t earliestKeyTime,
										  uint32_t reconnectSeconds,
										  BRMerkleBlock *blocks[], size_t blocksCount, const BRPeer peers[],
										  size_t peersCount, BRPeerMessages *peerMessages, const PluginTypes &plugins) {
			ELAPeerManager *manager = (ELAPeerManager *)calloc(1, sizeof(*manager));
			BRMerkleBlock orphan, *block = NULL;

			assert(manager != NULL);
			assert(params != NULL);
			assert(params->standardPort != 0);
			assert(wallet != NULL);
			assert(blocks != NULL || blocksCount == 0);
			assert(peers != NULL || peersCount == 0);
			memset(manager, 0, sizeof(*manager));

			manager->Plugins = plugins;
			manager->Raw.peerMessages = peerMessages;
			manager->Raw.params = params;
			manager->Raw.wallet = wallet;
			manager->Raw.earliestKeyTime = earliestKeyTime;
			manager->Raw.reconnectSeconds = reconnectSeconds;
			manager->Raw.averageTxPerBlock = 1400;
			manager->Raw.maxConnectCount = PEER_MAX_CONNECTIONS;
			array_new(manager->Raw.peers, peersCount);
			if (peers) array_add_array(manager->Raw.peers, peers, peersCount);
			qsort(manager->Raw.peers, array_count(manager->Raw.peers), sizeof(*manager->Raw.peers), _peerTimestampCompare);
			array_new(manager->Raw.connectedPeers, PEER_MAX_CONNECTIONS);
			manager->Raw.blocks = BRSetNew(BRMerkleBlockHash, BRMerkleBlockEq, blocksCount);
			manager->Raw.orphans = BRSetNew(_BRPrevBlockHash, _BRPrevBlockEq, blocksCount); // orphans are indexed by prevBlock
			manager->Raw.checkpoints = BRSetNew(_BRBlockHeightHash, _BRBlockHeightEq, 100); // checkpoints are indexed by height
			manager->Raw.reconnectTaskCount = 0;

			time_t now = time(nullptr);
			for (size_t i = 0; i < manager->Raw.params->checkpointsCount; i++) {
				block = manager->Raw.peerMessages->MerkleBlockNew(manager);
				block->height = manager->Raw.params->checkpoints[i].height;
				block->blockHash = UInt256Reverse(&manager->Raw.params->checkpoints[i].hash);
				block->timestamp = manager->Raw.params->checkpoints[i].timestamp;
				block->target = manager->Raw.params->checkpoints[i].target;
				BRSetAdd(manager->Raw.checkpoints, block);
				BRSetAdd(manager->Raw.blocks, block);
				if (i == 0 || block->timestamp + 1*24*60*60 < manager->Raw.earliestKeyTime ||
					(manager->Raw.earliestKeyTime == 0 && block->timestamp + 1*24*60*60 < now))
					manager->Raw.lastBlock = block;
			}

			block = NULL;

			for (size_t i = 0; blocks && i < blocksCount; i++) {
				assert(blocks[i]->height != BLOCK_UNKNOWN_HEIGHT); // height must be saved/restored along with serialized block
				BRSetAdd(manager->Raw.orphans, blocks[i]);

				if ((blocks[i]->height % BLOCK_DIFFICULTY_INTERVAL) == 0 &&
					(! block || blocks[i]->height > block->height)) block = blocks[i]; // find last transition block
			}

			while (block) {
				BRSetAdd(manager->Raw.blocks, block);
				manager->Raw.lastBlock = block;
				orphan.prevBlock = block->prevBlock;
				BRSetRemove(manager->Raw.orphans, &orphan);
				orphan.prevBlock = block->blockHash;
				block = (BRMerkleBlock *)BRSetGet(manager->Raw.orphans, &orphan);
			}

			array_new(manager->Raw.txRelays, 10);
			array_new(manager->Raw.txRequests, 10);
			array_new(manager->Raw.publishedTx, 10);
			array_new(manager->Raw.publishedTxHashes, 10);
			pthread_mutex_init(&manager->Raw.lock, NULL);
			manager->Raw.threadCleanup = _dummyThreadCleanup;

			manager->Raw.wallet->lastBlockHeight = manager->Raw.lastBlock->height;

			return manager;
		}

		void ELAPeerManagerFree(ELAPeerManager *manager) {
			BRTransaction *tx;

			assert(manager != NULL);
			pthread_mutex_lock(&manager->Raw.lock);
			array_free(manager->Raw.peers);
			for (size_t i = array_count(manager->Raw.connectedPeers); i > 0; i--) BRPeerFree(manager->Raw.connectedPeers[i - 1]);
			array_free(manager->Raw.connectedPeers);
			BRSetApply(manager->Raw.blocks, manager, manager->Raw.peerMessages->ApplyFreeBlock);
			BRSetFree(manager->Raw.blocks);
			BRSetApply(manager->Raw.orphans, manager, manager->Raw.peerMessages->ApplyFreeBlock);
			BRSetFree(manager->Raw.orphans);
			BRSetFree(manager->Raw.checkpoints);
			for (size_t i = array_count(manager->Raw.txRelays); i > 0; i--) array_free(manager->Raw.txRelays[i - 1].peers);
			array_free(manager->Raw.txRelays);
			for (size_t i = array_count(manager->Raw.txRequests); i > 0; i--) array_free(manager->Raw.txRequests[i - 1].peers);
			array_free(manager->Raw.txRequests);

			for (size_t i = array_count(manager->Raw.publishedTx); i > 0; i--) {
				tx = manager->Raw.publishedTx[i - 1].tx;
				if (tx && tx != BRWalletTransactionForHash(manager->Raw.wallet, tx->txHash)) BRTransactionFree(tx);
			}

			array_free(manager->Raw.publishedTx);
			array_free(manager->Raw.publishedTxHashes);
			pthread_mutex_unlock(&manager->Raw.lock);
			pthread_mutex_destroy(&manager->Raw.lock);
			free(manager);
		}


	}
}