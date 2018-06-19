// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <sys/time.h>
#include <BRPeerMessages.h>
#include <BRMerkleBlock.h>

#include "BRArray.h"
#include "BRPeerMessages.h"
#include "BRPeerManager.h"
#include "BRWallet.h"

#include "PingMessage.h"

namespace Elastos {
	namespace ElaWallet {

		int PingMessage::Accept(BRPeer *peer, const uint8_t *msg, size_t msgLen) {
			return BRPeerAcceptPingMessage(peer, msg, msgLen);
		}

		void PingMessage::Send(BRPeer *peer) {

		}

		void PingMessage::sendPing(BRPeer *peer, void *info, void (*pongCallback)(void *, int)) {
			BRPeerContext *ctx = (BRPeerContext *)peer;
			uint8_t msg[sizeof(uint64_t)];
			struct timeval tv;

			gettimeofday(&tv, nullptr);
			ctx->startTime = tv.tv_sec + (double)tv.tv_usec/1000000;
			array_add(ctx->pongInfo, info);
			array_add(ctx->pongCallback, pongCallback);
			UInt64SetLE(msg, ctx->manager->lastBlock->height);
			BRPeerSendMessage(peer, msg, sizeof(msg), MSG_PING);
		}
	}
}