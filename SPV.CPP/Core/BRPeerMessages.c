// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <float.h>
#include <sys/time.h>
#include <malloc.h>
#include <assert.h>
#include "BRPeer.h"
#include "BRPeerMessages.h"
#include "BRArray.h"

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

static int _BRPeerAcceptTxMessage(BRPeer *peer, const uint8_t *msg, size_t msgLen)
{
	BRPeerContext *ctx = (BRPeerContext *)peer;
	BRTransaction *tx = BRTransactionParse(msg, msgLen);
	UInt256 txHash;
	int r = 1;

	if (! tx) {
		peer_log(peer, "malformed tx message with length: %zu", msgLen);
		r = 0;
	}
	else if (! ctx->sentFilter && ! ctx->sentGetdata) {
		peer_log(peer, "got tx message before loading filter");
		BRTransactionFree(tx);
		r = 0;
	}
	else {
		txHash = tx->txHash;
		peer_log(peer, "got tx: %s", u256hex(txHash));

		if (ctx->relayedTx) {
			ctx->relayedTx(ctx->info, tx);
		}
		else BRTransactionFree(tx);

		if (ctx->currentBlock) { // we're collecting tx messages for a merkleblock
			for (size_t i = array_count(ctx->currentBlockTxHashes); i > 0; i--) {
				if (! UInt256Eq(&txHash, &(ctx->currentBlockTxHashes[i - 1]))) continue;
				array_rm(ctx->currentBlockTxHashes, i - 1);
				break;
			}

			if (array_count(ctx->currentBlockTxHashes) == 0) { // we received the entire block including all matched tx
				BRMerkleBlock *block = ctx->currentBlock;

				ctx->currentBlock = NULL;
				if (ctx->relayedBlock) ctx->relayedBlock(ctx->info, block);
			}
		}
	}

	return r;
}

static void BRPeerSendTxMessage(BRPeer *peer, BRTransaction *tx)
{
	uint8_t buf[BRTransactionSerialize(tx, NULL, 0)];
	size_t bufLen = BRTransactionSerialize(tx, buf, sizeof(buf));
	char txHex[bufLen*2 + 1];

	for (size_t j = 0; j < bufLen; j++) {
		sprintf(&txHex[j*2], "%02x", buf[j]);
	}

	peer_log(peer, "publishing tx: %s", txHex);
	BRPeerSendMessage(peer, buf, bufLen, MSG_TX);
}

BRPeerMessages *BRPeerMessageNew(void) {
	BRPeerMessages *peerMessages = calloc(1, sizeof(*peerMessages));

	peerMessages->BRPeerAcceptVerackMessage = _BRPeerAcceptVerackMessage;
	peerMessages->BRPeerSendVerackMessage = BRPeerSendVerackMessage;

	peerMessages->BRPeerAcceptTxMessage = _BRPeerAcceptTxMessage;
	peerMessages->BRPeerSendTxMessage = BRPeerSendTxMessage;
}

void BRPeerMessageFree(BRPeerMessages *peerMessages) {
	assert(peerMessages != NULL);
	if (peerMessages) {
		free(peerMessages);
	}
}
