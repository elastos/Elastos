// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "SubAccountBase.h"

namespace Elastos {
	namespace ElaWallet {

		SubAccountBase::SubAccountBase(IAccount *account) :
			_coinIndex(0),
			_parentAccount(account) {

		}

		IAccount *SubAccountBase::GetParent() const {
			return _parentAccount;
		}

		void SubAccountBase::InitAccount(const std::vector<TransactionPtr> &transactions, Lockable *lock) {

		}

		void SubAccountBase::AddUsedAddrs(const TransactionPtr &tx) {

		}

		void SubAccountBase::ClearUsedAddresses() {

		}

		std::string SubAccountBase::GetMainAccountPublicKey() const {
			return _parentAccount->GetPublicKey();
		}

		const CMBlock &SubAccountBase::GetVotePublicKey() const {
			return _votePublicKey;
		}

		bool SubAccountBase::IsDepositAddress(const Address &address) const {
			if (_votePublicKey.GetSize() == 0) {
				return false;
			}

			Key key;
			if (!key.SetPubKey(_votePublicKey)) {
				return false;
			}

			return address.IsEqual(key.GetAddress(PrefixDeposit));
		}

	}
}
