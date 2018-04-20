// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <float.h>
#include <sys/time.h>
#include <malloc.h>
#include <assert.h>
#include "BRPeer.h"
#include "BRPeerMessages.h"

static void _BRPeerDidConnect(BRPeer *peer)
{
	BRPeerContext *ctx = (BRPeerContext *)peer;

	if (ctx->status == BRPeerStatusConnecting && ctx->sentVerack && ctx->gotVerack) {
		peer_log(peer, "handshake completed");
		ctx->disconnectTime = DBL_MAX;
		ctx->status = BRPeerStatusConnected;
		peer_log(peer, "connected with lastblock: %"PRIu32, ctx->lastblock);
		if (ctx->connected) ctx->connected(ctx->info);
	}
}

static int _BRPeerAcceptVerackMessage(BRPeer *peer, const uint8_t *msg, size_t msgLen)
{
	BRPeerContext *ctx = (BRPeerContext *)peer;
	struct timeval tv;
	int r = 1;

	if (ctx->gotVerack) {
		peer_log(peer, "got unexpected verack");
	}
	else {
		gettimeofday(&tv, NULL);
		ctx->pingTime = tv.tv_sec + (double)tv.tv_usec/1000000 - ctx->startTime; // use verack time as initial ping time
		ctx->startTime = 0;
		peer_log(peer, "got verack in %fs", ctx->pingTime);
		ctx->gotVerack = 1;
		_BRPeerDidConnect(peer);
	}

	return r;
}

static void BRPeerSendVerackMessage(BRPeer *peer)
{
	BRPeerSendMessage(peer, NULL, 0, MSG_VERACK);
	((BRPeerContext *)peer)->sentVerack = 1;
}

BRPeerMessages *BRPeerMessageNew(void) {
	BRPeerMessages *peerMessages = calloc(1, sizeof(*peerMessages));
	peerMessages->BRPeerAcceptVerackMessage = _BRPeerAcceptVerackMessage;
	peerMessages->BRPeerSendVerackMessage = BRPeerSendVerackMessage;
}

void BRPeerMessageFree(BRPeerMessages *peerMessages) {
	assert(peerMessages != NULL);
	if (peerMessages) {
		free(peerMessages);
	}
}