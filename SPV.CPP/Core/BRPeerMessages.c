// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <float.h>
#include <sys/time.h>
#include <stdlib.h>
#include <assert.h>

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

static int _BRPeerAcceptVersionMessage(BRPeer *peer, const uint8_t *msg, size_t msgLen)
{
	BRPeerContext *ctx = (BRPeerContext *)peer;
	size_t off = 0, strLen = 0, len = 0;
	uint64_t recvServices, fromServices, nonce;
	UInt128 recvAddr, fromAddr;
	uint16_t recvPort, fromPort;
	int r = 1;

	if (85 > msgLen) {
		peer_log(peer, "malformed version message, length is %zu, should be >= 85", msgLen);
		r = 0;
	}
	else {
		ctx->version = UInt32GetLE(&msg[off]);
		off += sizeof(uint32_t);
		peer->services = UInt64GetLE(&msg[off]);
		off += sizeof(uint64_t);
		peer->timestamp = UInt64GetLE(&msg[off]);
		off += sizeof(uint64_t);
		recvServices = UInt64GetLE(&msg[off]);
		off += sizeof(uint64_t);
		UInt128Get(&recvAddr, &msg[off]);
		off += sizeof(UInt128);
		recvPort = UInt16GetBE(&msg[off]);
		off += sizeof(uint16_t);
		fromServices = UInt64GetLE(&msg[off]);
		off += sizeof(uint64_t);
		UInt128Get(&fromAddr, &msg[off]);
		off += sizeof(UInt128);
		fromPort = UInt16GetBE(&msg[off]);
		off += sizeof(uint16_t);
		nonce = UInt64GetLE(&msg[off]);
		off += sizeof(uint64_t);
		strLen = (size_t)BRVarInt(&msg[off], (off <= msgLen ? msgLen - off : 0), &len);
		off += len;

		if (off + strLen + sizeof(uint32_t) > msgLen) {
			peer_log(peer, "malformed version message, length is %zu, should be %zu", msgLen,
					 off + strLen + sizeof(uint32_t));
			r = 0;
		}
		else if (ctx->version < MIN_PROTO_VERSION) {
			peer_log(peer, "protocol version %"PRIu32" not supported", ctx->version);
			r = 0;
		}
		else {
			array_clear(ctx->useragent);
			array_add_array(ctx->useragent, &msg[off], strLen);
			array_add(ctx->useragent, '\0');
			off += strLen;
			ctx->lastblock = UInt32GetLE(&msg[off]);
			off += sizeof(uint32_t);
			peer_log(peer, "got version %"PRIu32", services %"PRIx64", useragent:\"%s\"", ctx->version, peer->services,
					 ctx->useragent);
			BRPeerSendVerackMessage(peer);
		}
	}

	return r;
}

void BRPeerSendVersionMessage(BRPeer *peer)
{
	BRPeerContext *ctx = (BRPeerContext *)peer;
	size_t off = 0, userAgentLen = strlen(USER_AGENT);
	uint8_t msg[80 + BRVarIntSize(userAgentLen) + userAgentLen + 5];

	UInt32SetLE(&msg[off], PROTOCOL_VERSION); // version
	off += sizeof(uint32_t);
	UInt64SetLE(&msg[off], ENABLED_SERVICES); // services
	off += sizeof(uint64_t);
	UInt64SetLE(&msg[off], time(NULL)); // timestamp
	off += sizeof(uint64_t);
	UInt64SetLE(&msg[off], peer->services); // services of remote peer
	off += sizeof(uint64_t);
	UInt128Set(&msg[off], peer->address); // IPv6 address of remote peer
	off += sizeof(UInt128);
	UInt16SetBE(&msg[off], peer->port); // port of remote peer
	off += sizeof(uint16_t);
	UInt64SetLE(&msg[off], ENABLED_SERVICES); // services
	off += sizeof(uint64_t);
	UInt128Set(&msg[off], LOCAL_HOST); // IPv4 mapped IPv6 header
	off += sizeof(UInt128);
	UInt16SetBE(&msg[off], peer->port);
	off += sizeof(uint16_t);
	ctx->nonce = ((uint64_t)BRRand(0) << 32) | (uint64_t)BRRand(0); // random nonce
	UInt64SetLE(&msg[off], ctx->nonce);
	off += sizeof(uint64_t);
	off += BRVarIntSet(&msg[off], (off <= sizeof(msg) ? sizeof(msg) - off : 0), userAgentLen);
	strncpy((char *)&msg[off], USER_AGENT, userAgentLen); // user agent string
	off += userAgentLen;
	UInt32SetLE(&msg[off], 0); // last block received
	off += sizeof(uint32_t);
	msg[off++] = 0; // relay transactions (0 for SPV bloom filter mode)
	peer_log(peer, "%d", (int)peer->port);
	BRPeerSendMessage(peer, msg, sizeof(msg), MSG_VERSION);
}

