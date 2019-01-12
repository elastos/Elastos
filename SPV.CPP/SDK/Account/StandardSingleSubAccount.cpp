// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "StandardSingleSubAccount.h"

#include <SDK/TransactionHub/TransactionHub.h>
#include <SDK/Common/Utils.h>
#include <SDK/Common/Log.h>

#include <Core/BRCrypto.h>
#include <Core/BRKey.h>

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
			Key key;
			UInt256 chainCode;
			BRBIP32PrivKeyPath(key.getRaw(), &chainCode, &seed, sizeof(seed), 3, 44 | BIP32_HARD,
							   _coinIndex | BIP32_HARD, 0 | BIP32_HARD);
			var_clean(&seed);
			return key;
		}

		std::string StandardSingleSubAccount::GetMainAccountPublicKey() const {
			return Utils::encodeHex(_masterPubKey.getPubKey());
		}

		std::vector<Address> StandardSingleSubAccount::GetAllAddresses(size_t addrsCount) const {
			std::vector<Address> address;
			CMBlock pubKey(65);
			size_t len = BRBIP32PubKey(pubKey, pubKey.GetSize(), *_masterPubKey.getRaw(), 0, 0);
			Key key;

			pubKey.Resize(len);
			key.SetPublicKey(pubKey);

			if (addrsCount > 0)
				address.emplace_back(key.address());

			return address;
		}

		WrapperList<Key, BRKey> StandardSingleSubAccount::DeriveAccountAvailableKeys(const std::string &payPassword,
																					 const TransactionPtr &transaction) {
			WrapperList<Key, BRKey> result;
			UInt512 seed = _parentAccount->DeriveSeed(payPassword);
			UInt256 chainCode;
			BRKey brKey;
			BRBIP32PrivKeyPath(&brKey, &chainCode, &seed, sizeof(seed), 5, 44 | BIP32_HARD,
							   _coinIndex | BIP32_HARD, 0 | BIP32_HARD, SEQUENCE_EXTERNAL_CHAIN, 0);
			Key key(brKey);
			var_clean(&seed);
			result.push_back(key);
			return result;
		}

		Key StandardSingleSubAccount::DeriveVoteKey(const std::string &payPasswd) {
			UInt512 seed = _parentAccount->DeriveSeed(payPasswd);

			UInt256 chainCode;

			BRKey brKey;
			BRBIP32PrivKeyPath(&brKey, &chainCode, &seed, sizeof(seed), 5, 44 | BIP32_HARD,
								_coinIndex | BIP32_HARD, BIP32::Account::Vote | BIP32_HARD, BIP32::External, 0);
			Key key(brKey);

			var_clean(&seed);
			var_clean(&chainCode);
			var_clean(&brKey.secret);

			return key;
		}

	}
}
