// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <SDK/Wrapper/TransactionHub.h>
#include <SDK/Common/Utils.h>
#include <SDK/Common/Log.h>
#include "StandardSingleSubAccount.h"

namespace Elastos {
	namespace ElaWallet {

		StandardSingleSubAccount::StandardSingleSubAccount(const MasterPubKey &masterPubKey, IAccount *account,
														   uint32_t coinIndex) :
				SingleSubAccount(account) {
				_masterPubKey = masterPubKey;
				_coinIndex = coinIndex;
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

		void StandardSingleSubAccount::InitWallet(BRTransaction **transactions, size_t txCount, ELAWallet *wallet) {
			wallet->IsSingleAddress = true;

			size_t len = BRBIP32PubKey(NULL, 0, *_masterPubKey.getRaw(), SEQUENCE_EXTERNAL_CHAIN, 0);
			CMBlock pubKey(len);
			BRBIP32PubKey(pubKey, pubKey.GetSize(), *_masterPubKey.getRaw(), SEQUENCE_EXTERNAL_CHAIN, 0);
			Key key;
			key.setPubKey(pubKey);
			wallet->SingleAddress = key.address();

			wallet->Raw.WalletUpdateBalance((BRWallet *) wallet);
		}

		WrapperList<Key, BRKey> StandardSingleSubAccount::DeriveAccountAvailableKeys(const std::string &payPassword,
																					 const TransactionPtr &transaction) {
			WrapperList<Key, BRKey> result;
			UInt512 seed = _parentAccount->DeriveSeed(payPassword);
			Key key;
			UInt256 chainCode;
			BRBIP32PrivKeyPath(key.getRaw(), &chainCode, &seed, sizeof(seed), 5, 44 | BIP32_HARD,
							   _coinIndex | BIP32_HARD, 0 | BIP32_HARD, SEQUENCE_EXTERNAL_CHAIN, 0);
			key.setPublicKey();
			var_clean(&seed);
			result.push_back(key);
			return result;
		}
	}
}
