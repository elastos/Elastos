// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license(BRPeerContext *) peer;, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <sys/time.h>
#include <cfloat>

#include "BRPeerMessages.h"

#include "VerackMessage.h"
#include "Log.h"

namespace Elastos {
	namespace SDK {

		VerackMessage::VerackMessage() {

		}

		int VerackMessage::Accept(BRPeer *peer, const uint8_t *msg, size_t msgLen) {
			Log::getLogger()->warn("VerackMessage.Accept");

			BRPeerContext *ctx = (BRPeerContext *)peer;
			struct timeval tv;
			int r = 1;

			if (ctx->gotVerack) {
				Log::getLogger()->warn("got unexpected verack");
			}
			else {
				gettimeofday(&tv, nullptr);
				ctx->pingTime = tv.tv_sec + (double)tv.tv_usec/1000000 - ctx->startTime; // use verack time as initial ping time
				ctx->startTime = 0;
				Log::getLogger()->warn("got verack in %fs", ctx->pingTime);
				ctx->gotVerack = 1;
				BRPeerDidConnect(peer);
			}

			return r;
		}

		void VerackMessage::Send(BRPeer *peer) {
			Log::getLogger()->warn("VerackMessage.Send");
			BRPeerSendMessage(peer, nullptr, 0, MSG_VERACK);
			((BRPeerContext *)peer)->sentVerack = 1;
		}

		void VerackMessage::BRPeerDidConnect(BRPeer *peer) {
			Log::getLogger()->warn("VerackMessage.BRPeerDidConnect");
			BRPeerContext *ctx = (BRPeerContext *)peer;

			if (ctx->status == BRPeerStatusConnecting && ctx->sentVerack && ctx->gotVerack) {
				Log::getLogger()->warn("handshake completed");
				ctx->disconnectTime = DBL_MAX;
				ctx->status = BRPeerStatusConnected;
				Log::getLogger()->warn("connected with lastblock: %"PRIu32, ctx->lastblock);
				if (ctx->connected) ctx->connected(ctx->info);
			}
		}
	}
}