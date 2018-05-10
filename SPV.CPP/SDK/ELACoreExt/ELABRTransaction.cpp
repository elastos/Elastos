// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <cstdlib>
#include <cstring>
#include <BRTransaction.h>

#include "BRArray.h"
#include "ELABRTransaction.h"
#include "ELABRTxOutput.h"

namespace Elastos {
	namespace SDK {
		// returns a newly allocated empty transaction that must be freed by calling ELABRTransactionFree()
		ELABRTransaction *ELABRTransactionNew(void) {
			ELABRTransaction *tx = (ELABRTransaction *) calloc(1, sizeof(ELABRTransaction));
			memset(tx, 0, sizeof(ELABRTransaction));
			array_new(tx->raw.inputs, 1);
			array_new(tx->raw.outputs, 2);
			BRTransaction *temp = BRTransactionNew();
			tx->raw.version = temp->version;
			tx->raw.lockTime = temp->lockTime;
			tx->raw.blockHeight = TX_UNCONFIRMED;
			tx->type = 0;
			tx->payloadVersion = 0;
			tx->programData.clear();
			tx->attributeData.clear();
			tx->outputAssetIDList.clear();
			tx->outputLockList.clear();
			tx->outputProgramHashList.clear();
			BRTransactionFree(temp);
			return tx;
		}

		ELABRTransaction *ELABRTransactioCopy(const ELABRTransaction *tx) {
			ELABRTransaction *elabrTransaction = ELABRTransactionNew();
			elabrTransaction->raw.timestamp = tx->raw.timestamp;
			elabrTransaction->raw.version = tx->raw.version;
			UInt256Set(&elabrTransaction->raw.txHash, tx->raw.txHash);
			elabrTransaction->raw.blockHeight = tx->raw.blockHeight;
			elabrTransaction->raw.inCount = tx->raw.inCount;
			elabrTransaction->raw.lockTime = tx->raw.lockTime;
			elabrTransaction->raw.outCount = tx->raw.outCount;

			for (size_t i = 0; i < tx->raw.inCount; i++) {
				BRTransactionAddInput(&elabrTransaction->raw, tx->raw.inputs[i].txHash, tx->raw.inputs[i].index, tx->raw.inputs[i].amount,
									  tx->raw.inputs[i].script, tx->raw.inputs[i].scriptLen,
									  tx->raw.inputs[i].signature, tx->raw.inputs[i].sigLen, tx->raw.inputs[i].sequence);
			}

			for (size_t i = 0; i < tx->raw.outCount; i++) {
				BRTransactionAddOutput(&elabrTransaction->raw, tx->raw.outputs[i].amount, tx->raw.outputs[i].script, tx->raw.outputs[i].scriptLen);
			}


			elabrTransaction->type = tx->type;
			elabrTransaction->payloadVersion = tx->payloadVersion;
			for (size_t i = 0; i < tx->programData.size(); ++i) {
				ByteData byteData(new uint8_t[tx->programData[i].length], tx->programData[i].length);
				memcpy(byteData.data, tx->programData[i].data, tx->programData[i].length);
				elabrTransaction->programData.push_back(byteData);
			}
			for (size_t i = 0; i < tx->attributeData.size(); ++i) {
				ByteData byteData(new uint8_t[tx->attributeData[i].length], tx->attributeData[i].length);
				memcpy(byteData.data, tx->attributeData[i].data, tx->attributeData[i].length);
				elabrTransaction->attributeData.push_back(byteData);
			}

			elabrTransaction->outputAssetIDList.reserve(tx->outputAssetIDList.size());
			for (size_t i = 0; i < tx->outputAssetIDList.size(); ++i) {
				UInt256Set(&elabrTransaction->outputAssetIDList[i], tx->outputAssetIDList[i]);
			}

			elabrTransaction->outputLockList.reserve(tx->outputLockList.size());
			for (size_t i = 0; i < tx->outputLockList.size(); ++i) {
				elabrTransaction->outputLockList[i] = tx->outputLockList[i];
			}

			elabrTransaction->outputProgramHashList.reserve(tx->outputProgramHashList.size());
			for (size_t i = 0; i < tx->outputProgramHashList.size(); ++i) {
				UInt168Set(&elabrTransaction->outputProgramHashList[i], tx->outputProgramHashList[i]);
			}
			return elabrTransaction;
		}

		// frees memory allocated for tx
		void ELABRTransactionFree(ELABRTransaction *tx) {
			array_free(tx->raw.outputs);
			array_free(tx->raw.inputs);

			free(tx);
		}
	}
}