// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "MultiSignSubAccount.h"
#include "ErrorCode.h"

namespace Elastos {
	namespace ElaWallet {

		MultiSignSubAccount::MultiSignSubAccount(IAccount *account) :
				SingleSubAccount(account) {
			_multiSignAccount = dynamic_cast<MultiSignAccount *>(account);
			if (_multiSignAccount == nullptr) {
				ErrorCode::StandardLogicError(ErrorCode::WrongAccountType,
											  "Multi-sign sub account do not allow account that are not multi-sign type.");
			}
		}

		void MultiSignSubAccount::SignTransaction(const TransactionPtr &transaction, ELAWallet *wallet,
												  const std::string &payPassword) {
			ELATransaction *elaTransaction = (ELATransaction *) transaction->getRaw();
			if (elaTransaction->programs.empty()) {
				Program *program = new Program;
				program->setCode(_multiSignAccount->GenerateRedeemScript());
				elaTransaction->programs.push_back(program);
			}

			if (elaTransaction->programs.size() != 1)
				ErrorCode::StandardLogicError(ErrorCode::TransactionContentError,
											  "Multi-sign program should be unique.");

			ByteStream stream;
			if (elaTransaction->programs[0]->getParameter().GetSize() > 0) {
				stream.putBytes(elaTransaction->programs[0]->getParameter(),
								elaTransaction->programs[0]->getParameter().GetSize());
			}

			CMBlock shaData = transaction->GetShaData();
			CMBlock signData = DeriveMainAccountKey(payPassword).compactSign(shaData);
			uint8_t buff[65];
			memset(buff, 0, 65);
			memcpy(buff, signData, signData.GetSize());
			stream.putBytes(buff, 65);

			elaTransaction->programs[0]->setParameter(stream.getBuffer());
		}

		nlohmann::json MultiSignSubAccount::GetBasicInfo() const {
			nlohmann::json j;
			j["Type"] = "Multi-Sign Account";
			return j;
		}

		std::vector<std::string> MultiSignSubAccount::GetTransactionSignedSigners(const TransactionPtr &transaction) {
			std::vector<std::string> result;

			return result;
		}

	}
}
