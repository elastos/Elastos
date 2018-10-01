// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <SDK/Common/ParamChecker.h>
#include "MultiSignSubAccount.h"
#include "Program.h"

#define SignatureScriptLength 65

namespace Elastos {
	namespace ElaWallet {

		MultiSignSubAccount::MultiSignSubAccount(IAccount *account) :
				SingleSubAccount(account) {
			_multiSignAccount = dynamic_cast<MultiSignAccount *>(account);
			ParamChecker::checkCondition(_multiSignAccount == nullptr, Error::WrongAccountType,
										 "Multi-sign sub account do not allow account that are not multi-sign type.");
		}

		void MultiSignSubAccount::SignTransaction(const TransactionPtr &transaction, ELAWallet *wallet,
												  const std::string &payPassword) {
			ELATransaction *elaTransaction = (ELATransaction *) transaction->getRaw();
			if (elaTransaction->programs.empty()) {
				Program *program = new Program;
				program->setCode(_multiSignAccount->GenerateRedeemScript());
				elaTransaction->programs.push_back(program);
			}

			ParamChecker::checkCondition(elaTransaction->programs.size() != 1, Error::Transaction,
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

			for (std::vector<Program *>::const_iterator programIt = transaction->getPrograms().cbegin();
				 programIt != transaction->getPrograms().cend(); ++programIt) {

				const CMBlock &code = (*programIt)->getCode();
				const CMBlock &parameter = (*programIt)->getParameter();
				if (code[code.GetSize() - 1] == ELA_MULTISIG) {
					std::vector<std::string> result;

					uint8_t m, n;
					std::vector<std::string> signers;
					Program::ParseMultiSignRedeemScript(code, m, n, signers);

					CMBlock hashData = transaction->GetShaData();
					UInt256 md;
					memcpy(md.u8, hashData, sizeof(UInt256));

					for (int i = 0; i < parameter.GetSize(); i += SignatureScriptLength) {
						CMBlock signature(SignatureScriptLength);
						memcpy(signature, &parameter[i], SignatureScriptLength);

						for (std::vector<std::string>::iterator signerIt = signers.begin();
							 signerIt != signers.end(); ++signerIt) {

							if (Key::verifyByPublicKey(*signerIt, md, signature))
								result.push_back(*signerIt);
						}
					}

					return result;
				}
			}

			return std::vector<std::string>();
		}

	}
}
