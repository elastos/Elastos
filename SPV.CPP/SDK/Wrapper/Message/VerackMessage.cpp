// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <sys/time.h>
#include <cfloat>

#include "BRPeerMessages.h"

#include "VerackMessage.h"

namespace Elastos {
	namespace SDK {

		VerackMessage::VerackMessage() {

		}

		int VerackMessage::Accept(BRPeer *peer, const uint8_t *msg, size_t msgLen) {
			BRPeerContext *ctx = (BRPeerContext *)peer;
			struct timeval tv;
			int r = 1;

			if (ctx->gotVerack) {
				//todo instead with Log
//				peer_log(peer, "got unexpected verack");
			}
			else {
				gettimeofday(&tv, NULL);
				ctx->pingTime = tv.tv_sec + (double)tv.tv_usec/1000000 - ctx->startTime; // use verack time as initial ping time
				ctx->startTime = 0;
				//todo instead with Log
//				peer_log(peer, "got verack in %fs", ctx->pingTime);
				ctx->gotVerack = 1;
				BRPeerDidConnect(peer);
			}

			return r;
		}

		void VerackMessage::Send(BRPeer *peer) {
			BRPeerSendMessage(peer, NULL, 0, MSG_VERACK);
			((BRPeerContext *)peer)->sentVerack = 1;
		}

		void VerackMessage::BRPeerDidConnect(BRPeer *peer) {
			BRPeerContext *ctx = (BRPeerContext *)peer;

			if (ctx->status == BRPeerStatusConnecting && ctx->sentVerack && ctx->gotVerack) {
				//todo instead with Log
//				peer_log(peer, "handshake completed");
				ctx->disconnectTime = DBL_MAX;
				ctx->status = BRPeerStatusConnected;
//				peer_log(peer, "connected with lastblock: %"PRIu32, ctx->lastblock);
				if (ctx->connected) ctx->connected(ctx->info);
			}
		}
	}
}