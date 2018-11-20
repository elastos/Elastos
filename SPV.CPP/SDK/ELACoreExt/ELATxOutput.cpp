// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "ELATxOutput.h"
#include "Address.h"

namespace Elastos {
	namespace ElaWallet {

		ELATxOutput *ELATxOutputNew() {
			ELATxOutput *output = new ELATxOutput();
			return output;
		}

		void ELATxOutputFree(ELATxOutput *output) {
			ELATxOutputSetScript(output, nullptr, 0);
			delete output;
		}

		void ELATxOutputSetScript(ELATxOutput *elaoutput, const uint8_t *script, size_t scriptLen, int signType) {
			BRTxOutput *output = (BRTxOutput *)elaoutput;
			assert(output != NULL);
			if (output->script) array_free(output->script);
			output->script = NULL;
			output->scriptLen = 0;
			elaoutput->signType = signType;

			if (script) {
				output->scriptLen = scriptLen;
				array_new(output->script, scriptLen);
				array_add_array(output->script, script, scriptLen);
			}
		}

	}
}
