// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <cstdlib>
#include <cstring>

#include <BRTransaction.h>

#include "BRArray.h"
#include "ELABRTransaction.h"
#include "Utils.h"
#include "BRAddress.h"

namespace Elastos {
	namespace SDK {
		ELABRTransaction &ELABRTransaction::operator = (const _ELABRTransaction &ebt) {
			if (0 < raw.inCount && raw.inputs) {
				for (size_t i = 0; i < raw.inCount; i++) {
					BRTxInputSetScript(&raw.inputs[i], NULL, 0);
					BRTxInputSetSignature(&raw.inputs[i], NULL, 0);
				}
				raw.inCount = 0;
				raw.inputs = NULL;
				array_free(raw.inputs);
				array_new(raw.inputs, 1);
			}

			if (0 < raw.outCount) {
				for (size_t i = 0; i < raw.outCount; i++) {
					if (raw.outputs) {
						BRTxOutputSetScript(&raw.outputs[i], NULL, 0);
					}
					if (outputs) {
						BRTxOutputSetScript((BRTxOutput *)&outputs[i], NULL, 0);
					}
				}
				raw.outCount = 0;
				raw.outputs = NULL;
				if (raw.outputs) {
					array_free(raw.outputs);
				}
				if (outputs) {
					array_free(outputs);
				}
				array_new(raw.outputs, 2);
				array_new(outputs, 2);
			}

			raw = ebt.raw;
			for (size_t i = 0; i < ebt.raw.inCount; i++) {
				BRTransactionAddInput(&raw, ebt.raw.inputs[i].txHash, ebt.raw.inputs[i].index, ebt.raw.inputs[i].amount,
									  ebt.raw.inputs[i].script, ebt.raw.inputs[i].scriptLen,
									  ebt.raw.inputs[i].signature, ebt.raw.inputs[i].sigLen, ebt.raw.inputs[i].sequence);


			}

			for (size_t i = 0; i < ebt.raw.outCount; i++) {
				ELABRTransactionAddOutput(this, ebt.outputs[i].raw.amount, ebt.outputs[i].raw.script,
				                          ebt.outputs[i].raw.scriptLen);
				UInt256Set(&outputs[i].assetId, ebt.outputs[i].assetId);
				outputs[i].outputLock = ebt.outputs[i].outputLock;
				UInt168Set(&outputs[i].programHash, ebt.outputs[i].programHash);
			}
			type = ebt.type;
			payloadVersion = ebt.payloadVersion;

			payloadData.Resize(ebt.payloadData.GetSize());
			memcpy(payloadData, ebt.payloadData, ebt.payloadData.GetSize());

			attributeData.clear();
			programData.clear();
			return *this;
		}

		// returns a newly allocated empty transaction that must be freed by calling ELABRTransactionFree()
		ELABRTransaction *ELABRTransactionNew(void) {
			ELABRTransaction *tx = (ELABRTransaction *) calloc(1, sizeof(ELABRTransaction));
			memset(tx, 0, sizeof(ELABRTransaction));
			array_new(tx->raw.inputs, 1);
			array_new(tx->raw.outputs, 2);
			array_new(tx->outputs, 2);
			for (int i = 0; i < 2; i++) {
				tx->outputs[i] = ELABR_TX_OUTPUT_NONE;
			}
			BRTransaction *temp = BRTransactionNew();
			tx->raw.version = temp->version;
			tx->raw.lockTime = temp->lockTime;
			tx->raw.blockHeight = TX_UNCONFIRMED;
			tx->type = 0;
			tx->payloadVersion = 0;
			tx->payloadData = CMBlock();
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
				ELABRTransactionAddOutput(elabrTransaction, tx->outputs[i].raw.amount, tx->outputs[i].raw.script,
											tx->outputs[i].raw.scriptLen);
				UInt256Set(&elabrTransaction->outputs[i].assetId, tx->outputs[i].assetId);
				elabrTransaction->outputs[i].outputLock = tx->outputs[i].outputLock;
				UInt168Set(&elabrTransaction->outputs[i].programHash, tx->outputs[i].programHash);
			}


