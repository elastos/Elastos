// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef __ELASTOS_SDK_ELATxOutput_H
#define __ELASTOS_SDK_ELATxOutput_H

#include "BRTransaction.h"

namespace Elastos {
	namespace SDK {

		struct ELATxOutput {

			ELATxOutput() {
				raw = BR_TX_OUTPUT_NONE;
				assetId = UINT256_ZERO;
				outputLock = 0;
				programHash = UINT168_ZERO;
			}

			ELATxOutput(const ELATxOutput *output) {
				raw = output->raw;
				assetId = output->assetId;
				outputLock = output->outputLock;
				programHash = output->programHash;

				raw.script = nullptr;
				BRTxOutputSetScript(&raw, output->raw.script, output->raw.scriptLen);
			}

			BRTxOutput raw;
			UInt256 assetId;
			uint32_t outputLock;
			UInt168 programHash;
		};

		ELATxOutput *ELATxOutputNew();
		ELATxOutput *ELATxOutputCopy(const ELATxOutput *output);
		void ELATxOutputFree(ELATxOutput *output);

	}
}

#endif //__ELASTOS_SDK_ELATxOutput_H
