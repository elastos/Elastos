// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef __ELASTOS_SDK_ELATxOutput_H
#define __ELASTOS_SDK_ELATxOutput_H

#include <Core/BRTransaction.h>
#include <SDK/Common/Utils.h>
#include "BRAddress.h"
#include "BRTransaction.h"

namespace Elastos {
	namespace ElaWallet {
		struct ELATxOutput;

		ELATxOutput *ELATxOutputNew();
		ELATxOutput *ELATxOutputCopy(const ELATxOutput *output);
		void ELATxOutputFree(ELATxOutput *output);
		void ELATxOutputSetScript(ELATxOutput *output, const uint8_t *script, size_t scriptLen,
		                          int signType = ELA_STANDARD);

		struct ELATxOutput {

			ELATxOutput() {
				raw = BR_TX_OUTPUT_NONE;
				assetId = UINT256_ZERO;
				outputLock = 0;
				programHash = UINT168_ZERO;
				signType = ELA_STANDARD;
			}

			BRTxOutput raw;
			UInt256 assetId;
			uint32_t outputLock;
			UInt168 programHash;
			int signType;
		};

	}
}

#endif //__ELASTOS_SDK_ELATxOutput_H
