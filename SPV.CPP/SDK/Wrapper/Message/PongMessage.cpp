// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <BRArray.h>
#include <sys/time.h>
#include <BRPeerMessages.h>

#include "PongMessage.h"
#include "Log.h"

namespace Elastos {
	namespace SDK {

		int PongMessage::Accept(BRPeer *peer, const uint8_t *msg, size_t msgLen) {
			BRPeerContext *ctx = (BRPeerContext *)peer;
			struct timeval tv;
			double pingTime;
			int r = 1;

			if (sizeof(uint64_t) > msgLen) {
				Log::getLogger()->warn("malformed pong message, length is %zu, should be %zu", msgLen, sizeof(uint64_t));
				r = 0;
			}
			else if (array_count(ctx->pongCallback) == 0) {
				Log::getLogger()->warn("got unexpected pong");
				r = 0;
			}
			else {
				if (ctx->startTime > 1) {
					gettimeofday(&tv, nullptr);
					pingTime = tv.tv_sec + (double)tv.tv_usec/1000000 - ctx->startTime;

					// 50% low pass filter on current ping time
					ctx->pingTime = ctx->pingTime*0.5 + pingTime*0.5;
					ctx->startTime = 0;
					Log::getLogger()->info("got pong in {}", pingTime);
				}
				else Log::getLogger()->info("got pong");

				if (array_count(ctx->pongCallback) > 0) {
					void (*pongCallback)(void *, int) = ctx->pongCallback[0];
					void *pongInfo = ctx->pongInfo[0];

					array_rm(ctx->pongCallback, 0);
					array_rm(ctx->pongInfo, 0);
					if (pongCallback) pongCallback(pongInfo, 1);
				}
			}

			return r;
		}

		void PongMessage::Send(BRPeer *peer) {

		}
	}
}