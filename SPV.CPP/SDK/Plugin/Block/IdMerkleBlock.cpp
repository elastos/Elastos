// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <cstring>
#include <stdlib.h>
#include <assert.h>

#include "Utils.h"
#include "IdMerkleBlock.h"

namespace Elastos {
	namespace ElaWallet {
		IdMerkleBlock *IdMerkleBlockNew() {
			return new IdMerkleBlock;
		}

		IdMerkleBlock *IdMerkleBlockCopy(IdMerkleBlock *orig) {
			assert(orig != nullptr);

			IdMerkleBlock *cpy = IdMerkleBlockNew();
			cpy->raw = orig->raw;
			cpy->idAuxPow = orig->idAuxPow;

			cpy->raw.hashes = nullptr;
			cpy->raw.flags = nullptr;
			BRMerkleBlockSetTxHashes(&cpy->raw, orig->raw.hashes, orig->raw.hashesCount, orig->raw.flags, orig->raw.flagsLen);

			return cpy;
		}

		void IdMerkleBlockFree(IdMerkleBlock *idBlock) {
			assert(idBlock != nullptr);

			BRMerkleBlock *block = (BRMerkleBlock *)idBlock;

			if (block->hashes) free(block->hashes);
			if (block->flags) free(block->flags);

			delete idBlock;
		}

		IdMerkleBlock &IdMerkleBlock::operator=(const IdMerkleBlock &orig) {
			this->raw = orig.raw;
			this->raw.hashes = nullptr;
			this->raw.flags = nullptr;
			BRMerkleBlockSetTxHashes(&this->raw, orig.raw.hashes, orig.raw.hashesCount, orig.raw.flags, orig.raw.flagsLen);

			this->idAuxPow = orig.idAuxPow;

			return *this;
		}

	}
}
