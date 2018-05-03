// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <cfloat>

#include "BRArray.h"

#include "ELAPeerContext.h"

namespace Elastos {
	namespace SDK {

		static void _dummyThreadCleanup(void *info)
		{
		}

		BRPeer *ELAPeerNew(uint32_t magicNumber)
		{
			ELAPeerContext *elactx = new ELAPeerContext;
			BRPeerContext *ctx = (BRPeerContext *)elactx;

			assert(ctx != nullptr);
			memset(ctx, 0, sizeof(*ctx));

			ctx->magicNumber = magicNumber;
			array_new(ctx->useragent, 40);
			array_new(ctx->knownBlockHashes, 10);
			array_new(ctx->currentBlockTxHashes, 10);
			array_new(ctx->knownTxHashes, 10);
			ctx->knownTxHashSet = BRSetNew(BRTransactionHash, BRTransactionEq, 10);
			array_new(ctx->pongInfo, 10);
			array_new(ctx->pongCallback, 10);
			ctx->pingTime = DBL_MAX;
			ctx->mempoolTime = DBL_MAX;
			ctx->disconnectTime = DBL_MAX;
			ctx->socket = -1;
			ctx->threadCleanup = _dummyThreadCleanup;
			return &ctx->peer;
		}

		BRPeer *ELAPeerCopy(const BRPeer *peer)
		{
			BRPeerContext *ctx = (BRPeerContext *)peer;
			BRPeer *newPeer = ELAPeerNew(ctx->magicNumber);
			newPeer->timestamp = peer->timestamp;
			newPeer->address = peer->address;
			newPeer->port = peer->port;
			newPeer->flags = peer->flags;
			newPeer->services = peer->services;

			return newPeer;
		}

		void ELAPeerFree(BRPeer *peer)
		{
			ELAPeerContext *elactx = (ELAPeerContext *)peer;
			BRPeerContext *ctx = (BRPeerContext *)peer;

			if (ctx->useragent) array_free(ctx->useragent);
			if (ctx->currentBlockTxHashes) array_free(ctx->currentBlockTxHashes);
			if (ctx->knownBlockHashes) array_free(ctx->knownBlockHashes);
			if (ctx->knownTxHashes) array_free(ctx->knownTxHashes);
			if (ctx->knownTxHashSet) BRSetFree(ctx->knownTxHashSet);
			if (ctx->pongCallback) array_free(ctx->pongCallback);
			if (ctx->pongInfo) array_free(ctx->pongInfo);
			delete elactx;
		}

		bool UInt256Compare::operator()(const UInt256 &a, const UInt256 &b) const {
			if (a.u64[0] != b.u64[0]) {
				return a.u64[0] < b.u64[0];
			} else if (a.u64[1] != b.u64[1]) {
				return a.u64[1] < b.u64[1];
			} else if (a.u64[2] != b.u64[2]) {
				return a.u64[2] < b.u64[2];
			} else {
				return a.u64[3] < b.u64[3];
			}
		}
	}
}