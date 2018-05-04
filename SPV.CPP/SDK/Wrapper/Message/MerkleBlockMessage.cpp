// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <BRMerkleBlock.h>

#include "BRPeerManager.h"
#include "BRPeerMessages.h"
#include "BRArray.h"
#include "BRMerkleBlock.h"

#include "Peer.h"
#include "MerkleBlock.h"
#include "MerkleBlockMessage.h"
#include "Log.h"
#include "Utils.h"
#include "AuxPow.h"
#include "ELACoreExt/ELAPeerContext.h"

namespace Elastos {
	namespace SDK {

		namespace {

#define MAX_PROOF_OF_WORK 0x1d00ffff    // highest value for difficulty target (higher values are less difficult)

			inline static int _ceil_log2(int x)
			{
				int r = (x & (x - 1)) ? 1 : 0;

				while ((x >>= 1) != 0) r++;
				return r;
			}

			// recursively walks the merkle tree to calculate the merkle root
			// NOTE: this merkle tree design has a security vulnerability (CVE-2012-2459), which can be defended against by
			// considering the merkle root invalid if there are duplicate hashes in any rows with an even number of elements
			static UInt256 MerkleBlockRootR(const MerkleBlock &block, size_t *hashIdx, size_t *flagIdx, int depth)
			{
				uint8_t flag;
				UInt256 hashes[2], md = UINT256_ZERO;

				if (*flagIdx/8 < block.getRaw()->flagsLen && *hashIdx < block.getRaw()->hashesCount) {
					flag = (block.getRaw()->flags[*flagIdx/8] & (1 << (*flagIdx % 8)));
					(*flagIdx)++;

					if (flag && depth != _ceil_log2(block.getRaw()->totalTx)) {
						hashes[0] = MerkleBlockRootR(block, hashIdx, flagIdx, depth + 1); // left branch
						hashes[1] = MerkleBlockRootR(block, hashIdx, flagIdx, depth + 1); // right branch

						if (! UInt256IsZero(&hashes[0]) && ! UInt256Eq(&(hashes[0]), &(hashes[1]))) {
							if (UInt256IsZero(&hashes[1])) hashes[1] = hashes[0]; // if right branch is missing, dup left branch
							BRSHA256_2(&md, hashes, sizeof(hashes));
						}
						else *hashIdx = SIZE_MAX; // defend against (CVE-2012-2459)
					}
					else md = block.getRaw()->hashes[(*hashIdx)++]; // leaf
				}

				return md;
			}

			// true if merkle tree and timestamp are valid, and proof-of-work matches the stated difficulty target
			// NOTE: this only checks if the block difficulty matches the difficulty target in the header, it does not check if the
			// target is correct for the block's height in the chain - use BRMerkleBlockVerifyDifficulty() for that
			int MerkleBlockIsValid(const MerkleBlock &block, uint32_t currentTime)
			{
				// target is in "compact" format, where the most significant byte is the size of resulting value in bytes, the next
				// bit is the sign, and the remaining 23bits is the value after having been right shifted by (size - 3)*8 bits
				static const uint32_t maxsize = MAX_PROOF_OF_WORK >> 24, maxtarget = MAX_PROOF_OF_WORK & 0x00ffffff;
				const uint32_t size = block.getRaw()->target >> 24, target = block.getRaw()->target & 0x00ffffff;
				size_t hashIdx = 0, flagIdx = 0;
				UInt256 merkleRoot = MerkleBlockRootR(block, &hashIdx, &flagIdx, 0), t = UINT256_ZERO;
				int r = 1;

				// check if merkle root is correct
				if (block.getRaw()->totalTx > 0 && ! UInt256Eq(&(merkleRoot), &(block.getRaw()->merkleRoot))) r = 0;

				// check if timestamp is too far in future
				if (block.getRaw()->timestamp > currentTime + BLOCK_MAX_TIME_DRIFT) r = 0;

				//todo check pow later
				return r;

				// check if proof-of-work target is out of range
				if (target == 0 || target & 0x00800000 || size > maxsize || (size == maxsize && target > maxtarget)) r = 0;

				if (size > 3) UInt32SetLE(&t.u8[size - 3], target);
				else UInt32SetLE(t.u8, target >> (3 - size)*8);

				UInt256 auxBlockHash = block.getAuxPow().getParBlockHeaderHash();
				for (int i = sizeof(t) - 1; r && i >= 0; i--) { // check proof-of-work
					if (auxBlockHash.u8[i] < t.u8[i]) break;
					if (auxBlockHash.u8[i] > t.u8[i]) r = 0;
				}

				return r;
			}

		}

		int MerkleBlockMessage::Accept(BRPeer *peer, const uint8_t *msg, size_t msgLen) {

			BRPeerContext *ctx = (BRPeerContext *) peer;
			ELAPeerContext *elactx = (ELAPeerContext *) peer;
			// msg is holding by payload pointer create by malloc, do not match delete[] in ByteStream
			ByteStream stream(const_cast<uint8_t *>(msg), msgLen, false);
			MerkleBlock wrappedBlock;
			wrappedBlock.Deserialize(stream);

			BRMerkleBlock *block = BRMerkleBlockCopy(wrappedBlock.getRaw());
			block->blockHash = wrappedBlock.getBlockHash();
			int r = 1;

			std::vector<UInt256> blockTxHashes;
			if (!MerkleBlockIsValid(wrappedBlock, (uint32_t) time(nullptr))) {
				Log::getLogger()->warn("invalid merkleblock: {}",
									   Utils::UInt256ToString(block->blockHash));
				BRMerkleBlockFree(block);
				block = nullptr;
				r = 0;
			} else if (!ctx->sentFilter && !ctx->sentGetdata) {
				Log::getLogger()->warn("got merkleblock message before loading a filter");
				BRMerkleBlockFree(block);
				block = nullptr;
				r = 0;
			} else {
				size_t count = wrappedBlock.getTransactionCount();
				UInt256 _hashes[(sizeof(UInt256) * count <= 0x1000) ? count : 0],
						*hashes = (sizeof(UInt256) * count <= 0x1000) ? _hashes : (UInt256 *) malloc(
						count * sizeof(*hashes));

				assert(hashes != nullptr);
				count = BRMerkleBlockTxHashes(block, hashes, count);

				for (size_t i = count; i > 0; i--) { // reverse order for more efficient removal as tx arrive
					if (BRSetContains(ctx->knownTxHashSet, &hashes[i - 1])) continue;
					blockTxHashes.push_back(hashes[i - 1]);
				}

				ctx->manager->peerMessages->BRPeerSendGetdataMessage(peer, hashes, count, nullptr, 0);

				if (hashes != _hashes) free(hashes);
			}

			if (block) {
				if (!blockTxHashes.empty()) { // wait til we get all tx messages before processing the block
					for (std::vector<UInt256>::iterator it = blockTxHashes.begin(); it != blockTxHashes.end(); ++it) {
						elactx->txBlockMap[*it] = block;
					}
					elactx->blockTxListMap[block] = blockTxHashes;
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
