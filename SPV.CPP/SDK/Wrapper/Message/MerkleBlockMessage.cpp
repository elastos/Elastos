// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <BRMerkleBlock.h>
#include <BRPeerMessages.h>
#include "BRPeerMessages.h"
#include "BRArray.h"

#include "MerkleBlock.h"
#include "MerkleBlockMessage.h"
#include "Log.h"
#include "Utils.h"

namespace Elastos {
	namespace SDK {

		int MerkleBlockMessage::Accept(BRPeer *peer, const uint8_t *msg, size_t msgLen) {

			BRPeerContext *ctx = (BRPeerContext *) peer;
			// msg is holding by payload pointer create by malloc, do not match delete[] in ByteStream
			ByteStream stream(const_cast<uint8_t *>(msg), msgLen, false);
			MerkleBlock wrappedBlock;
			wrappedBlock.Deserialize(stream);

			BRMerkleBlock *block = BRMerkleBlockCopy(wrappedBlock.getRaw());
			int r = 1;

			if (!block) {
				Log::getLogger()->warn("malformed merkleblock message with length: %zu", msgLen);
				r = 0;
#if 0
			} else if (!BRMerkleBlockIsValid(block, (uint32_t) time(nullptr))) {
				Log::getLogger()->warn("invalid merkleblock: {}",
									   Utils::UInt256ToString(block->blockHash));
				BRMerkleBlockFree(block);
				block = nullptr;
				r = 0;
#endif
			} else if (!ctx->sentFilter && !ctx->sentGetdata) {
				Log::getLogger()->warn("got merkleblock message before loading a filter");
				BRMerkleBlockFree(block);
				block = nullptr;
				r = 0;
			} else {
#if 0
				size_t count = BRMerkleBlockTxHashes(block, nullptr, 0);
				UInt256 _hashes[(sizeof(UInt256) * count <= 0x1000) ? count : 0];
				UInt256 *hashes = (sizeof(UInt256) * count <= 0x1000)
								  ? _hashes : (UInt256 *)malloc(count * sizeof(*hashes));

				assert(hashes != nullptr);
				count = BRMerkleBlockTxHashes(block, hashes, count);

				for (size_t i = count; i > 0; i--) { // reverse order for more efficient removal as tx arrive
					if (BRSetContains(ctx->knownTxHashSet, &hashes[i - 1])) continue;
					array_add(ctx->currentBlockTxHashes, hashes[i - 1]);
				}

				if (hashes != _hashes) free(hashes);
#endif

				for (size_t i = block->hashesCount; i > 0; --i) {
					if (BRSetContains(ctx->knownTxHashSet, &block->hashes[i])) continue;
					array_add(ctx->currentBlockTxHashes, block->hashes[i]);
				}
				BRMerkleBlockFree(block);
				block = nullptr;
			}

			if (block) {
				if (array_count(ctx->currentBlockTxHashes) >
					0) { // wait til we get all tx messages before processing the block
					ctx->currentBlock = block;
				} else if (ctx->relayedBlock) {
					ctx->relayedBlock(ctx->info, block);
				} else {
					BRMerkleBlockFree(block);
				}
			}

			return r;
		}

		void MerkleBlockMessage::Send(BRPeer *peer, void *serializable) {
		}
	}
}
