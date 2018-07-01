// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <cstdlib>
#include <cstring>

#include <BRTransaction.h>
#include <Core/BRTransaction.h>
#include <SDK/ELACoreExt/Payload/PayloadCoinBase.h>
#include <SDK/ELACoreExt/Payload/PayloadRegisterAsset.h>
#include <SDK/ELACoreExt/Payload/PayloadTransferAsset.h>
#include <SDK/ELACoreExt/Payload/PayloadRecord.h>
#include <SDK/ELACoreExt/Payload/PayloadSideMining.h>
#include <SDK/ELACoreExt/Payload/PayloadIssueToken.h>
#include <SDK/ELACoreExt/Payload/PayloadWithDrawAsset.h>
#include <SDK/ELACoreExt/Payload/PayloadTransferCrossChainAsset.h>
#include <SDK/ELACoreExt/Payload/PayloadRegisterIdentification.h>

#include "BRArray.h"
#include "ELATransaction.h"
#include "ELATxOutput.h"
#include "Utils.h"

namespace Elastos {
	namespace ElaWallet {

		IPayload *ELAPayloadNew(ELATransaction::Type type) {
			if (type == ELATransaction::CoinBase) {
				return new PayloadCoinBase();
			} else if (type == ELATransaction::RegisterAsset) {
				return new PayloadRegisterAsset();
			} else if (type == ELATransaction::TransferAsset) {
				return new PayloadTransferAsset();
			} else if (type == ELATransaction::Record) {
				return new PayloadRecord();
			} else if (type == ELATransaction::Deploy) {
				//todo add deploy payload
				//_transaction->payload = boost::shared_ptr<PayloadDeploy>(new PayloadDeploy());
			} else if (type == ELATransaction::SideMining) {
				return new PayloadSideMining();
			} else if (type == ELATransaction::IssueToken) {
				return new PayloadIssueToken();
			} else if (type == ELATransaction::WithdrawAsset) {
				return new PayloadWithDrawAsset();
			} else if (type == ELATransaction::TransferCrossChainAsset) {
				return new PayloadTransferCrossChainAsset();
			} else if (type == ELATransaction::RegisterIdentification) {
				return new PayloadRegisterIdentification();
			}

			return nullptr;
		}

		// returns a newly allocated empty transaction that must be freed by calling ELATransactionFree()
		ELATransaction *ELATransactionNew(void) {
			return new ELATransaction();
		}


		ELATransaction *ELATransactionCopy(const ELATransaction *orig) {
			ELATransaction *tx = new ELATransaction();

			BRTxInput *inputs = tx->raw.inputs;
			BRTxOutput *outputs = tx->raw.outputs;

			assert(orig != NULL);
			tx->raw = orig->raw;
			tx->raw.inputs =  inputs;
			tx->raw.outputs = outputs;
			tx->raw.inCount = tx->raw.outCount = 0;

			tx->type = orig->type;
			tx->payloadVersion = orig->payloadVersion;
			tx->fee = orig->fee;

			if (orig->payload) {
				tx->payload = ELAPayloadNew(orig->type);

				ByteStream stream;
				orig->payload->Serialize(stream);
				stream.setPosition(0);
				tx->payload->Deserialize(stream);
			}

			for (size_t i = 0; i < orig->raw.inCount; ++i) {
				BRTransactionAddInput(&tx->raw, orig->raw.inputs[i].txHash, orig->raw.inputs[i].index,
									  orig->raw.inputs[i].amount, orig->raw.inputs[i].script, orig->raw.inputs[i].scriptLen,
									  orig->raw.inputs[i].signature, orig->raw.inputs[i].sigLen, orig->raw.inputs[i].sequence);
			}

			for (size_t i = 0; i < orig->outputs.size(); ++i) {
				tx->outputs.push_back(new TransactionOutput(*orig->outputs[i]));
			}

			for (size_t i = 0; i < orig->attributes.size(); ++i) {
				tx->attributes.push_back(new Attribute(*orig->attributes[i]));
			}

			for (size_t i = 0; i < orig->programs.size(); ++i) {
				tx->programs.push_back(new Program(*orig->programs[i]));
			}

			tx->Remark = orig->Remark;
			return tx;
		}

		void ELATransactionFree(ELATransaction *tx) {
			if (tx->raw.inputs) {
				for (size_t i = 0; i < tx->raw.inCount; i++) {
					BRTxInputSetScript(&tx->raw.inputs[i], nullptr, 0);
					BRTxInputSetSignature(&tx->raw.inputs[i], nullptr, 0);
				}
				array_free(tx->raw.inputs);
			}

			if (tx->raw.outputs) {
				for (size_t i = 0; i < tx->raw.outCount; i++) {
					ELATxOutputSetScript((ELATxOutput *)&tx->raw.outputs[i], nullptr, 0);
				}
				array_free(tx->raw.outputs);
			}

			for (size_t i = 0; i < tx->outputs.size(); ++i)
				delete tx->outputs[i];

			for (size_t i = 0; i < tx->attributes.size(); ++i)
				delete tx->attributes[i];

			for (size_t i = 0; i < tx->programs.size(); ++i)
				delete tx->programs[i];

			delete tx->payload;
			delete tx;
		}

		// shuffles order of tx outputs
		void ELATransactionShuffleOutputs(ELATransaction *tx) {
			assert(tx != NULL);

			for (uint32_t i = 0; tx && i + 1 < tx->outputs.size(); i++) { // fischer-yates shuffle
				uint32_t j = i + BRRand((uint32_t)tx->outputs.size() - i);
				TransactionOutput *t;

				if (j != i) {
					t = tx->outputs[i];
					tx->outputs[i] = tx->outputs[j];
					tx->outputs[j] = t;
				}
			}
		}

		// size in bytes if signed, or estimated size assuming compact pubkey sigs
		size_t ELATransactionSize(const ELATransaction *tx) {
			BRTxInput *input;
			size_t size;

			ELATransaction *txn = (ELATransaction *)tx;

			assert(txn != NULL);
			size = (txn) ? 8 + BRVarIntSize(txn->raw.inCount) + BRVarIntSize(txn->outputs.size()) : 0;

			for (size_t i = 0; txn && i < txn->raw.inCount; i++) {
				input = &txn->raw.inputs[i];

				if (input->signature) {
					size += sizeof(UInt256) + sizeof(uint32_t) + BRVarIntSize(input->sigLen) + input->sigLen + sizeof(uint32_t);
				}
				else size += TX_INPUT_SIZE;
			}

			for (size_t i = 0; txn && i < txn->outputs.size(); i++) {
				size += sizeof(uint64_t) + BRVarIntSize(txn->outputs[i]->getRaw()->scriptLen) + txn->outputs[i]->getRaw()->scriptLen;
			}

			return size;
		}

		// minimum transaction fee needed for tx to relay across the bitcoin network
		uint64_t ELATransactionStandardFee(const ELATransaction *tx)
		{
			assert(tx != NULL);
			return tx->fee ? tx->fee : ((ELATransactionSize(tx) + 999)/1000)*TX_FEE_PER_KB;
		}

		bool ELATransactionIsSign(const ELATransaction *tx) {
			size_t len = tx->programs.size();
			if (len <= 0) {
				return false;
			}
			for (size_t i = 0; i < len; ++i) {
				if (!tx->programs[i]->isValid()) {
					return false;
				}
			}
			return true;
		}

	}

}