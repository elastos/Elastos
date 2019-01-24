// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "HDSubAccount.h"

#include <SDK/TransactionHub/TransactionHub.h>
#include <SDK/Common/Utils.h>
#include <SDK/Common/Log.h>
#include <SDK/Common/ParamChecker.h>
#include <SDK/BIPs/BIP32Sequence.h>

#include <Core/BRCrypto.h>

namespace Elastos {
	namespace ElaWallet {

		HDSubAccount::HDSubAccount(const MasterPubKey &masterPubKey, const CMBlock &votePubKey,
								   IAccount *account, uint32_t coinIndex) :
				SubAccountBase(account) {
				_coinIndex = coinIndex;
				_masterPubKey = masterPubKey;
				_votePublicKey = votePubKey;
		}

		void HDSubAccount::InitAccount(const std::vector<TransactionPtr> &transactions, Lockable *lock) {
			_lock = lock;

			for (size_t i = 0; i < transactions.size(); i++) {
				if (transactions[i]->isSigned()) {
					AddUsedAddrs(transactions[i]);
				}
			}

			UnusedAddresses(SEQUENCE_GAP_LIMIT_EXTERNAL + 100, 0);
			UnusedAddresses(SEQUENCE_GAP_LIMIT_INTERNAL + 100, 1);
		}

		Key HDSubAccount::DeriveMainAccountKey(const std::string &payPassword) {
			UInt512 seed = _parentAccount->DeriveSeed(payPassword);
			UInt256 chainCode;
			Key key = BIP32Sequence::PrivKeyPath(&seed, sizeof(seed), chainCode, 3, 44 | BIP32_HARD,
												 _coinIndex | BIP32_HARD, 0 | BIP32_HARD);
			var_clean(&seed);
			var_clean(&chainCode);
			return key;
		}

		void HDSubAccount::SignTransaction(const TransactionPtr &transaction, const WalletPtr &wallet,
										   const std::string &payPassword) {
			std::vector<Key> keys = DeriveAccountAvailableKeys(payPassword, wallet, transaction);
			ParamChecker::checkCondition(!transaction->Sign(keys, wallet), Error::Sign,
										 "Transaction Sign error!");
		}

		std::vector<Key>
		HDSubAccount::DeriveAccountAvailableKeys(const std::string &payPassword, const WalletPtr &wallet,
												 const TransactionPtr &transaction) {
			uint32_t index;
			size_t i;
			std::vector<uint32_t> indexInternal, indexExternal;

			for (i = 0; i < transaction->getInputs().size(); i++) {
				const TransactionInput &txInput = transaction->getInputs()[i];
				const TransactionPtr &tx = wallet->transactionForHash(txInput.getTransctionHash());
				Address inputAddr = tx->getOutputs()[txInput.getIndex()].getAddress();

				_lock->Lock();
				for (index = (uint32_t) internalChain.size(); index > 0; index--) {
					if (inputAddr.IsEqual(internalChain[index - 1])) {
						indexInternal.push_back(index - 1);
					}
				}

				for (index = (uint32_t) externalChain.size(); index > 0; index--) {
					if (inputAddr.IsEqual(externalChain[index - 1])) {
						indexExternal.push_back(index - 1);
					}
				}
				_lock->Unlock();
			}

			UInt512 seed = _parentAccount->DeriveSeed(payPassword);

			std::vector<Key> keys =  BIP32Sequence::PrivKeyList(&seed, sizeof(seed), _coinIndex,
																SEQUENCE_INTERNAL_CHAIN, indexInternal);
			std::vector<Key> externalKeys = BIP32Sequence::PrivKeyList(&seed, sizeof(seed), _coinIndex,
																	   SEQUENCE_EXTERNAL_CHAIN, indexExternal);

			keys.insert(keys.end(), externalKeys.begin(), externalKeys.end());
			var_clean(&seed);

			return keys;
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
				CMBlock pubKey = BIP32Sequence::PubKey(_masterPubKey, chain, count);

				Key key;
				key.SetPubKey(pubKey);
				Address address = key.GetAddress(PrefixStandard);

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

		Key HDSubAccount::DeriveVoteKey(const std::string &payPasswd) {
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
