// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <BRPeerMessages.h>
#include <BRPeerManager.h>
#include "BRArray.h"
#include "BRPeerMessages.h"

#include "GetDataMessage.h"
#include "Log.h"

namespace Elastos {
	namespace SDK {


		void GetDataMessage::SendGetData(BRPeer *peer, const UInt256 *txHashes,
										 size_t txCount, const UInt256 *blockHashes, size_t blockCount) {
			size_t msgLen = sizeof(uint8_t) + sizeof(UInt256);
			uint8_t msg[msgLen];

			size_t i, off;
			for (i = 0; i < txCount; i++) {
				off = 0;
				msg[off] = inv_tx;
				off += sizeof(uint8_t);
				UInt256Set(&msg[off], txHashes[i]);
				off += sizeof(UInt256);

				BRPeerSendMessage(peer, msg, off, MSG_GETDATA);
			}

			for (i = 0; i < blockCount; i++) {
				off = 0;
				msg[off] = inv_block;	//todo core get message use inv_filtered_block here, figure out differences later
				off += sizeof(uint8_t);
				UInt256Set(&msg[off], blockHashes[i]);
				off += sizeof(UInt256);

				BRPeerSendMessage(peer, msg, off, MSG_GETDATA);
			}

			((BRPeerContext *) peer)->sentGetdata = 1;
		}

		int GetDataMessage::Accept(BRPeer *peer, const uint8_t *msg, size_t msgLen) {

			BRPeerContext *ctx = (BRPeerContext *) peer;
			size_t off = 0;

			int r = 1;

			BRTransaction *tx = nullptr;

			inv_type type = (inv_type) msg[off];
			off += sizeof(uint8_t);

			UInt256 hash;
			UInt256Get(&hash, &msg[off]);

			bool notfound = true;
			if (type == inv_tx) {
				if (ctx->requestedTx) tx = ctx->requestedTx(ctx->info, hash);

				if (tx && BRTransactionSize(tx) < TX_MAX_SIZE) {
					notfound = false;
					ctx->manager->peerMessages->BRPeerSendTxMessage(peer, tx);
				}
			}

			if (notfound) {
				size_t bufLength = sizeof(hash); // + sizeof(uint8_t);
				off = 0;

				uint8_t buf[bufLength];

				//todo uncomment this when notfound message of ELA.Utility updated
//				buf[off] = uint8_t(type);
//				off += sizeof(uint8_t);
				UInt256Set(&buf[off], hash);
				BRPeerSendMessage(peer, buf, bufLength, MSG_NOTFOUND);
			}

			return r;
		}

		void GetDataMessage::Send(BRPeer *peer) {

		}
	}
}