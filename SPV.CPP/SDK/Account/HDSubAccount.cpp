// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <SDK/TransactionHub/TransactionHub.h>
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

		void HDSubAccount::InitAccount(const std::vector<TransactionPtr> &transactions, Lockable *lock) {
			_lock = lock;

			for (size_t i = 0; transactions.size(); i++) {
				if (!transactions[i]->isSigned()) continue;
				AddUsedAddrs(transactions[i]);
			}

			UnusedAddresses(SEQUENCE_GAP_LIMIT_EXTERNAL + 100, 0);
			UnusedAddresses(SEQUENCE_GAP_LIMIT_INTERNAL + 100, 1);
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

		void HDSubAccount::SignTransaction(const TransactionPtr &transaction, const WalletPtr &wallet,
										   const std::string &payPassword) {
			WrapperList<Key, BRKey> keyList = DeriveAccountAvailableKeys(payPassword, wallet, transaction);
			ParamChecker::checkCondition(!transaction->sign(keyList, 0), Error::Sign,
										 "Transaction Sign error!");
		}

		WrapperList<Key, BRKey>
		HDSubAccount::DeriveAccountAvailableKeys(const std::string &payPassword, const WalletPtr &wallet,
												 const TransactionPtr &transaction) {
			uint32_t j, internalIdx[transaction->getInputs().size()], externalIdx[transaction->getInputs().size()];
			size_t i, internalCount = 0, externalCount = 0;

			Log::info("SubWallet signTransaction begin get indices.");

			_lock->Lock();
			for (i = 0; i < transaction->getInputs().size(); i++) {
				const TransactionInput &txInput = transaction->getInputs()[i];
				const TransactionPtr &tx = wallet->transactionForHash(txInput.getTransctionHash());
				Address inputAddr = tx->getOutputs()[txInput.getIndex()].getAddress();

				for (j = (uint32_t) internalChain.size(); j > 0; j--) {
					if (inputAddr.IsEqual(internalChain[j - 1])) {
						internalIdx[internalCount++] = j - 1;
					}
				}

				for (j = (uint32_t) externalChain.size(); j > 0; j--) {
					if (inputAddr.IsEqual(externalChain[j - 1])) {
						externalIdx[externalCount++] = j - 1;
					}
				}
			}
			_lock->Unlock();

			UInt512 seed;
			BRKey keys[internalCount + externalCount];
			BRBIP44PrivKeyList(keys, internalCount, &seed, sizeof(seed), _coinIndex,
							   SEQUENCE_INTERNAL_CHAIN, internalIdx);
			BRBIP44PrivKeyList(&keys[internalCount], externalCount, &seed, sizeof(seed), _coinIndex,
							   SEQUENCE_EXTERNAL_CHAIN, externalIdx);
			var_clean(&seed);

			Log::info("SubWallet signTransaction calculate private key list done.");
			WrapperList<Key, BRKey> keyList;
			if (transaction) {
				Log::info("SubWallet signTransaction begin sign method.");
				for (i = 0; i < internalCount + externalCount; ++i) {
					Key key(keys[i].secret, keys[i].compressed);
					keyList.push_back(key);
				}

				Log::info("SubWallet signTransaction end sign method.");
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

		bool HDSubAccount::IsSingleAddress() const {
			return false;
		}

		std::vector<Address> HDSubAccount::GetAllAddresses(size_t addrsCount) const {
			std::vector<Address> result;
			size_t i, internalCount = 0, externalCount = 0;

			_lock->Lock();

			externalCount = externalChain.size() < addrsCount?
							externalChain.size() : addrsCount;

			for (i = 0; i < externalCount; i++) {
				result.push_back(externalChain[i]);
			}

			internalCount = internalChain.size() < addrsCount - externalCount ?
							internalChain.size() : addrsCount - externalCount;

			for (i = 0; i < internalCount; i++) {
				result.push_back(internalChain[i]);
			}

			_lock->Unlock();

			return result;
		}

		std::vector<Address> HDSubAccount::UnusedAddresses(uint32_t gapLimit, bool internal) {

			size_t i, j = 0, count, startCount;
			uint32_t chain = (internal) ? SEQUENCE_INTERNAL_CHAIN : SEQUENCE_EXTERNAL_CHAIN;

			assert(gapLimit > 0);

			_lock->Lock();
			std::vector<Address> &addrChain = internal ? internalChain : externalChain;
			i = count = startCount = addrChain.size();

			// keep only the trailing contiguous block of addresses with no transactions
			while (i > 0 && usedAddrs.find(addrChain[i - 1]) == usedAddrs.end()) i--;

			std::vector<Address> addrs;
			while (i + gapLimit > count) { // generate new addresses up to gapLimit
				Key key;

				uint8_t pubKey[BRBIP32PubKey(NULL, 0, *_masterPubKey.getRaw(), chain, count)];
				size_t len = BRBIP32PubKey(pubKey, sizeof(pubKey), *_masterPubKey.getRaw(), chain, count);

				CMBlock publicKey(len);
				memcpy(publicKey, pubKey, len);

				if (!key.setPubKey(publicKey)) break;
				Address address = KeyToAddress(key.getRaw());
				if (address.IsEqual(Address::None))
					break;

				addrChain.push_back(address);
				count++;
				if (usedAddrs.find(address) != usedAddrs.end()) i = count;
			}

			if (i + gapLimit <= count) {
				for (j = 0; j < gapLimit; j++) {
					addrs.push_back(addrChain[i + j]);
				}
			}

			for (i = startCount; i < count; i++) {
				allAddrs.insert(addrChain[i]);
			}
			_lock->Unlock();

			return addrs;
		}

		void HDSubAccount::AddUsedAddrs(const TransactionPtr &tx) {
			if (tx == nullptr)
				return;

			for (size_t j = 0; j < tx->getOutputs().size(); j++) {
				if (!tx->getOutputs()[j].getAddress().empty())
					usedAddrs.insert(tx->getOutputs()[j].getAddress());
			}
		}

		bool HDSubAccount::IsAddressUsed(const Address &address) const {
			return usedAddrs.find(address) != usedAddrs.end();
		}

		bool HDSubAccount::ContainsAddress(const Address &address) const {
			return allAddrs.find(address) != allAddrs.end();
		}

		void HDSubAccount::ClearUsedAddresses() {
			usedAddrs.clear();
		}

	}
}
