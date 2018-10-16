// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <SDK/Wrapper/Wallet.h>
#include <SDK/Common/Utils.h>
#include <SDK/Common/Log.h>
#include <SDK/Common/ParamChecker.h>
#include "HDSubAccount.h"

namespace Elastos {
	namespace ElaWallet {

		HDSubAccount::HDSubAccount(const MasterPubKey &masterPubKey, IAccount *account, uint32_t coinIndex) :
				SubAccountBase(account) {
				_coinIndex = coinIndex;
				_masterPubKey = masterPubKey;
		}

		void HDSubAccount::InitWallet(BRTransaction *transactions[], size_t txCount, ELAWallet *wallet) {
			wallet->IsSingleAddress = false;
			wallet->SingleAddress = "";

			wallet->Raw.masterPubKey = *_masterPubKey.getRaw();

			BRTransaction *tx;
			for (size_t i = 0; transactions && i < txCount; i++) {
				tx = transactions[i];
				if (!wallet->Raw.TransactionIsSigned(tx)) continue;

				if (wallet->Raw.WalletAddUsedAddrs) {
					wallet->Raw.WalletAddUsedAddrs((BRWallet *) wallet, tx);
				} else {
					for (size_t j = 0; j < tx->outCount; j++) {
						if (tx->outputs[j].address[0] != '\0') BRSetAdd(wallet->Raw.usedAddrs, tx->outputs[j].address);
					}
				}
			}

			wallet->Raw.WalletUnusedAddrs((BRWallet *) wallet, NULL, SEQUENCE_GAP_LIMIT_EXTERNAL + 100, 0);
			wallet->Raw.WalletUnusedAddrs((BRWallet *) wallet, NULL, SEQUENCE_GAP_LIMIT_INTERNAL + 100, 1);

			wallet->Raw.WalletUpdateBalance((BRWallet *) wallet);

			if (txCount > 0 && !wallet->Raw.WalletContainsTx((BRWallet *) wallet,
															 transactions[0])) { // verify transactions match master pubKey
				ELAWalletFree(wallet);
				wallet = NULL;
				std::stringstream ess;
				ess << "txCount = " << txCount
					<< ", wallet do not contain tx[0] = "
					<< Utils::UInt256ToString(transactions[0]->txHash);
				ParamChecker::checkCondition(true, Error::Wallet, ess.str());
			}
		}

		Key HDSubAccount::DeriveMainAccountKey(const std::string &payPassword) {
			UInt512 seed = _parentAccount->DeriveSeed(payPassword);
			Key key;
			UInt256 chainCode;
			BRBIP32PrivKeyPath(key.getRaw(), &chainCode, &seed, sizeof(seed), 3, 44 | BIP32_HARD,
							   _coinIndex | BIP32_HARD, 0 | BIP32_HARD);
			var_clean(&seed);
			return key;
		}

		void HDSubAccount::SignTransaction(const TransactionPtr &transaction, ELAWallet *wallet,
										   const std::string &payPassword) {
			WrapperList<Key, BRKey> keyList = DeriveAccountAvailableKeys(wallet, payPassword, transaction);
			ParamChecker::checkCondition(!transaction->sign(keyList, 0), Error::Sign,
										 "Transaction Sign error!");
		}

		WrapperList<Key, BRKey>
		HDSubAccount::DeriveAccountAvailableKeys(ELAWallet *wallet, const std::string &payPassword,
												 const TransactionPtr &transaction) {
			BRTransaction *tx = transaction->getRaw();
			uint32_t j, internalIdx[tx->inCount], externalIdx[tx->inCount];
			size_t i, internalCount = 0, externalCount = 0;

			Log::getLogger()->info("SubWallet signTransaction begin get indices.");
			pthread_mutex_lock(&wallet->Raw.lock);
			for (i = 0; i < tx->inCount; i++) {
				if (wallet->Raw.internalChain) {
					for (j = (uint32_t) array_count(wallet->Raw.internalChain); j > 0; j--) {
						if (BRAddressEq(tx->inputs[i].address, &wallet->Raw.internalChain[j - 1])) {
							internalIdx[internalCount++] = j - 1;
						}
					}
				}

				for (j = (uint32_t) array_count(wallet->Raw.externalChain); j > 0; j--) {
					if (BRAddressEq(tx->inputs[i].address, &wallet->Raw.externalChain[j - 1])) {
						externalIdx[externalCount++] = j - 1;
					}
				}
			}
			pthread_mutex_unlock(&wallet->Raw.lock);
			Log::getLogger()->info("SubWallet signTransaction end get indices.");

			UInt512 seed = _parentAccount->DeriveSeed(payPassword);

			BRKey keys[internalCount + externalCount];
			BRBIP44PrivKeyList(keys, internalCount, &seed, sizeof(seed), _coinIndex,
							   SEQUENCE_INTERNAL_CHAIN, internalIdx);
			BRBIP44PrivKeyList(&keys[internalCount], externalCount, &seed, sizeof(seed), _coinIndex,
							   SEQUENCE_EXTERNAL_CHAIN, externalIdx);
			var_clean(&seed);

			Log::getLogger()->info("SubWallet signTransaction calculate private key list done.");

			WrapperList<Key, BRKey> keyList;
			if (tx) {
				Log::getLogger()->info("SubWallet signTransaction begin sign method.");
				for (i = 0; i < internalCount + externalCount; ++i) {
					Key key(keys[i].secret, keys[i].compressed);
					keyList.push_back(key);
				}

				Log::getLogger()->info("SubWallet signTransaction end sign method.");
			}

			for (i = 0; i < internalCount + externalCount; i++) BRKeyClean(&keys[i]);
			return keyList;
		}

		nlohmann::json HDSubAccount::GetBasicInfo() const {
			nlohmann::json j;
			j["Type"] = "HD Account";
			nlohmann::json details;
			details["CoinIndex"] = _coinIndex;
			j["Details"] = details;
			return j;
		}

		std::string HDSubAccount::GetMainAccountPublicKey() const {
			return Utils::encodeHex(_masterPubKey.getPubKey());
		}

	}
}
