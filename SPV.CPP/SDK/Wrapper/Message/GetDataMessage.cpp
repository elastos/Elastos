// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <BRPeerMessages.h>
#include <BRPeerManager.h>
#include "BRArray.h"
#include "BRPeerMessages.h"

#include "GetDataMessage.h"
#include "Log.h"
#include "Utils.h"

namespace Elastos {
	namespace SDK {


		void GetDataMessage::SendGetData(BRPeer *peer, const UInt256 *txHashes,
		                                 size_t txCount, const UInt256 *blockHashes, size_t blockCount) {
			size_t i, off = 0;
			size_t count = txCount > 0 ? txCount : blockCount;
			size_t msgLen = sizeof(uint32_t) + count * (sizeof(uint32_t) + sizeof(UInt256));
			uint8_t msg[msgLen];

			UInt32SetLE(&msg[off], uint32_t(count));
			off += sizeof(uint32_t);
			for (i = 0; i < txCount; i++) {
				UInt32SetLE(&msg[off], uint32_t(inv_tx));
				off += sizeof(uint32_t);
				UInt256Set(&msg[off], txHashes[i]);
				off += sizeof(UInt256);
			}

			for (i = 0; i < blockCount; i++) {
				UInt32SetLE(&msg[off], uint32_t(inv_filtered_block));
				off += sizeof(uint32_t);
				UInt256Set(&msg[off], blockHashes[i]);
				off += sizeof(UInt256);
			}
			BRPeerSendMessage(peer, msg, off, MSG_GETDATA);
			((BRPeerContext *) peer)->sentGetdata = 1;
		}

		int GetDataMessage::Accept(BRPeer *peer, const uint8_t *msg, size_t msgLen) {

			BRPeerContext *ctx = (BRPeerContext *) peer;
			size_t off = 0, count = -1;
			int r = 1;

			count = UInt32GetLE(&msg[off]);
			off += sizeof(uint32_t);

			if (off == 0 || off + 36 * count > msgLen) {
				peer_log(peer, "malformed getdata message, length is %zu, should %zu for %zu item(s)", msgLen,
				         BRVarIntSize(count) + 36 * count, count);
				r = 0;
			} else if (count > MAX_GETDATA_HASHES) {
				peer_log(peer, "dropping getdata message, %zu is too many items, max is %d", count, MAX_GETDATA_HASHES);
			} else {
				struct inv_item {
					uint8_t item[36];
				} *notfound = NULL;
				BRTransaction *tx = NULL;

				peer_log(peer, "got getdata with %zu item(s)", count);
				for (size_t i = 0; i < count; i++) {
					inv_type type = (inv_type) UInt32GetLE(&msg[off]);
					UInt256 hash;
					UInt256Get(&hash, &msg[off + sizeof(uint32_t)]);
					switch (type) {
						case inv_tx:
							if (ctx->requestedTx) tx = ctx->requestedTx(ctx->info, hash);

							if (tx && BRTransactionSize(tx) < TX_MAX_SIZE) {
								ctx->manager->peerMessages->BRPeerSendTxMessage(peer, tx);
								break;
							}

							// fall through
						default:
							if (!notfound) array_new(notfound, 1);
							array_add(notfound, *(struct inv_item *) &msg[off]);
							break;
					}

					off += 36;
				}
				if (notfound) {
					size_t bufLen = BRVarIntSize(array_count(notfound)) + 36 * array_count(notfound), o = 0;
					uint8_t *buf = (uint8_t *) malloc(bufLen);

					assert(buf != NULL);
					UInt32SetLE(&buf[o], array_count(notfound));
					o += sizeof(uint32_t);
					memcpy(&buf[o], notfound, 36 * array_count(notfound));
					o += 36 * array_count(notfound);
					array_free(notfound);

					BRPeerSendMessage(peer, buf, o, MSG_NOTFOUND);
					free(buf);
				}
			}
			return r;
		}

		void GetDataMessage::Send(BRPeer *peer) {

		}
	}
}