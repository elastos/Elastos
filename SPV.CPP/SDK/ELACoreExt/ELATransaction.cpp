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
#include "Utils.h"
#include "BRAddress.h"

namespace Elastos {
	namespace SDK {

		PayloadPtr ELAPayloadNew(ELATransaction::Type type) {
			if (type == ELATransaction::CoinBase) {
				return boost::shared_ptr<PayloadCoinBase>(new PayloadCoinBase());
			} else if (type == ELATransaction::RegisterAsset) {
				return boost::shared_ptr<PayloadRegisterAsset>(new PayloadRegisterAsset());
			} else if (type == ELATransaction::TransferAsset) {
				return boost::shared_ptr<PayloadTransferAsset>(new PayloadTransferAsset());
			} else if (type == ELATransaction::Record) {
				return boost::shared_ptr<PayloadRecord>(new PayloadRecord());
			} else if (type == ELATransaction::Deploy) {
				//todo add deploy payload
				//_transaction->payload = boost::shared_ptr<PayloadDeploy>(new PayloadDeploy());
			} else if (type == ELATransaction::SideMining) {
				return boost::shared_ptr<PayloadSideMining>(new PayloadSideMining());
			} else if (type == ELATransaction::IssueToken) {
				return boost::shared_ptr<PayloadIssueToken>(new PayloadIssueToken());
			} else if (type == ELATransaction::WithdrawAsset) {
				return boost::shared_ptr<PayloadWithDrawAsset>(new PayloadWithDrawAsset());
			} else if (type == ELATransaction::TransferCrossChainAsset) {
				return boost::shared_ptr<PayloadTransferCrossChainAsset>(new PayloadTransferCrossChainAsset());
			} else if (type == ELATransaction::RegisterIdentification) {
				return boost::shared_ptr<PayloadRegisterIdentification>(new PayloadRegisterIdentification());
			}

			return nullptr;
		}

		// returns a newly allocated empty transaction that must be freed by calling ELATransactionFree()
		ELATransaction *ELATransactionNew(void) {
			ELATransaction *tx = new ELATransaction();
			BRTransaction *txRaw = BRTransactionNew();
			tx->raw = *txRaw;
			BRTransactionFree(txRaw);

			tx->raw.inputs = nullptr;
			tx->raw.outputs = nullptr;
			tx->raw.inCount = 0;
			tx->raw.outCount = 0;

			array_new(tx->raw.inputs, 1);

			tx->payload = ELAPayloadNew(tx->type);

			return tx;
		}


		ELATransaction *ELATransactioCopy(const ELATransaction *orig) {
			ELATransaction *tx = new ELATransaction();

			tx->raw = orig->raw;
			tx->raw.inputs = nullptr;
			tx->raw.outputs = nullptr;
			tx->raw.inCount = 0;
			tx->raw.outCount = 0;

			array_new(tx->raw.inputs, 1);

			tx->type = orig->type;
			tx->payloadVersion = orig->payloadVersion;
			tx->payload = ELAPayloadNew(orig->type);
			if (tx->payload) {
				ByteStream stream;
				tx->payload->Serialize(stream);
				stream.setPosition(0);
				tx->payload->Deserialize(stream);
			}

			for (size_t i = 0; i < orig->raw.inCount; ++i) {
				BRTransactionAddInput(&tx->raw, orig->raw.inputs[i].txHash, orig->raw.inputs[i].index,
									  orig->raw.inputs[i].amount, orig->raw.inputs[i].script, orig->raw.inputs[i].scriptLen,
									  orig->raw.inputs[i].signature, orig->raw.inputs[i].sigLen, orig->raw.inputs[i].sequence);
			}

			tx->outputs.clear();
			for (size_t i = 0; i < orig->outputs.size(); ++i) {
				TransactionOutput *output = new TransactionOutput(*orig->outputs[i]);
				tx->outputs.push_back(TransactionOutputPtr(output));
			}

			tx->attributes.clear();
			for (size_t i = 0; i < orig->attributes.size(); ++i) {
				Attribute *attr = new Attribute(*orig->attributes[i]);
				tx->attributes.push_back(AttributePtr(attr));
			}

			tx->programs.clear();
			for (size_t i = 0; i < orig->programs.size(); ++i) {
				Program *program = new Program(*orig->programs[i]);
				tx->programs.push_back(ProgramPtr(program));
			}

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
					BRTxOutputSetScript(&tx->raw.outputs[i], nullptr, 0);
				}
				array_free(tx->raw.outputs);
			}

			tx->payload = nullptr;
			tx->outputs.clear();
			tx->attributes.clear();
			tx->programs.clear();

			delete tx;
		}

		// shuffles order of tx outputs
		void ELATransactionShuffleOutputs(ELATransaction *tx) {
			assert(tx != NULL);

			for (uint32_t i = 0; tx && i + 1 < tx->outputs.size(); i++) { // fischer-yates shuffle
				uint32_t j = i + BRRand((uint32_t)tx->outputs.size() - i);
				TransactionOutputPtr t;

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
	}

}