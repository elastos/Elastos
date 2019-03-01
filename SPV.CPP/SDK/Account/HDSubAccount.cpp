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
				if (transactions[i]->IsSigned()) {
					AddUsedAddrs(transactions[i]);
				}
			}

			UnusedAddresses(SEQUENCE_GAP_LIMIT_EXTERNAL + 100, 0);
			UnusedAddresses(SEQUENCE_GAP_LIMIT_INTERNAL + 100, 1);
		}

		CMBlock HDSubAccount::GetRedeemScript(const std::string &addr) const {
			uint32_t index;
			CMBlock pubKey;
			Key key;

			if (IsDepositAddress(addr)) {
				pubKey = GetVotePublicKey();
				key.SetPubKey(pubKey);
				return key.RedeemScript(PrefixDeposit);
			}

			for (index = internalChain.size(); index > 0; index--) {
				if (internalChain[index - 1].stringify() == addr) {
					pubKey = BIP32Sequence::PubKey(_masterPubKey, SEQUENCE_INTERNAL_CHAIN, index - 1);
					break;
				}
			}

			for (index = externalChain.size(); index > 0 && pubKey.GetSize() == 0; index--) {
				if (externalChain[index - 1].stringify() == addr) {
					pubKey = BIP32Sequence::PubKey(_masterPubKey, SEQUENCE_EXTERNAL_CHAIN, index - 1);
					break;
				}
			}

			ParamChecker::checkLogic(pubKey.GetSize() == 0, Error::Address, "Can't found pubKey for addr " + addr);

			key.SetPubKey(pubKey);
			return key.RedeemScript(PrefixStandard);
		}

		bool HDSubAccount::FindKey(Key &key, const CMBlock &pubKey, const std::string &payPasswd) {
			if (SubAccountBase::FindKey(key, pubKey, payPasswd))
				return true;

			bool found = false;
			Key searchKey;
			searchKey.SetPubKey(pubKey);
			std::string addr = searchKey.GetAddress(PrefixStandard);
			uint32_t index, change;
			uint32_t coinIndex;

			_lock->Lock();

			coinIndex = _coinIndex;

			for (index = (uint32_t) internalChain.size(); index > 0; --index) {
				if (addr == internalChain[index - 1].stringify()) {
					found = true;
					change = SEQUENCE_INTERNAL_CHAIN;
					break;
				}
			}

			if (!found) {
				for (index = (uint32_t) externalChain.size(); index > 0; --index) {
					if (addr == externalChain[index - 1].stringify()) {
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
			const CMBlock &producerPubKey = GetVotePublicKey();
			if (producerPubKey.GetSize() > 0) {
				Key key;
				key.SetPubKey(GetVotePublicKey());
				if (address.IsEqual(key.GetAddress(PrefixDeposit)))
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
