// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef __ELASTOS_SDK_ELATransaction_H
#define __ELASTOS_SDK_ELATransaction_H

#include <vector>
#include <cstdint>
#include <Core/BRTransaction.h>

#include "SDK/ELACoreExt/Payload/PayloadCoinBase.h"
#include "SDK/Wrapper/SharedWrapperList.h"
#include "SDK/Transaction/TransactionOutput.h"
#include "SDK/Wrapper/Program.h"
#include "BRTransaction.h"
#include "CMemBlock.h"
#include "Attribute.h"
#include "ELATxOutput.h"
#include "ELACoreExt/Payload/IPayload.h"
#include "BRArray.h"

namespace Elastos {
	namespace ElaWallet {
		#define TX_VERSION           0x00000001
		#define TX_LOCKTIME          0x00000000

		struct ELATransaction {

			enum Type {
				CoinBase                = 0x00,
				RegisterAsset           = 0x01,
				TransferAsset           = 0x02,
				Record                  = 0x03,
				Deploy                  = 0x04,
				SideMining              = 0x05,
				IssueToken              = 0x06,
				WithdrawAsset           = 0x07,
				TransferCrossChainAsset = 0x08,
				RegisterIdentification	= 0x09,
				TypeMaxCount
			};

			ELATransaction() {
				memset(&raw, 0, sizeof(raw));
				raw.version = TX_VERSION;
				raw.lockTime = TX_LOCKTIME;
				raw.blockHeight = TX_UNCONFIRMED;
				type = CoinBase;
				payloadVersion = 0;
				fee = 0;
				payload = nullptr;
				outputs.clear();
				attributes.clear();
				programs.clear();

				BRTransaction *txRaw = BRTransactionNew();
				raw = *txRaw;
				BRTransactionFree(txRaw);

				raw.inputs = nullptr;
				raw.outputs = nullptr;
				raw.inCount = 0;
				raw.outCount = 0;

				array_new(raw.inputs, 1);

				payload = boost::shared_ptr<PayloadCoinBase>(new PayloadCoinBase());
			}

			BRTransaction raw;
			Type type;
			uint8_t payloadVersion;
			uint64_t fee;
			PayloadPtr payload;
			SharedWrapperList<TransactionOutput, BRTxOutput *> outputs;
			std::vector<AttributePtr> attributes;
			std::vector<ProgramPtr> programs;
			std::string Remark;
		};

		PayloadPtr ELAPayloadNew(ELATransaction::Type type);
		ELATransaction *ELATransactionNew(void);
		ELATransaction *ELATransactionCopy(const ELATransaction *tx);
		void ELATransactionFree(ELATransaction *tx);

		void ELATransactionShuffleOutputs(ELATransaction *tx);
		size_t ELATransactionSize(const ELATransaction *tx);
		uint64_t ELATransactionStandardFee(const ELATransaction *tx);
		bool ELATransactionIsSign(const ELATransaction *tx);
	}
}

#endif //__ELASTOS_SDK_ELATransaction_H
