// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "StandardSingleSubAccount.h"

#include <SDK/TransactionHub/TransactionHub.h>
#include <SDK/Common/Utils.h>
#include <SDK/Common/Log.h>
#include <SDK/BIPs/BIP32Sequence.h>

#include <Core/BRCrypto.h>

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
		}

		Key StandardSingleSubAccount::DeriveMainAccountKey(const std::string &payPassword) {
			UInt512 seed = _parentAccount->DeriveSeed(payPassword);
			UInt256 chainCode;

			Key key = BIP32Sequence::PrivKeyPath(&seed, sizeof(seed), chainCode, 3, 44 | BIP32_HARD,
												 _coinIndex | BIP32_HARD, 0 | BIP32_HARD);

			var_clean(&seed);
			var_clean(&chainCode);

			return key;
		}

		std::string StandardSingleSubAccount::GetMainAccountPublicKey() const {
			return Utils::encodeHex(_masterPubKey.GetPubKey());
		}

		std::vector<Address> StandardSingleSubAccount::GetAllAddresses(size_t addrsCount) const {
			std::vector<Address> address;

			CMBlock pubKey = BIP32Sequence::PubKey(_masterPubKey, 0, 0);

			Key key;
			key.SetPubKey(pubKey);

			if (addrsCount > 0)
				address.emplace_back(key.GetAddress(PrefixStandard));

			return address;
		}

		std::vector<Key> StandardSingleSubAccount::DeriveAccountAvailableKeys(const std::string &payPassword,
																			  const TransactionPtr &transaction) {
			std::vector<Key> keys;
			UInt512 seed = _parentAccount->DeriveSeed(payPassword);
			UInt256 chainCode;

			Key key = BIP32Sequence::PrivKeyPath(&seed, sizeof(seed), chainCode, 5, 44 | BIP32_HARD,
												 _coinIndex | BIP32_HARD, 0 | BIP32_HARD,
												 SEQUENCE_EXTERNAL_CHAIN, 0);

			var_clean(&seed);
			var_clean(&chainCode);

			keys.push_back(key);

			return keys;
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
