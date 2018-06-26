// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <cfloat>

#include "BRInt.h"
#include "BRArray.h"
#include "BRPeerMessages.h"

#include "NotFoundMessage.h"
#include "Log.h"
#include "Utils.h"

namespace Elastos {
	namespace ElaWallet {

		NotFoundMessage::NotFoundMessage() {

		}

		int NotFoundMessage::Accept(BRPeer *peer, const uint8_t *msg, size_t msgLen) {
			BRPeerContext *ctx = (BRPeerContext *)peer;
			size_t off = 0, count = 0;
			int r = 1;

			count = UInt32GetLE(&msg[off]);
			off += sizeof(uint32_t);

			if (off == 0 || off + 36*count > msgLen) {
				peer_log(peer, "malformed notfound message, length is %zu, should be %zu for %zu item(s)", msgLen,
						 BRVarIntSize(count) + 36*count, count);
				r = 0;
			}
			else if (count > MAX_GETDATA_HASHES) {
				peer_log(peer, "dropping notfound message, %zu is too many items, max is %d", count, MAX_GETDATA_HASHES);
			}
			else {
				inv_type type;
				UInt256 *txHashes, *blockHashes, hash;

				peer_log(peer, "got notfound with %zu item(s)", count);
				array_new(txHashes, 1);
				array_new(blockHashes, 1);

				for (size_t i = 0; i < count; i++) {
					type = (inv_type)UInt32GetLE(&msg[off]);
					UInt256Get(&hash, &msg[off + sizeof(uint32_t)]);

					Log::getLogger()->info("not found type = {}, hash = {}", type, Utils::UInt256ToString(hash));

					switch (type) {
						case inv_tx: array_add(txHashes, hash); break;
						case inv_filtered_block: // drop through
						case inv_block: array_add(blockHashes, hash); break;
						default: break;
					}

					off += 36;
				}

				if (ctx->notfound) {
					ctx->notfound(ctx->info, txHashes, array_count(txHashes), blockHashes, array_count(blockHashes));
				}

				array_free(txHashes);
				array_free(blockHashes);
			}

			return r;
		}

		void NotFoundMessage::Send(BRPeer *peer) {
		}
	}
}