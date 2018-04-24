// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <float.h>
#include <sys/time.h>
#include <stdlib.h>
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

static int _BRPeerAcceptMerkleblockMessage(BRPeer *peer, const uint8_t *msg, size_t msgLen)
{
	// Bitcoin nodes don't support querying arbitrary transactions, only transactions not yet accepted in a block. After
	// a merkleblock message, the remote node is expected to send tx messages for the tx referenced in the block. When a
	// non-tx message is received we should have all the tx in the merkleblock.
	BRPeerContext *ctx = (BRPeerContext *)peer;
	BRMerkleBlock *block = BRMerkleBlockParse(msg, msgLen);
	int r = 1;

	if (! block) {
		peer_log(peer, "malformed merkleblock message with length: %zu", msgLen);
		r = 0;
	}
	else if (! BRMerkleBlockIsValid(block, (uint32_t)time(NULL))) {
		peer_log(peer, "invalid merkleblock: %s", u256hex(block->blockHash));
		BRMerkleBlockFree(block);
		block = NULL;
		r = 0;
	}
	else if (! ctx->sentFilter && ! ctx->sentGetdata) {
		peer_log(peer, "got merkleblock message before loading a filter");
		BRMerkleBlockFree(block);
		block = NULL;
		r = 0;
	}
	else {
		size_t count = BRMerkleBlockTxHashes(block, NULL, 0);
		UInt256 _hashes[(sizeof(UInt256)*count <= 0x1000) ? count : 0],
				*hashes = (sizeof(UInt256)*count <= 0x1000) ? _hashes : malloc(count*sizeof(*hashes));

		assert(hashes != NULL);
		count = BRMerkleBlockTxHashes(block, hashes, count);

		for (size_t i = count; i > 0; i--) { // reverse order for more efficient removal as tx arrive
			if (BRSetContains(ctx->knownTxHashSet, &hashes[i - 1])) continue;
			array_add(ctx->currentBlockTxHashes, hashes[i - 1]);
		}

		if (hashes != _hashes) free(hashes);
	}

	if (block) {
		if (array_count(ctx->currentBlockTxHashes) > 0) { // wait til we get all tx messages before processing the block
			ctx->currentBlock = block;
		}
		else if (ctx->relayedBlock) {
			ctx->relayedBlock(ctx->info, block);
		}
		else BRMerkleBlockFree(block);
	}

	return r;
}

BRPeerMessages *BRPeerMessageNew(void) {
	BRPeerMessages *peerMessages = calloc(1, sizeof(*peerMessages));

	peerMessages->BRPeerAcceptVerackMessage = _BRPeerAcceptVerackMessage;
	peerMessages->BRPeerSendVerackMessage = BRPeerSendVerackMessage;

	peerMessages->BRPeerAcceptTxMessage = _BRPeerAcceptTxMessage;
	peerMessages->BRPeerSendTxMessage = BRPeerSendTxMessage;

	peerMessages->BRPeerAcceptMerkleblockMessage = _BRPeerAcceptMerkleblockMessage;
}

void BRPeerMessageFree(BRPeerMessages *peerMessages) {
	assert(peerMessages != NULL);
	if (peerMessages) {
		free(peerMessages);
	}
}