static int _BRPeerAcceptAddrMessage(BRPeer *peer, const uint8_t *msg, size_t msgLen)
{
	BRPeerContext *ctx = (BRPeerContext *)peer;
	size_t off = 0, count = (size_t)BRVarInt(msg, msgLen, &off);
	int r = 1;

	if (off == 0 || off + count*30 > msgLen) {
		peer_log(peer, "malformed addr message, length is %zu, should be %zu for %zu address(es)", msgLen,
				 BRVarIntSize(count) + 30*count, count);
		r = 0;
	}
	else if (count > 1000) {
		peer_log(peer, "dropping addr message, %zu is too many addresses, max is 1000", count);
	}
	else if (ctx->sentGetaddr) { // simple anti-tarpitting tactic, don't accept unsolicited addresses
		BRPeer peers[count], p;
		size_t peersCount = 0;
		time_t now = time(NULL);

		peer_log(peer, "got addr with %zu address(es)", count);

		for (size_t i = 0; i < count; i++) {
			p.timestamp = UInt32GetLE(&msg[off]);
			off += sizeof(uint32_t);
			p.services = UInt64GetLE(&msg[off]);
			off += sizeof(uint64_t);
			UInt128Get(&p.address, &msg[off]);
			off += sizeof(UInt128);
			p.port = UInt16GetBE(&msg[off]);
			off += sizeof(uint16_t);

			if (! (p.services & SERVICES_NODE_NETWORK)) continue; // skip peers that don't carry full blocks
			if (! (peer->address.u64[0] == 0 && peer->address.u16[4] == 0 && peer->address.u16[5] == 0xffff))
				continue; // ignore IPv6 for now

			// if address time is more than 10 min in the future or unknown, set to 5 days old
			if (p.timestamp > now + 10*60 || p.timestamp == 0) p.timestamp = now - 5*24*60*60;
			p.timestamp -= 2*60*60; // subtract two hours
			peers[peersCount++] = p; // add it to the list
		}

		if (peersCount > 0 && ctx->relayedPeers) ctx->relayedPeers(ctx->info, peers, peersCount);
	}

	return r;
}

void BRPeerSendAddr(BRPeer *peer)
{
	uint8_t msg[BRVarIntSize(0)];
	size_t msgLen = BRVarIntSet(msg, sizeof(msg), 0);

	//TODO: send peer addresses we know about
	BRPeerSendMessage(peer, msg, msgLen, MSG_ADDR);
}


