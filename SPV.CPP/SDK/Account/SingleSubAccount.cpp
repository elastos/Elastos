// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "SingleSubAccount.h"

#include <SDK/TransactionHub/TransactionHub.h>
#include <SDK/Common/ParamChecker.h>
#include <SDK/Common/Utils.h>

namespace Elastos {
	namespace ElaWallet {

		SingleSubAccount::SingleSubAccount(IAccount *account) :
				SubAccountBase(account) {

		}

		SingleSubAccount::~SingleSubAccount() {
		}

		nlohmann::json SingleSubAccount::GetBasicInfo() const {
			nlohmann::json j;
			j["Type"] = "Single Account";
			return j;
		}

		CMBlock SingleSubAccount::GetRedeemScript(const Address &addr) const {
			Key key;
			key.SetPubKey(_parentAccount->GetMultiSignPublicKey());

			ParamChecker::checkLogic(addr != key.GetAddress(PrefixStandard) || addr != key.GetAddress(PrefixDeposit),
									 Error::Address, "Can't found pubKey for addr " + addr.String());

			return key.RedeemScript(PrefixStandard);
		}

		bool SingleSubAccount::IsSingleAddress() const {
			return true;
		}

		std::vector<Address> SingleSubAccount::GetAllAddresses(uint32_t start, size_t addrsCount, bool containInternal) const {
			std::vector<Address> result;
			if (addrsCount > 0) {
				result.push_back(GetParent()->GetAddress());
			}
			return result;
		}

		std::vector<Address> SingleSubAccount::UnusedAddresses(uint32_t gapLimit, bool internal) {
			std::vector<Address> result;
			result.push_back(GetParent()->GetAddress());
			return result;
		}

		bool SingleSubAccount::IsAddressUsed(const Address &address) const {
			return true;
		}

		bool SingleSubAccount::ContainsAddress(const Address &address) const {
			return address == GetParent()->GetAddress();
		}

		Key SingleSubAccount::DeriveVoteKey(const std::string &payPasswd) {
			ParamChecker::throwLogicException(Error::AccountNotSupportVote, "This account do not support vote");
			return Key();
		}

	}
}

