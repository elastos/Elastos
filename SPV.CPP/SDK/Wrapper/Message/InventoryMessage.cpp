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
#include "Utils.h"

#define MAX_BLOCKS_COUNT 100  //note max blocks count is 500 in btc while 100 in ela

namespace Elastos {
	namespace ElaWallet {

		InventoryMessage::InventoryMessage() {

		}

		int InventoryMessage::Accept(BRPeer *peer, const uint8_t *msg, size_t msgLen) {
			BRPeerContext *ctx = (BRPeerContext *) peer;
			size_t off = 0;
			inv_type type;

			size_t count = UInt32GetLE(&msg[off]);
			off += sizeof(uint32_t);

			int r = 1;

			if (off + count * 36 > msgLen) {
				peer_log(peer, "malformed inv message, length is %zu, should be %zu for %zu item(s)", msgLen,
					off + count * 36, count);
				r = 0;
			} else if (count > MAX_GETDATA_HASHES) {
				peer_log(peer, "dropping inv message, %zu is too many items, max is %d", count,
									   MAX_GETDATA_HASHES);
			} else {
				const uint8_t *transactions[count], *blocks[count];
				size_t i, j, txCount = 0, blockCount = 0;

				peer_log(peer, "got inv with %zu item(s)", count);

				for (i = 0; i < count; i++) {
					type = inv_type(UInt32GetLE(&msg[off]));
					off += sizeof(uint32_t);

					if (type == inv_block) {
						blocks[blockCount++] = &msg[off];
						off += sizeof(UInt256);
					} else if (type == inv_tx) {
						transactions[txCount++] = &msg[off];
						off += sizeof(UInt256);
					}
				}

				if (txCount > 0 && !ctx->sentFilter && !ctx->sentMempool && !ctx->sentGetblocks) {
					peer_log(peer, "got inv message before loading a filter");
					r = 0;
				} else if (txCount > 10000) { // sanity check
					peer_log(peer, "too many transactions, disconnecting");
					r = 0;
				} else if (ctx->currentBlockHeight > 0 && blockCount > 2 && blockCount < MAX_BLOCKS_COUNT &&
						   ctx->currentBlockHeight + array_count(ctx->knownBlockHashes) + blockCount < ctx->lastblock) {
					peer_log(peer, "non-standard inv, %zu is fewer block hash(es) than expected", blockCount);
					r = 0;
				} else {
					if (!ctx->sentFilter && !ctx->sentGetblocks) blockCount = 0;
					UInt256 blockHash;
					if (blockCount > 0) {
						UInt256Get(&blockHash, blocks[0]);
						if (blockCount == 1 && UInt256Eq(&(ctx->lastBlockHash), &blockHash)) blockCount = 0;
						if (blockCount == 1) UInt256Get(&ctx->lastBlockHash, blocks[0]);
					}

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

					peer_log(peer, "got inv with txCount=%zu, blockCount=%zu", j, blockCount);
					BRPeerAddKnownTxHashes(peer, txHashes, j);
					if (j > 0 || blockCount > 0)
						ctx->manager->peerMessages->BRPeerSendGetdataMessage(peer, txHashes, j, blockHashes, blockCount);

					// to improve chain download performance, if we received 500 block hashes, request the next 500 block hashes
					if (blockCount >= MAX_BLOCKS_COUNT) {
						UInt256 locators[] = {blockHashes[blockCount - 1], blockHashes[0]};

						ctx->manager->peerMessages->BRPeerSendGetblocksMessage(peer, locators, 2, UINT256_ZERO);
					}

					if (txCount > 0 && ctx->mempoolCallback) {
						peer_log(peer, "got initial mempool response");
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
			BRPeerContext *ctx = (BRPeerContext *) peer;
			size_t knownCount = array_count(ctx->knownTxHashes);

			BRPeerAddKnownTxHashes(peer, txHashes, txCount);
			txCount = array_count(ctx->knownTxHashes) - knownCount;

			if (txCount > 0) {
				size_t i, off = 0, msgLen = sizeof(uint32_t) + (sizeof(uint32_t) + sizeof(*txHashes)) * txCount;
				uint8_t msg[msgLen];
				UInt32SetLE(&msg[off], txCount);
				off += sizeof(uint32_t);
				for (size_t i = 0; i < txCount; i++) {
					UInt32SetLE(&msg[off], inv_tx); // version
					off += sizeof(uint32_t);
					UInt256Set(&msg[off], ctx->knownTxHashes[knownCount + i]);
					off += sizeof(UInt256);
				}
				peer_log(peer, "sending inv tx count=%zu type=%d", txCount, inv_tx);
				BRPeerSendMessage(peer, msg, off, MSG_INV);
			}
		}

	}
}