static int _BRPeerAcceptInvMessage(BRPeer *peer, const uint8_t *msg, size_t msgLen)
{
	BRPeerContext *ctx = (BRPeerContext *)peer;
	size_t off = 0, count = (size_t)BRVarInt(msg, msgLen, &off);
	int r = 1;

	if (off == 0 || off + count*36 > msgLen) {
		peer_log(peer, "malformed inv message, length is %zu, should be %zu for %zu item(s)", msgLen,
				 BRVarIntSize(count) + 36*count, count);
		r = 0;
	}
	else if (count > MAX_GETDATA_HASHES) {
		peer_log(peer, "dropping inv message, %zu is too many items, max is %d", count, MAX_GETDATA_HASHES);
	}
	else {
		inv_type type;
		const uint8_t *transactions[count], *blocks[count];
		size_t i, j, txCount = 0, blockCount = 0;

		peer_log(peer, "got inv with %zu item(s)", count);

		for (i = 0; i < count; i++) {
			type = UInt32GetLE(&msg[off]);

			switch (type) { // inv messages only use inv_tx or inv_block
				case inv_tx: transactions[txCount++] = &msg[off + sizeof(uint32_t)]; break;
				case inv_block: blocks[blockCount++] = &msg[off + sizeof(uint32_t)]; break;
				default: break;
			}

			off += 36;
		}

		if (txCount > 0 && ! ctx->sentFilter && ! ctx->sentMempool && ! ctx->sentGetblocks) {
			peer_log(peer, "got inv message before loading a filter");
			r = 0;
		}
		else if (txCount > 10000) { // sanity check
			peer_log(peer, "too many transactions, disconnecting");
			r = 0;
		}
		else if (ctx->currentBlockHeight > 0 && blockCount > 2 && blockCount < 500 &&
				 ctx->currentBlockHeight + array_count(ctx->knownBlockHashes) + blockCount < ctx->lastblock) {
			peer_log(peer, "non-standard inv, %zu is fewer block hash(es) than expected", blockCount);
			r = 0;
		}
		else {
			if (! ctx->sentFilter && ! ctx->sentGetblocks) blockCount = 0;
			UInt256 blockHash;
			UInt256Get(&blockHash, blocks[0]);
			if (blockCount == 1 && UInt256Eq(&(ctx->lastBlockHash), &blockHash)) blockCount = 0;
			if (blockCount == 1)UInt256Get(&ctx->lastBlockHash, blocks[0]);

			UInt256 hash, blockHashes[blockCount], txHashes[txCount];

			for (i = 0; i < blockCount; i++) {
				UInt256Get(&blockHashes[i],blocks[i]);
				// remember blockHashes in case we need to re-request them with an updated bloom filter
				array_add(ctx->knownBlockHashes, blockHashes[i]);
			}

			while (array_count(ctx->knownBlockHashes) > MAX_GETDATA_HASHES) {
				array_rm_range(ctx->knownBlockHashes, 0, array_count(ctx->knownBlockHashes)/3);
			}

			if (ctx->needsFilterUpdate) blockCount = 0;

			for (i = 0, j = 0; i < txCount; i++) {
				UInt256Get(&hash, transactions[i]);

				if (BRSetContains(ctx->knownTxHashSet, &hash)) {
					if (ctx->hasTx) ctx->hasTx(ctx->info, hash);
				}
				else txHashes[j++] = hash;
			}

			BRPeerAddKnownTxHashes(peer, txHashes, j);
			if (j > 0 || blockCount > 0) BRPeerSendGetdata(peer, txHashes, j, blockHashes, blockCount);

			// to improve chain download performance, if we received 500 block hashes, request the next 500 block hashes
			if (blockCount >= 500) {
				UInt256 locators[] = { blockHashes[blockCount - 1], blockHashes[0] };

				BRPeerSendGetblocks(peer, locators, 2, UINT256_ZERO);
			}

			if (txCount > 0 && ctx->mempoolCallback) {
				peer_log(peer, "got initial mempool response");
				BRPeerSendPing(peer, ctx->mempoolInfo, ctx->mempoolCallback);
				ctx->mempoolCallback = NULL;
				ctx->mempoolTime = DBL_MAX;
			}
		}
	}

	return r;
}

void BRPeerSendInv(BRPeer *peer, const UInt256 txHashes[], size_t txCount)
{
	BRPeerContext *ctx = (BRPeerContext *)peer;
	size_t knownCount = array_count(ctx->knownTxHashes);

	BRPeerAddKnownTxHashes(peer, txHashes, txCount);
	txCount = array_count(ctx->knownTxHashes) - knownCount;

	if (txCount > 0) {
		size_t i, off = 0, msgLen = BRVarIntSize(txCount) + (sizeof(uint32_t) + sizeof(*txHashes))*txCount;
		uint8_t msg[msgLen];

		off += BRVarIntSet(&msg[off], (off <= msgLen ? msgLen - off : 0), txCount);

		for (i = 0; i < txCount; i++) {
			UInt32SetLE(&msg[off], inv_tx);
			off += sizeof(uint32_t);
			UInt256Set(&msg[off], ctx->knownTxHashes[knownCount + i]);
			off += sizeof(UInt256);
		}

		BRPeerSendMessage(peer, msg, off, MSG_INV);
	}
}

