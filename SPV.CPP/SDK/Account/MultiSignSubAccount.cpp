// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "MultiSignSubAccount.h"

#include <SDK/Common/ParamChecker.h>
#include <SDK/Common/Utils.h>
#include <SDK/Plugin/Transaction/Program.h>

#include <Core/BRAddress.h>

#define SignatureScriptLength 65

namespace Elastos {
	namespace ElaWallet {

		MultiSignSubAccount::MultiSignSubAccount(IAccount *account) :
				SingleSubAccount(account) {
			_multiSignAccount = dynamic_cast<MultiSignAccount *>(account);
			ParamChecker::checkCondition(_multiSignAccount == nullptr, Error::WrongAccountType,
										 "Multi-sign sub account do not allow account that are not multi-sign type.");
		}

		void
		MultiSignSubAccount::SignTransaction(const TransactionPtr &transaction, const WalletPtr &wallet,
											 const std::string &payPassword) {
			if (transaction->getPrograms().empty()) {
				Program program;
				program.setCode(_multiSignAccount->GetRedeemScript());
				transaction->addProgram(program);
			}

			ParamChecker::checkCondition(transaction->getPrograms().size() != 1, Error::Transaction,
											  "Multi-sign program should be unique.");

			ByteStream stream;
			Program &program = transaction->getPrograms()[0];
			if (program.getParameter().GetSize() > 0) {
				stream.writeBytes(program.getParameter(), program.getParameter().GetSize());
			}

			UInt256 md = transaction->GetShaData();
			CMBlock signData = DeriveMainAccountKey(payPassword).Sign(md);
			stream.writeVarBytes(signData);

			program.setParameter(stream.getBuffer());
		}

		nlohmann::json MultiSignSubAccount::GetBasicInfo() const {
			nlohmann::json j;
			j["Type"] = "Multi-Sign Account";
			return j;
		}

		std::vector<std::string> MultiSignSubAccount::GetTransactionSignedSigners(const TransactionPtr &transaction) {

			for (std::vector<Program>::const_iterator programIt = transaction->getPrograms().cbegin();
				 programIt != transaction->getPrograms().cend(); ++programIt) {

				const CMBlock &code = programIt->getCode();
				const CMBlock &parameter = programIt->getParameter();
				if (code[code.GetSize() - 1] == ELA_MULTISIG) {
					std::vector<std::string> result;

					Key key;
					uint8_t m, n;
					std::vector<std::string> signers;
					Program::ParseMultiSignRedeemScript(code, m, n, signers);

					UInt256 md = transaction->GetShaData();

					ByteStream stream(parameter);
					CMBlock signature;
					while (stream.readVarBytes(signature)) {
						for (int i = 0; i < signers.size(); ++i) {
							key.SetPubKey(Utils::decodeHex(signers[i]));

							if (key.Verify(md, signature)) {
								result.push_back(signers[i]);
							}
						}
					}

					return result;
				}
			}

			return std::vector<std::string>();
		}

	}
}
