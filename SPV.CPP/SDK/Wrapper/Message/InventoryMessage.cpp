// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.
#include <float.h>
#include <BRPeerMessages.h>
#include <BRPeerManager.h>

#include "BRPeer.h"
#include "BRPeerMessages.h"
#include "BRPeerManager.h"
#include "BRArray.h"

#include "InventoryMessage.h"
#include "Log.h"

#define MAX_BLOCKS_COUNT 100  //note max blocks count is 500 in btc while 100 in ela

namespace Elastos {
	namespace SDK {

		InventoryMessage::InventoryMessage() {

		}

		int InventoryMessage::Accept(BRPeer *peer, const uint8_t *msg, size_t msgLen) {
			Log::getLogger()->warn("InventoryMessage.Accept");

			BRPeerContext *ctx = (BRPeerContext *) peer;
			size_t off = 0;

			inv_type type = inv_type(msg[off]);
			off += sizeof(uint8_t);

			size_t count = UInt32GetLE(&msg[off]);
			off += sizeof(uint32_t);
			
			int r = 1;

			if (count > MAX_GETDATA_HASHES) {
				Log::getLogger()->warn("dropping inv message, {} is too many items, max is {}", count, MAX_GETDATA_HASHES);
			} else {
				const uint8_t *transactions[count], *blocks[count];
				size_t i, j, txCount = 0, blockCount = 0;

				Log::getLogger()->warn("got inv with {} item(s)", count);

				if (type == inv_block)
					blockCount = count;
				else if(type == inv_tx)
					txCount = count;

				for (i = 0; i < blockCount; i++) {
					blocks[i] = &msg[off];
					off += sizeof(UInt256);
				}

				for (i = 0; i < txCount; i++) {
					transactions[i] = &msg[off];
					off += sizeof(UInt256);
				}

				if (txCount > 0 && !ctx->sentFilter && !ctx->sentMempool && !ctx->sentGetblocks) {
					Log::getLogger()->warn("got inv message before loading a filter");
					r = 0;
				} else if (txCount > 10000) { // sanity check
					Log::getLogger()->warn("too many transactions, disconnecting");
					r = 0;
				} else if (ctx->currentBlockHeight > 0 && blockCount > 2 && blockCount < MAX_BLOCKS_COUNT &&
						   ctx->currentBlockHeight + array_count(ctx->knownBlockHashes) + blockCount < ctx->lastblock) {
					Log::getLogger()->warn("non-standard inv, {} is fewer block hash(es) than expected", blockCount);
					r = 0;
				} else {
					if (!ctx->sentFilter && !ctx->sentGetblocks) blockCount = 0;
					UInt256 blockHash;
					UInt256Get(&blockHash, blocks[0]);
					if (blockCount == 1 && UInt256Eq(&(ctx->lastBlockHash), &blockHash)) blockCount = 0;
					if (blockCount == 1)UInt256Get(&ctx->lastBlockHash, blocks[0]);

					UInt256 hash, blockHashes[blockCount], txHashes[txCount];

					for (i = 0; i < blockCount; i++) {
						UInt256Get(&blockHashes[i], blocks[i]);
						// remember blockHashes in case we need to re-request them with an updated bloom filter
						array_add(ctx->knownBlockHashes, blockHashes[i]);
					}

					while (array_count(ctx->knownBlockHashes) > MAX_GETDATA_HASHES) {
						array_rm_range(ctx->knownBlockHashes, 0, array_count(ctx->knownBlockHashes) / 3);
					}

					if (ctx->needsFilterUpdate) blockCount = 0;

					for (i = 0, j = 0; i < txCount; i++) {
						UInt256Get(&hash, transactions[i]);

						if (BRSetContains(ctx->knownTxHashSet, &hash)) {
							if (ctx->hasTx) ctx->hasTx(ctx->info, hash);
						} else txHashes[j++] = hash;
					}

					BRPeerAddKnownTxHashes(peer, txHashes, j);
					if (j > 0 || blockCount > 0)
						ctx->manager->peerMessages->BRPeerSendGetdataMessage(peer, txHashes, j, blockHashes, blockCount);

					// to improve chain download performance, if we received 500 block hashes, request the next 500 block hashes
					if (blockCount >= MAX_BLOCKS_COUNT) {
						UInt256 locators[] = {blockHashes[blockCount - 1], blockHashes[0]};

						ctx->manager->peerMessages->BRPeerSendGetblocksMessage(peer, locators, 2, UINT256_ZERO);
					}

					if (txCount > 0 && ctx->mempoolCallback) {
						Log::getLogger()->warn("got initial mempool response");
						ctx->manager->peerMessages->BRPeerSendPingMessage(peer, ctx->mempoolInfo, ctx->mempoolCallback);
						ctx->mempoolCallback = nullptr;
						ctx->mempoolTime = DBL_MAX;
					}
				}
			}

			return r;
		}

		void InventoryMessage::Send(BRPeer *peer) {
		}

		void InventoryMessage::Send(BRPeer *peer, const UInt256 txHashes[], size_t txCount) {
			Log::getLogger()->warn("InventoryMessage.Send");

			BRPeerContext *ctx = (BRPeerContext *) peer;
			size_t knownCount = array_count(ctx->knownTxHashes);

			BRPeerAddKnownTxHashes(peer, txHashes, txCount);
			txCount = array_count(ctx->knownTxHashes) - knownCount;

			if (txCount > 0) {
				for (size_t i = 0; i < txCount; i++) {
					sendTransaction(peer, ctx->knownTxHashes[knownCount + i]);
				}
			}
		}

		void InventoryMessage::sendTransaction(BRPeer *peer, const UInt256 &txHash) {
			BRPeerContext *ctx = (BRPeerContext *) peer;
			BRPeerManager *manager = ctx->manager;

			BRPublishedTx pubTx = {nullptr, nullptr, nullptr};
			for (size_t i = array_count(manager->publishedTx); i > 0; i--) {
				if (UInt256Eq(&(manager->publishedTxHashes[i - 1]), &txHash)) {
					manager->peerMessages->BRPeerSendTxMessage(peer, manager->publishedTx[i - 1].tx);
					break;
				}
			}
		}

	}
}