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
		}

		CMBlock StandardSingleSubAccount::GetRedeemScript(const std::string &addr) const {
			CMBlock pubKey;
			Key key;

			if (IsDepositAddress(addr)) {
				pubKey = GetVotePublicKey();
				key.SetPubKey(pubKey);
				return key.RedeemScript(PrefixDeposit);
			}

			pubKey = BIP32Sequence::PubKey(_masterPubKey, 0, 0);
			key.SetPubKey(pubKey);
			ParamChecker::checkLogic(addr != key.GetAddress(PrefixStandard), Error::Address,
									 "Can't found pubKey for addr " + addr);
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
			std::vector<Address> address;

			address.push_back(GetAddress());

			return address;
		}

		std::vector<Address> StandardSingleSubAccount::GetAllAddresses(size_t addrsCount) const {
			std::vector<Address> address;

			if (addrsCount > 0)
				address.emplace_back(GetAddress());

			return address;
		}

		bool StandardSingleSubAccount::ContainsAddress(const Address &address) const {
			const CMBlock &producerPubKey = GetVotePublicKey();
			if (producerPubKey.GetSize() > 0) {
				Key key;
				key.SetPubKey(GetVotePublicKey());
				if (address.IsEqual(key.GetAddress(PrefixDeposit)))
					return true;
			}

			return address.IsEqual(GetAddress());
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

		std::string StandardSingleSubAccount::GetAddress() const {
			CMBlock pubKey = BIP32Sequence::PubKey(_masterPubKey, 0, 0);

			Key key;
			key.SetPubKey(pubKey);

			return key.GetAddress(PrefixStandard);
		}

	}
}
