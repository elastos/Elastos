// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <cstdlib>

#include "ELABRTransaction.h"

namespace Elastos {
	namespace SDK {
		// returns a newly allocated empty transaction that must be freed by calling ELABRTransactionFree()
		ELABRTransaction *ELABRTransactionNew(void) {
			ELABRTransaction *tx = (ELABRTransaction *) calloc(1, sizeof(*tx));
			tx->raw = *BRTransactionNew();
			tx->type = 0;
			tx->payloadVersion = 0;
			tx->programData.clear();
			tx->attributeData.clear();
			return tx;
		}

		// frees memory allocated for tx
		void ELABRTransactionFree(ELABRTransaction *tx) {
			free(tx);
		}
	}
}