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
				if (votePubKey.GetSize() > 0)
					_depositAddress = Address(votePubKey, PrefixDeposit);
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

		CMBlock HDSubAccount::GetRedeemScript(const Address &addr) const {
			uint32_t index;
			CMBlock pubKey;
			Key key;

			if (IsDepositAddress(addr)) {
				pubKey = GetVotePublicKey();
				key.SetPubKey(pubKey);
				return key.RedeemScript(PrefixDeposit);
			}

			for (index = internalChain.size(); index > 0; index--) {
				if (internalChain[index - 1] == addr) {
					pubKey = BIP32Sequence::PubKey(_masterPubKey, SEQUENCE_INTERNAL_CHAIN, index - 1);
					break;
				}
			}

			for (index = externalChain.size(); index > 0 && pubKey.GetSize() == 0; index--) {
				if (externalChain[index - 1] == addr) {
					pubKey = BIP32Sequence::PubKey(_masterPubKey, SEQUENCE_EXTERNAL_CHAIN, index - 1);
					break;
				}
			}

			ParamChecker::checkLogic(pubKey.GetSize() == 0, Error::Address, "Can't found pubKey for addr " +
				addr.String());

			key.SetPubKey(pubKey);
			return key.RedeemScript(PrefixStandard);
		}

		bool HDSubAccount::FindKey(Key &key, const CMBlock &pubKey, const std::string &payPasswd) {
			if (SubAccountBase::FindKey(key, pubKey, payPasswd))
				return true;

			bool found = false;
			Address addr = Address(pubKey, PrefixStandard);
			uint32_t index, change;
			uint32_t coinIndex;

			_lock->Lock();

			coinIndex = _coinIndex;

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

			UInt256 chainCode;
			UInt512 seed = _parentAccount->DeriveSeed(payPasswd);
			key = BIP32Sequence::PrivKeyPath(seed.u8, sizeof(seed), chainCode, 5, 44 | BIP32_HARD,
											 coinIndex | BIP32_HARD,
											 BIP32::Account::Default | BIP32_HARD,
											 change, index - 1);

			var_clean(&seed);
			var_clean(&chainCode);

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

		std::vector<Address> HDSubAccount::GetAllAddresses(uint32_t start, size_t addrsCount, bool containInternal) const {
			std::vector<Address> result;

			if ((!containInternal && start >= externalChain.size()) ||
				(containInternal && start >= externalChain.size() + internalChain.size())) {
				return result;
			}

			_lock->Lock();

			for (size_t i = start; i < externalChain.size() && result.size() < addrsCount; i++) {
				result.push_back(externalChain[i]);
			}

			if (containInternal && result.size() < addrsCount) {
				for (size_t i = start + result.size(); i < externalChain.size() + internalChain.size() && result.size() < addrsCount; i++) {
					result.push_back(internalChain[i - externalChain.size()]);
				}
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

				Address address(pubKey, PrefixStandard);

				if (!address.IsValid())
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
				usedAddrs.insert(tx->getOutputs()[j].GetAddress());
			}
		}

		bool HDSubAccount::IsAddressUsed(const Address &address) const {
			return usedAddrs.find(address) != usedAddrs.end();
		}

		bool HDSubAccount::ContainsAddress(const Address &address) const {
			if (IsDepositAddress(address)) {
				return true;
			}

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