static int _BRPeerAcceptGetaddrMessage(BRPeer *peer, const uint8_t *msg, size_t msgLen)
{
	peer_log(peer, "got getaddr");
	BRPeerSendAddr(peer);
	return 1;
}

void BRPeerSendGetaddr(BRPeer *peer)
{
	((BRPeerContext *)peer)->sentGetaddr = 1;
	BRPeerSendMessage(peer, NULL, 0, MSG_GETADDR);
}

void BRPeerAddKnownTxHashes(const BRPeer *peer, const UInt256 txHashes[], size_t txCount)
{
	BRPeerContext *ctx = (BRPeerContext *)peer;
	UInt256 *knownTxHashes = ctx->knownTxHashes;
	size_t i, j;

	for (i = 0; i < txCount; i++) {
		if (! BRSetContains(ctx->knownTxHashSet, &txHashes[i])) {
			array_add(knownTxHashes, txHashes[i]);

			if (ctx->knownTxHashes != knownTxHashes) { // check if knownTxHashes was moved to a new memory location
				ctx->knownTxHashes = knownTxHashes;
				BRSetClear(ctx->knownTxHashSet);
				for (j = array_count(knownTxHashes); j > 0; j--) BRSetAdd(ctx->knownTxHashSet, &knownTxHashes[j - 1]);
			}
			else BRSetAdd(ctx->knownTxHashSet, &knownTxHashes[array_count(knownTxHashes) - 1]);
		}
	}
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

static int _BRPeerAcceptNotfoundMessage(BRPeer *peer, const uint8_t *msg, size_t msgLen)
{
	BRPeerContext *ctx = (BRPeerContext *)peer;
	size_t off = 0, count = (size_t)BRVarInt(msg, msgLen, &off);
	int r = 1;

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
			type = UInt32GetLE(&msg[off]);
			UInt256Get(&hash, &msg[off + sizeof(uint32_t)]);

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

BRPeerMessages *BRPeerMessageNew(void) {
	BRPeerMessages *peerMessages = calloc(1, sizeof(*peerMessages));

	peerMessages->BRPeerAcceptVerackMessage = _BRPeerAcceptVerackMessage;
	peerMessages->BRPeerSendVerackMessage = BRPeerSendVerackMessage;

	peerMessages->BRPeerAcceptVersionMessage = _BRPeerAcceptVersionMessage;
	peerMessages->BRPeerSendVersionMessage = BRPeerSendVersionMessage;

	peerMessages->BRPeerAcceptAddressMessage = _BRPeerAcceptAddrMessage;
	peerMessages->BRPeerSendAddressMessage = BRPeerSendAddr;

	peerMessages->BRPeerAcceptInventoryMessage = _BRPeerAcceptInvMessage;
	peerMessages->BRPeerSendInventoryMessage = BRPeerSendInv;

	peerMessages->BRPeerAcceptGetAddressMessage = _BRPeerAcceptGetaddrMessage;
	peerMessages->BRPeerSendGetAddressMessage = BRPeerSendGetaddr;

	peerMessages->BRPeerAcceptTxMessage = _BRPeerAcceptTxMessage;
	peerMessages->BRPeerSendTxMessage = BRPeerSendTxMessage;

	peerMessages->BRPeerAcceptMerkleblockMessage = _BRPeerAcceptMerkleblockMessage;

	peerMessages->BRPeerAcceptNotFoundMessage = _BRPeerAcceptNotfoundMessage;
}

void BRPeerMessageFree(BRPeerMessages *peerMessages) {
	assert(peerMessages != NULL);
	if (peerMessages) {
		free(peerMessages);
	}
}
