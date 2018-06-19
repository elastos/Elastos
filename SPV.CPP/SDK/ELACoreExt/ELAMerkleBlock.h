// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef __ELASTOS_SDK_ELAMERKLEBLOCK_H__
#define __ELASTOS_SDK_ELAMERKLEBLOCK_H__

#include "BRMerkleBlock.h"
#include "AuxPow.h"

namespace Elastos {
	namespace ElaWallet {

		struct ELAMerkleBlock {
			BRMerkleBlock raw;
			AuxPow auxPow;
		};

		ELAMerkleBlock *ELAMerkleBlockNew();

		ELAMerkleBlock *ELAMerkleBlockCopy(ELAMerkleBlock *elaproto);

		void ELAMerkleBlockFree(ELAMerkleBlock *block);
	}
}

#endif //__ELASTOS_SDK_ELAMERKLEBLOCK_H__
