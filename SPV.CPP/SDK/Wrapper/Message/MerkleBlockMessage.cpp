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

		int MerkleBlockMessage::Accept(BRPeer *peer, const uint8_t *msg, size_t msgLen) {

			BRPeerContext *ctx = (BRPeerContext *) peer;
			ELAPeerContext *elactx = (ELAPeerContext *) peer;
			// msg is holding by payload pointer create by malloc, do not match delete[] in ByteStream
			ByteStream stream(const_cast<uint8_t *>(msg), msgLen, false);
			MerkleBlock wrappedBlock;
			wrappedBlock.Deserialize(stream);

			ELAMerkleBlock *elablock = ELAMerkleBlockCopy((ELAMerkleBlock *)wrappedBlock.getRaw());
			BRMerkleBlock *block = (BRMerkleBlock *)elablock;
			block->blockHash = wrappedBlock.getBlockHash();
			int r = 1;

			std::vector<UInt256> blockTxHashes;
			if (!wrappedBlock.isValid((uint32_t) time(nullptr))) {
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
