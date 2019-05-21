// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "HDSubAccount.h"

#include <SDK/TransactionHub/TransactionHub.h>
#include <SDK/Common/Utils.h>
#include <SDK/Common/Log.h>
#include <SDK/Common/ErrorChecker.h>

#include <Core/BRCrypto.h>

namespace Elastos {
	namespace ElaWallet {



		HDSubAccount::HDSubAccount(const HDKeychain &masterPubKey, const bytes_t &ownerPubKey,
								   IAccount *account, uint32_t coinIndex) :
				SubAccountBase(account) {
				_coinIndex = coinIndex;
				_masterPubKey = masterPubKey;
				_ownerPublicKey = ownerPubKey;
				if (ownerPubKey.size() > 0) {
					_depositAddress = Address(PrefixDeposit, ownerPubKey);
					_ownerAddress = Address(PrefixStandard, ownerPubKey);
				}
		}

		void HDSubAccount::InitAccount(const std::vector<TransactionPtr> &transactions, Lockable *lock) {
			_lock = lock;

			for (size_t i = 0; i < transactions.size(); i++) {
				if (transactions[i]->IsSigned()) {
					AddUsedAddrs(transactions[i]);
				}
			}

			UnusedAddresses(SEQUENCE_GAP_LIMIT_EXTERNAL + 100, 0);
			UnusedAddresses(SEQUENCE_GAP_LIMIT_INTERNAL + 100, 1);
		}

		bytes_t HDSubAccount::GetRedeemScript(const Address &addr) const {
			uint32_t index;
			bytes_t pubKey;

			if (IsDepositAddress(addr)) {
				pubKey = GetOwnerPublicKey();
				return Address(PrefixDeposit, pubKey).RedeemScript();
			}

			if (IsOwnerAddress(addr)) {
				pubKey = GetOwnerPublicKey();
				return Address(PrefixStandard, pubKey).RedeemScript();
			}

			for (index = internalChain.size(); index > 0; index--) {
				if (internalChain[index - 1] == addr) {
					pubKey = _masterPubKey.getChild(SEQUENCE_INTERNAL_CHAIN).getChild(index - 1).pubkey();
					break;
				}
			}

			for (index = externalChain.size(); index > 0 && pubKey.empty(); index--) {
				if (externalChain[index - 1] == addr) {
					pubKey = _masterPubKey.getChild(SEQUENCE_EXTERNAL_CHAIN).getChild(index - 1).pubkey();
					break;
				}
			}

			ErrorChecker::CheckLogic(pubKey.empty(), Error::Address, "Can't found pubKey for addr " +
																			addr.String());

			return Address(PrefixStandard, pubKey).RedeemScript();
		}

		bool HDSubAccount::FindKey(Key &key, const bytes_t &pubKey, const std::string &payPasswd) {
			if (SubAccountBase::FindKey(key, pubKey, payPasswd))
				return true;

			bool found = false;
			Address addr = Address(PrefixStandard, pubKey);
			uint32_t index, change;

			_lock->Lock();

			for (index = (uint32_t) internalChain.size(); index > 0; --index) {
				if (addr == internalChain[index - 1]) {
					found = true;
					change = SEQUENCE_INTERNAL_CHAIN;
					break;
				}
			}

			if (!found) {
				for (index = (uint32_t) externalChain.size(); index > 0; --index) {
					if (addr == externalChain[index - 1]) {
						found = true;
						change = SEQUENCE_EXTERNAL_CHAIN;
						break;
					}
				}
			}

			_lock->Unlock();

			if (!found)
				return false;

			HDSeed hdseed(_parentAccount->DeriveSeed(payPasswd).bytes());
			HDKeychain rootKey(hdseed.getExtendedKey(true));

			HDKeychain masterKey = rootKey.getChild("44'/0'/0'");
			key = masterKey.getChild(change).getChild(index - 1);

			return true;
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

		size_t HDSubAccount::GetAllAddresses(std::vector<Address> &addr, uint32_t start, size_t count, bool containInternal) const {
			size_t maxCount = externalChain.size() + (containInternal ? internalChain.size() : 0);

			addr.clear();

			if ((!containInternal && start >= externalChain.size()) ||
				(containInternal && start >= externalChain.size() + internalChain.size())) {
				return maxCount;
			}

			_lock->Lock();

			for (size_t i = start; i < externalChain.size() && addr.size() < count; i++) {
				addr.push_back(externalChain[i]);
			}

			if (containInternal) {
				maxCount += internalChain.size();
				for (size_t i = start + addr.size(); addr.size() < count && i < externalChain.size() + internalChain.size(); i++) {
					addr.push_back(internalChain[i - externalChain.size()]);
				}
			}

			_lock->Unlock();

			return maxCount;
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
				bytes_t pubKey = _masterPubKey.getChild(chain).getChild(count).pubkey();

				Address address(PrefixStandard, pubKey);

				if (!address.Valid()) break;

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

			for (size_t j = 0; j < tx->GetOutputs().size(); j++) {
				usedAddrs.insert(tx->GetOutputs()[j].GetAddress());
			}
		}

		bool HDSubAccount::IsAddressUsed(const Address &address) const {
			return usedAddrs.find(address) != usedAddrs.end();
		}

		bool HDSubAccount::ContainsAddress(const Address &address) const {
			if (IsDepositAddress(address)) {
				return true;
			}

			if (IsOwnerAddress(address)) {
				return true;
			}

			return allAddrs.find(address) != allAddrs.end();
		}

		void HDSubAccount::ClearUsedAddresses() {
			usedAddrs.clear();
		}

		Key HDSubAccount::DeriveOwnerKey(const std::string &payPasswd) {
			HDSeed hdseed(_parentAccount->DeriveSeed(payPasswd).bytes());
			HDKeychain rootKey(hdseed.getExtendedKey(true));

			// 44'/coinIndex'/account'/change/index
			return rootKey.getChild("44'/0'/1'/0/0");
		}

		size_t HDSubAccount::TxInternalChainIndex(const TransactionPtr &tx) const {
			for (size_t i = internalChain.size(); i > 0; --i) {
				for (size_t o = 0; o < tx->GetOutputs().size(); o++) {
					if (internalChain[i] == tx->GetOutputs()[o].GetAddress())
						return i - 1;
				}
			}

			return -1;
		}

		size_t HDSubAccount::TxExternalChainIndex(const TransactionPtr &tx) const {
			for (size_t i = externalChain.size(); i > 0; --i) {
				for (size_t o = 0; o < tx->GetOutputs().size(); ++o) {
					if (externalChain[i] == tx->GetOutputs()[o].GetAddress())
						return i - 1;
				}
			}

			return -1;
		}

	}
}
