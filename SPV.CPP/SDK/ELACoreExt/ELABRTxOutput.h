// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef __ELASTOS_SDK_ELABRTXOUTPUT_H
#define __ELASTOS_SDK_ELABRTXOUTPUT_H

#include "BRTransaction.h"

namespace Elastos {
	namespace SDK {

		typedef struct {
			BRTxOutput raw;
			UInt256 assetId;
			uint32_t outputLock;
			UInt168 programHash;
		} ELABRTxOutput;

	}
}

#endif //__ELASTOS_SDK_ELABRTXOUTPUT_H
