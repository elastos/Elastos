// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "StandardSingleSubAccount.h"

#include <SDK/TransactionHub/TransactionHub.h>
#include <SDK/Common/Utils.h>
#include <SDK/Common/Log.h>
#include <SDK/BIPs/BIP32Sequence.h>

#include <Core/BRCrypto.h>
#include <SDK/Common/ParamChecker.h>

namespace Elastos {
	namespace ElaWallet {

		StandardSingleSubAccount::StandardSingleSubAccount(const MasterPubKey &masterPubKey,
														   const CMBlock &votePubKey,
														   IAccount *account,
														   uint32_t coinIndex) :
				SingleSubAccount(account) {
				_masterPubKey = masterPubKey;
				_coinIndex = coinIndex;
				_votePublicKey = votePubKey;
				if (votePubKey.GetSize() > 0)
					_depositAddress = Address(votePubKey, PrefixDeposit);

				CMBlock pubKey = BIP32Sequence::PubKey(_masterPubKey, 0, 0);

				_address = Address(pubKey, PrefixStandard);
		}

		CMBlock StandardSingleSubAccount::GetRedeemScript(const Address &addr) const {
			CMBlock pubKey;
			Key key;

			if (IsDepositAddress(addr)) {
				pubKey = GetVotePublicKey();
				key.SetPubKey(pubKey);
				return key.RedeemScript(PrefixDeposit);
			}

			pubKey = BIP32Sequence::PubKey(_masterPubKey, 0, 0);
			key.SetPubKey(pubKey);
			ParamChecker::checkLogic(addr != _address, Error::Address, "Can't found pubKey for addr " + addr.String());
			return key.RedeemScript(PrefixStandard);
		}

		bool StandardSingleSubAccount::FindKey(Key &key, const CMBlock &pubKey, const std::string &payPasswd) {
			if (SubAccountBase::FindKey(key, pubKey, payPasswd)) {
				return true;
			}

			if (pubKey == BIP32Sequence::PubKey(_masterPubKey, 0, 0)) {
				UInt512 seed = _parentAccount->DeriveSeed(payPasswd);
				UInt256 chainCode;
				key = BIP32Sequence::PrivKeyPath(seed.u8, sizeof(seed), chainCode, 5, 44 | BIP32_HARD,
												 _coinIndex | BIP32_HARD,
												 BIP32::Account::Default | BIP32_HARD,
												 BIP32::External, 0);
				var_clean(&seed);
				var_clean(&chainCode);
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
			UInt512 seed = _parentAccount->DeriveSeed(payPasswd);

			UInt256 chainCode;

			Key key = BIP32Sequence::PrivKeyPath(&seed, sizeof(seed), chainCode, 5, 44 | BIP32_HARD,
												 _coinIndex | BIP32_HARD, BIP32::Account::Vote | BIP32_HARD,
												 BIP32::External, 0);

			var_clean(&seed);
			var_clean(&chainCode);

			return key;
		}

	}
}
