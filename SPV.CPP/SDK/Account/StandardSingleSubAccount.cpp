// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "StandardSingleSubAccount.h"

#include <SDK/TransactionHub/TransactionHub.h>
#include <SDK/Common/Utils.h>
#include <SDK/Common/Log.h>

#include <Core/BRCrypto.h>
#include <SDK/Common/ErrorChecker.h>

namespace Elastos {
	namespace ElaWallet {

		StandardSingleSubAccount::StandardSingleSubAccount(const HDKeychain &masterPubKey,
														   const bytes_t &votePubKey,
														   IAccount *account,
														   uint32_t coinIndex) :
				SingleSubAccount(account) {
				_masterPubKey = masterPubKey;
				_coinIndex = coinIndex;
				_votePublicKey = votePubKey;
				if (votePubKey.size() > 0)
					_depositAddress = Address(PrefixDeposit, votePubKey);

				_address = Address(PrefixStandard, _masterPubKey.getChild("0/0").pubkey());
		}

		bytes_t StandardSingleSubAccount::GetRedeemScript(const Address &addr) const {
			bytes_t pubKey;
			Key key;

			if (IsDepositAddress(addr)) {
				pubKey = GetVotePublicKey();
				return Address(PrefixDeposit, pubKey).RedeemScript();
			}

			key.SetPubKey(_masterPubKey.getChild("0/0").pubkey());
			ErrorChecker::CheckLogic(addr != _address, Error::Address, "Can't found pubKey for addr " + addr.String());
			return _address.RedeemScript();
		}

		bool StandardSingleSubAccount::FindKey(Key &key, const bytes_t &pubKey, const std::string &payPasswd) {
			if (SubAccountBase::FindKey(key, pubKey, payPasswd)) {
				return true;
			}

			if (pubKey == _masterPubKey.getChild("0/0").pubkey()) {
				HDSeed hdseed(_parentAccount->DeriveSeed(payPasswd).bytes());
				HDKeychain rootKey(hdseed.getExtendedKey(true));

				key = rootKey.getChild("44'/0'/0'/0/0");
				return true;
			}

			return false;
		}

		std::vector<Address> StandardSingleSubAccount::UnusedAddresses(uint32_t gapLimit, bool internal) {
			return {_address};
		}

		size_t StandardSingleSubAccount::GetAllAddresses(std::vector<Address> &addr, uint32_t start, size_t count, bool containInternal) const {
			addr.clear();
			if (start == 0 && count > 0)
				addr.emplace_back(_address);

			return 1;
		}

		bool StandardSingleSubAccount::ContainsAddress(const Address &address) const {
			if (IsDepositAddress(address))
				return true;

			return address == _address;
		}

		Key StandardSingleSubAccount::DeriveVoteKey(const std::string &payPasswd) {
			HDSeed hdseed(_parentAccount->DeriveSeed(payPasswd).bytes());
			HDKeychain rootKey(hdseed.getExtendedKey(true));
			// account is 1
			return rootKey.getChild("44'/0'/1'/0/0");
		}

	}
}
