// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef __ELASTOS_SDK_ELAMERKLEBLOCK_H__
#define __ELASTOS_SDK_ELAMERKLEBLOCK_H__

#include "AuxPow.h"

#include <bitcoin/BRMerkleBlock.h>

namespace Elastos {
	namespace ElaWallet {

		struct ELAMerkleBlock {
			ELAMerkleBlock() {
				memset(&this->raw, 0, sizeof(this->raw));
				this->raw.height = BLOCK_UNKNOWN_HEIGHT;
			}

			BRMerkleBlock raw;
			AuxPow auxPow;

			ELAMerkleBlock &operator=(const ELAMerkleBlock &elaMerkleBlock);
		};

		ELAMerkleBlock *ELAMerkleBlockNew();

		ELAMerkleBlock *ELAMerkleBlockCopy(const ELAMerkleBlock *elaproto);

		void ELAMerkleBlockFree(ELAMerkleBlock *block);

	}
}

#endif //__ELASTOS_SDK_ELAMERKLEBLOCK_H__
