// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef __ELASTOS_SDK_IDMERKLEBLOCK_H__
#define __ELASTOS_SDK_IDMERKLEBLOCK_H__

#include <Core/BRMerkleBlock.h>
#include <SDK/Plugin/Block/IDAuxPow.h>

namespace Elastos {
	namespace ElaWallet {

		struct IDMerkleBlock {
			IDMerkleBlock() {
				memset(&this->raw, 0, sizeof(this->raw));
				this->raw.height = BLOCK_UNKNOWN_HEIGHT;
			}

			BRMerkleBlock raw;
			IDAuxPow idAuxPow;

			IDMerkleBlock &operator=(const IDMerkleBlock &idMerkleBlock);
		};

		IDMerkleBlock *IDMerkleBlockNew();

		IDMerkleBlock *IDMerkleBlockCopy(const IDMerkleBlock *block);

		void IDMerkleBlockFree(IDMerkleBlock *block);

	}
}

#endif //__ELASTOS_SDK_IDMERKLEBLOCK_H__
