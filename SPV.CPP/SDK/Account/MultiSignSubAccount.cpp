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

		CMBlock MultiSignSubAccount::GetRedeemScript(const Address &addr) const {
			ParamChecker::checkLogic(_multiSignAccount->GetAddress() != addr, Error::Address,
									 "Can't found pubKey for addr " + addr.String());
			return _multiSignAccount->GetRedeemScript();
		}

		bool MultiSignSubAccount::FindKey(Key &key, const CMBlock &pubKey, const std::string &payPasswd) {
			if (GetMultiSignPublicKey() == pubKey) {
				key = DeriveMultiSignKey(payPasswd);
				return true;
			}

			return false;
		}

		nlohmann::json MultiSignSubAccount::GetBasicInfo() const {
			nlohmann::json j;
			j["Type"] = "Multi-Sign Account";
			return j;
		}

		CMBlock MultiSignSubAccount::GetVotePublicKey() const {
			return CMBlock();
		}

		Key MultiSignSubAccount::DeriveVoteKey(const std::string &payPasswd) {
			ParamChecker::throwLogicException(Error::AccountNotSupportVote, "This account do not support vote");
			return Key();
		}

	}
}
