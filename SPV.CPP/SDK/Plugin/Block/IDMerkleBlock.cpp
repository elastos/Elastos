// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "IDMerkleBlock.h"

#include <SDK/Common/Utils.h>

#include <cstring>
#include <cstdlib>
#include <cassert>

namespace Elastos {
	namespace ElaWallet {
		IDMerkleBlock *IDMerkleBlockNew() {
			return new IDMerkleBlock;
		}

		IDMerkleBlock *IDMerkleBlockCopy(const IDMerkleBlock *orig) {
			assert(orig != nullptr);

			IDMerkleBlock *cpy = IDMerkleBlockNew();
			cpy->raw = orig->raw;
			cpy->idAuxPow = orig->idAuxPow;

			cpy->raw.hashes = nullptr;
			cpy->raw.flags = nullptr;
			BRMerkleBlockSetTxHashes(&cpy->raw, orig->raw.hashes, orig->raw.hashesCount, orig->raw.flags, orig->raw.flagsLen);

			return cpy;
		}

		void IDMerkleBlockFree(IDMerkleBlock *idBlock) {
			assert(idBlock != nullptr);

			BRMerkleBlock *block = (BRMerkleBlock *)idBlock;

			if (block->hashes) free(block->hashes);
			if (block->flags) free(block->flags);

			delete idBlock;
		}

		IDMerkleBlock &IDMerkleBlock::operator=(const IDMerkleBlock &orig) {
			this->raw = orig.raw;
			this->raw.hashes = nullptr;
			this->raw.flags = nullptr;
			BRMerkleBlockSetTxHashes(&this->raw, orig.raw.hashes, orig.raw.hashesCount, orig.raw.flags, orig.raw.flagsLen);

			this->idAuxPow = orig.idAuxPow;

			return *this;
		}

	}
}