			elabrTransaction->type = tx->type;
			elabrTransaction->payloadVersion = tx->payloadVersion;

			elabrTransaction->payloadData.Resize(tx->payloadData.GetSize());
			memcpy(elabrTransaction->payloadData, tx->payloadData, tx->payloadData.GetSize());

			for (size_t i = 0; i < tx->programData.size(); ++i) {
				CMBlock byteData(tx->programData[i].GetSize());
				memcpy(byteData, tx->programData[i], tx->programData[i].GetSize());
				elabrTransaction->programData.push_back(byteData);
			}
			for (size_t i = 0; i < tx->attributeData.size(); ++i) {
				CMBlock byteData(tx->attributeData[i].GetSize());
				memcpy(byteData, tx->attributeData[i], tx->attributeData[i].GetSize());
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

		void ELABRTransactionAddOutput(ELABRTransaction *tx, uint64_t amount, const uint8_t *script,
		                               size_t scriptLen) {
			ELABRTxOutput output = ELABR_TX_OUTPUT_NONE;
			output.raw.amount = amount;
			assert(tx != NULL);
			assert(script != NULL || scriptLen == 0);

			if (tx) {
				BRTxOutputSetScript(&(output.raw), script, scriptLen);
				output.programHash = Utils::AddressToUInt168(output.raw.address);
				output.assetId = Utils::getSystemAssetId();
				array_add(tx->outputs, output);
				ELABRTxOutput temp = ELABR_TX_OUTPUT_NONE;
				temp.assetId = output.assetId;
				memcpy(temp.raw.address, output.raw.address, sizeof(output.raw.address));
				temp.raw.amount = output.raw.amount;
				temp.raw.scriptLen = output.raw.scriptLen;
				BRTxOutputSetScript(&(temp.raw), script, scriptLen);
				temp.outputLock = output.outputLock;
				temp.programHash = output.programHash;
				array_add(tx->raw.outputs, *(BRTxOutput *)&temp);
				tx->raw.outCount = array_count(tx->outputs);
			}
		}

		// frees memory allocated for tx
		void ELABRTransactionFree(ELABRTransaction *tx) {
			if (0 < tx->raw.inCount && tx->raw.inputs) {
				for (size_t i = 0; i < tx->raw.inCount; i++) {
					BRTxInputSetScript(&tx->raw.inputs[i], NULL, 0);
					BRTxInputSetSignature(&tx->raw.inputs[i], NULL, 0);
				}
				array_free(tx->raw.inputs);
			}

			if (0 < tx->raw.outCount) {
				for (size_t i = 0; i < tx->raw.outCount; i++) {
					if (tx->raw.outputs) {
						BRTxOutputSetScript(&tx->raw.outputs[i], NULL, 0);
					}

					BRTxOutputSetScript((BRTxOutput *)&tx->outputs[i], NULL, 0);
				}
				if (tx->raw.outputs) {
					array_free(tx->raw.outputs);
				}
				array_free(tx->outputs);
			}

			tx->payloadData.Clear();

			tx->attributeData.clear();

			tx->programData.clear();

			free(tx);
		}

		// size in bytes if signed, or estimated size assuming compact pubkey sigs
		size_t ELABRTransactionSize(const ELABRTransaction *tx)
		{
			BRTxInput *input;
			size_t size;

			assert(tx != NULL);
			size = (tx) ? 8 + BRVarIntSize(tx->raw.inCount) + BRVarIntSize(tx->raw.outCount) : 0;

			for (size_t i = 0; tx && i < tx->raw.inCount; i++) {
				input = &tx->raw.inputs[i];

				if (input->signature) {
					size += sizeof(UInt256) + sizeof(uint32_t) + BRVarIntSize(input->sigLen) + input->sigLen + sizeof(uint32_t);
				}
				else size += TX_INPUT_SIZE;
			}

			for (size_t i = 0; tx && i < tx->raw.outCount; i++) {
				size += sizeof(uint64_t) + BRVarIntSize(tx->outputs[i].raw.scriptLen) + tx->outputs[i].raw.scriptLen;
			}

			return size;
		}
	}

}