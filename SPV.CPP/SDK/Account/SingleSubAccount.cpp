// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <SDK/TransactionHub/TransactionHub.h>
#include <SDK/Common/ParamChecker.h>
#include <SDK/Common/Utils.h>
#include "SingleSubAccount.h"

namespace Elastos {
	namespace ElaWallet {

		SingleSubAccount::SingleSubAccount(IAccount *account) :
				SubAccountBase(account) {

		}

		Key SingleSubAccount::DeriveMainAccountKey(const std::string &payPassword) {
			return _parentAccount->DeriveKey(payPassword);
		}

		std::string SingleSubAccount::GetMainAccountPublicKey() const {
			return _parentAccount->GetPublicKey();
		}

		WrapperList<Key, BRKey>
		SingleSubAccount::DeriveAccountAvailableKeys(const std::string &payPassword,
													 const Elastos::ElaWallet::TransactionPtr &transaction) {
			WrapperList<Key, BRKey> result;
			result.push_back(_parentAccount->DeriveKey(payPassword));
			return result;
		}

		void
		SingleSubAccount::SignTransaction(const TransactionPtr &transaction, const WalletPtr &wallet,
										  const std::string &payPassword) {
			WrapperList<Key, BRKey> keyList = DeriveAccountAvailableKeys(payPassword, transaction);
			ParamChecker::checkCondition(!transaction->sign(keyList, wallet), Error::Sign,
										 "Transaction Sign error!");
		}

		nlohmann::json SingleSubAccount::GetBasicInfo() const {
			nlohmann::json j;
			j["Type"] = "Single Account";
			return j;
		}

		bool SingleSubAccount::IsSingleAddress() const {
			return true;
		}

		std::vector<Address> SingleSubAccount::GetAllAddresses(size_t addrsCount) const {
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
			return address.IsEqual(GetParent()->GetAddress());
		}
	}
}

