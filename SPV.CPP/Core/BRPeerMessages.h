// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef BRPeerMessages_h
#define BRPeerMessages_h

#include <pthread.h>
#include <netinet/in.h>

#include "BRPeer.h"
#include "BRSet.h"

typedef struct {
	BRPeer peer; // superstruct on top of BRPeer
	uint32_t magicNumber;
	char host[INET6_ADDRSTRLEN];
	BRPeerStatus status;
	int waitingForNetwork;
	volatile int needsFilterUpdate;
	uint64_t nonce, feePerKb;
	char *useragent;
	uint32_t version, lastblock, earliestKeyTime, currentBlockHeight;
	double startTime, pingTime;
	volatile double disconnectTime, mempoolTime;
	int sentVerack, gotVerack, sentGetaddr, sentFilter, sentGetdata, sentMempool, sentGetblocks;
	UInt256 lastBlockHash;
	BRMerkleBlock *currentBlock;
	UInt256 *currentBlockTxHashes, *knownBlockHashes, *knownTxHashes;
	BRSet *knownTxHashSet;
	volatile int socket;
	void *info;
	void (*connected)(void *info);
	void (*disconnected)(void *info, int error);
	void (*relayedPeers)(void *info, const BRPeer peers[], size_t peersCount);
	void (*relayedTx)(void *info, BRTransaction *tx);
	void (*hasTx)(void *info, UInt256 txHash);
	void (*rejectedTx)(void *info, UInt256 txHash, uint8_t code);
	void (*relayedBlock)(void *info, BRMerkleBlock *block);
	void (*notfound)(void *info, const UInt256 txHashes[], size_t txCount, const UInt256 blockHashes[],
					 size_t blockCount);
	void (*setFeePerKb)(void *info, uint64_t feePerKb);
	BRTransaction *(*requestedTx)(void *info, UInt256 txHash);
	int (*networkIsReachable)(void *info);
	void (*threadCleanup)(void *info);
	void **volatile pongInfo;
	void (**volatile pongCallback)(void *info, int success);
	void *volatile mempoolInfo;
	void (*volatile mempoolCallback)(void *info, int success);
	pthread_t thread;
} BRPeerContext;

typedef struct {
	void (*BRPeerSendVerackMessage)(BRPeer *peer);
	int (*BRPeerAcceptVerackMessage)(BRPeer *peer, const uint8_t *msg, size_t msgLen);

} BRPeerMessages;

extern BRPeerMessages *GlobalMessages;

#endif //BRPeerMessages_h
