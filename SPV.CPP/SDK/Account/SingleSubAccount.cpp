// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "SingleSubAccount.h"

#include <SDK/TransactionHub/TransactionHub.h>
#include <SDK/Common/ErrorChecker.h>
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

		bytes_t SingleSubAccount::GetRedeemScript(const Address &addr) const {
			// TODO fix here later
			bytes_t pubkey = _parentAccount->GetMultiSignPublicKey();
			Address standard(PrefixStandard, pubkey);
			Address deposit(PrefixDeposit, pubkey);

			ErrorChecker::CheckLogic(addr != standard && addr != deposit,
									 Error::Address, "Can't found pubKey for addr " + addr.String());

			return standard.RedeemScript();
		}

		bool SingleSubAccount::IsSingleAddress() const {
			return true;
		}

		size_t SingleSubAccount::GetAllAddresses(std::vector<Address> &addr, uint32_t start, size_t count, bool containInternal) const {
			addr.clear();
			if (start == 0 && count > 0) {
				addr.push_back(GetParent()->GetAddress());
			}

			return 1;
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
			ErrorChecker::ThrowLogicException(Error::AccountNotSupportVote, "This account do not support vote");
			return Key();
		}

	}
}

