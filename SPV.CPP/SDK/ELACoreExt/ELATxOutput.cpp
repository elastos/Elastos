// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "ELATxOutput.h"

namespace Elastos {
	namespace SDK {

		ELATxOutput *ELATxOutputNew() {
			ELATxOutput *output = new ELATxOutput();
			return output;
		}

		ELATxOutput *ELATxOutputCopy(const ELATxOutput *output) {
			ELATxOutput *copy = new ELATxOutput(output);
			return copy;
		}

		void ELATxOutputFree(ELATxOutput *output) {
			BRTxOutputSetScript(&output->raw, nullptr, 0);
			delete output;
		}

	}
}
