// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <cstring>
#include <assert.h>

#include "ELAMerkleBlock.h"

namespace Elastos {
	namespace SDK {

		ELAMerkleBlock *ELAMerkleBlockNew() {
			ELAMerkleBlock *elablock = new ELAMerkleBlock;

			BRMerkleBlock *block = (BRMerkleBlock *)elablock;

			memset(elablock, 0, sizeof(*block));
			block->height = BLOCK_UNKNOWN_HEIGHT;

			return elablock;
		}

		ELAMerkleBlock *ELAMerkleBlockCopy(ELAMerkleBlock *elaproto) {
			assert(elaproto != nullptr);

			ELAMerkleBlock *elacpy = ELAMerkleBlockNew();
			*elacpy = *elaproto;

			elacpy->auxPow.setBTCTransaction(BRTransactionCopy(elaproto->auxPow.getBTCTransaction()));
			elacpy->auxPow.setParBlockHeader(BRMerkleBlockCopy(elacpy->auxPow.getParBlockHeader()));

			BRMerkleBlock *proto = (BRMerkleBlock *)elaproto;
			BRMerkleBlock *cpy = (BRMerkleBlock *)elacpy;
			cpy->hashes = nullptr;
			cpy->flags = nullptr;
			BRMerkleBlockSetTxHashes(cpy, proto->hashes, proto->hashesCount, proto->flags, proto->flagsLen);

			return elacpy;
		}

		void ELAMerkleBlockFree(ELAMerkleBlock *elablock) {
			assert(elablock != nullptr);

			BRMerkleBlock *block = (BRMerkleBlock *)elablock;

			if (block->hashes) free(block->hashes);
			if (block->flags) free(block->flags);

			delete elablock;
		}

	}
}