// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef __ELASTOS_SDK_IDMERKLEBLOCK_H__
#define __ELASTOS_SDK_IDMERKLEBLOCK_H__

#include "BRMerkleBlock.h"
#include "IdAuxPow.h"

namespace Elastos {
	namespace ElaWallet {

		struct IdMerkleBlock {
			IdMerkleBlock() {
				memset(&this->raw, 0, sizeof(this->raw));
				this->raw.height = BLOCK_UNKNOWN_HEIGHT;
			}

			BRMerkleBlock raw;
			IdAuxPow idAuxPow;

			IdMerkleBlock &operator=(const IdMerkleBlock &idMerkleBlock);
		};

		IdMerkleBlock *IdMerkleBlockNew();

		IdMerkleBlock *IdMerkleBlockCopy(IdMerkleBlock *block);

		void IdMerkleBlockFree(IdMerkleBlock *block);
	}
}

#endif //__ELASTOS_SDK_IDMERKLEBLOCK_H__
