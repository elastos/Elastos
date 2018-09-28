// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <boost/scoped_ptr.hpp>
#include <Core/BRMerkleBlock.h>

#include "BRPeerManager.h"
#include "BRPeerMessages.h"
#include "BRArray.h"
#include "BRMerkleBlock.h"

#include "Peer.h"
#include "MerkleBlockMessage.h"
#include "Log.h"
#include "Utils.h"
#include "AuxPow.h"
#include "ELACoreExt/ELAPeerManager.h"
#include "ELACoreExt/ELAMerkleBlock.h"
#include "Plugin/Registry.h"
#include "SDK/Plugin/Block/MerkleBlock.h"

namespace Elastos {
	namespace ElaWallet {

		int MerkleBlockMessage::Accept(BRPeer *peer, const uint8_t *msg, size_t msgLen) {
			BRPeerContext *ctx = (BRPeerContext *) peer;
			// msg is holding by payload pointer create by malloc, do not match delete[] in ByteStream
			ByteStream stream(const_cast<uint8_t *>(msg), msgLen, false);

			ELAPeerManager *elaPeerManager = (ELAPeerManager *)ctx->manager;

			MerkleBlockPtr block(Registry::Instance()->CreateMerkleBlock(elaPeerManager->Plugins.BlockType));
			//fixme [refactor] replace raw block with Merkle block
//			assert(block != nullptr);
//			if (!block->Deserialize(stream)) {
//				block->deleteRawBlock();
//				peer_dbg(peer, "merkle block orignal data: %s", Utils::encodeHex(msg, msgLen).c_str());
//				peer_log(peer, "error: %s merkle block deserialize fail", elaPeerManager->Plugins.BlockType.c_str());
//				return 0;
//			}
//
//			BRMerkleBlock *blockRaw = block->getRawBlock();
//			int r = 1;
//
//			if (!block->isValid((uint32_t) time(nullptr))) {
//				peer_log(peer, "error: invalid merkleblock: %s", Utils::UInt256ToString(block->getHash()).c_str());
//				block->deleteRawBlock();
//				blockRaw = nullptr;
//				r = 0;
//			} else if (!ctx->sentFilter && !ctx->sentGetdata) {
//				peer_log(peer, "error: got merkleblock message before loading a filter");
//				block->deleteRawBlock();
//				blockRaw = nullptr;
//				r = 0;
//			} else {
//				size_t count = BRMerkleBlockTxHashes(block->getRawBlock(), NULL, 0);
//				UInt256 _hashes[(sizeof(UInt256) * count <= 0x1000) ? count : 0],
//						*hashes = (sizeof(UInt256) * count <= 0x1000) ? _hashes : (UInt256 *) malloc(
//						count * sizeof(*hashes));
//				assert(hashes != nullptr);
//				count = BRMerkleBlockTxHashes(block->getRawBlock(), hashes, count);
//				for (size_t i = count; i > 0; i--) { // reverse order for more efficient removal as tx arrive
//					if (BRSetContains(ctx->knownTxHashSet, &hashes[i - 1])) continue;
//					array_add(ctx->currentBlockTxHashes, hashes[i - 1]);
//				}
//
//				if (hashes != _hashes) free(hashes);
//			}
//
//			if (blockRaw) {
//				if (array_count(ctx->currentBlockTxHashes) > 0) { // wait til we get all tx messages before processing the block
//					ctx->currentBlock = blockRaw;
//				} else if (ctx->relayedBlock) {
//					ctx->relayedBlock(ctx->info, blockRaw);
//				} else {
//					block->deleteRawBlock();
//				}
//			}
//			return r;
			return 0;
		}

		void MerkleBlockMessage::Send(BRPeer *peer, void *serializable) {
		}
	}
}
