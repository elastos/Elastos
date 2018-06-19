// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <cfloat>

#include "BRArray.h"
#include "BRPeerMessages.h"

#include "NotFoundMessage.h"
#include "Log.h"

namespace Elastos {
	namespace ElaWallet {

		NotFoundMessage::NotFoundMessage() {

		}

		int NotFoundMessage::Accept(BRPeer *peer, const uint8_t *msg, size_t msgLen) {
			Log::getLogger()->info("NotFoundMessage.Accept");

			BRPeerContext *ctx = (BRPeerContext *) peer;
			int r = 1;

			inv_type type;
			UInt256 *txHashes, *blockHashes, hash;

			array_new(txHashes, 1);
			array_new(blockHashes, 1);

			size_t off = 0;

			//todo uncomment this when notfound message of ELA.Utility updated
//			type = inv_type(msg[off]);
//			off += sizeof(uint8_t);

			UInt256Get(&hash, &msg[off]);

			switch (type) {
				case inv_tx:
					array_add(txHashes, hash);
					break;
				case inv_filtered_block: // drop through
				case inv_block:
					array_add(blockHashes, hash);
					break;
				default:
					break;
			}

			if (ctx->notfound) {
				ctx->notfound(ctx->info, txHashes, array_count(txHashes), blockHashes, array_count(blockHashes));
			}

			array_free(txHashes);
			array_free(blockHashes);

			return r;
		}

		void NotFoundMessage::Send(BRPeer *peer) {
		}
	}
}