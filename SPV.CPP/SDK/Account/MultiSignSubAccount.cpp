// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "MultiSignSubAccount.h"

#include <SDK/Common/ErrorChecker.h>
#include <SDK/Common/Utils.h>
#include <SDK/Plugin/Transaction/Program.h>

#include <Core/BRAddress.h>

#define SignatureScriptLength 65

namespace Elastos {
	namespace ElaWallet {

		MultiSignSubAccount::MultiSignSubAccount(IAccount *account) :
				SingleSubAccount(account) {
			_multiSignAccount = dynamic_cast<MultiSignAccount *>(account);
			ErrorChecker::CheckCondition(_multiSignAccount == nullptr, Error::WrongAccountType,
										 "Multi-sign sub account do not allow account that are not multi-sign type.");
		}

		bytes_t MultiSignSubAccount::GetRedeemScript(const Address &addr) const {
			ErrorChecker::CheckLogic(_multiSignAccount->GetAddress() != addr, Error::Address,
									 "Can't found pubKey for addr " + addr.String());
			return _multiSignAccount->GetRedeemScript();
		}

		bool MultiSignSubAccount::FindKey(Key &key, const bytes_t &pubKey, const std::string &payPasswd) {
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

		bytes_t MultiSignSubAccount::GetOwnerPublicKey() const {
			return bytes_t();
		}

		Key MultiSignSubAccount::DeriveOwnerKey(const std::string &payPasswd) {
			ErrorChecker::ThrowLogicException(Error::AccountNotSupportVote, "This account do not support vote");
			return Key();
		}

		size_t MultiSignSubAccount::TxInternalChainIndex(const TransactionPtr &tx) const {
			for (size_t i = 0; i < tx->GetOutputs().size(); ++i) {
				if (_parentAccount->GetAddress() == tx->GetOutputs()[i].GetAddress())
					return 0;
			}

			return -1;
		}

		size_t MultiSignSubAccount::TxExternalChainIndex(const TransactionPtr &tx) const {
			for (size_t i = 0; i < tx->GetOutputs().size(); ++i) {
				if (_parentAccount->GetAddress() == tx->GetOutputs()[i].GetAddress())
					return 0;
			}

			return -1;
		}
	}
}
