// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "ELAMerkleBlock.h"

#include <Common/Utils.h>

#include <cstring>
#include <cstdlib>
#include <cassert>

namespace Elastos {
	namespace ElaWallet {

		ELAMerkleBlock *ELAMerkleBlockNew() {
			return new ELAMerkleBlock;
		}

		ELAMerkleBlock *ELAMerkleBlockCopy(const ELAMerkleBlock *orig) {
			assert(orig != nullptr);

			ELAMerkleBlock *cpy = ELAMerkleBlockNew();
			cpy->raw = orig->raw;
			cpy->auxPow = orig->auxPow;

			BRMerkleBlock *proto = (BRMerkleBlock *)orig;
			cpy->raw.hashes = nullptr;
			cpy->raw.flags = nullptr;
			BRMerkleBlockSetTxHashes(&cpy->raw, proto->hashes, proto->hashesCount, proto->flags, proto->flagsLen);

			return cpy;
		}

		void ELAMerkleBlockFree(ELAMerkleBlock *elablock) {
			assert(elablock != nullptr);

			BRMerkleBlock *block = (BRMerkleBlock *)elablock;

 			if (block->hashes) free(block->hashes);
			if (block->flags) free(block->flags);

			delete elablock;
			elablock = nullptr;
		}

		ELAMerkleBlock &ELAMerkleBlock::operator=(const ELAMerkleBlock &orig) {
			this->raw = orig.raw;
			this->raw.hashes = nullptr;
			this->raw.flags = nullptr;
			BRMerkleBlockSetTxHashes(&this->raw, orig.raw.hashes, orig.raw.hashesCount, orig.raw.flags, orig.raw.flagsLen);

			this->auxPow = orig.auxPow;

			return *this;
		}

	}
}
