// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "SubAccount.h"

#include <SDK/Wallet/Wallet.h>
#include <SDK/Common/Utils.h>
#include <SDK/Common/Log.h>
#include <SDK/Common/ErrorChecker.h>
#include <SDK/Plugin/Transaction/Transaction.h>
#include <SDK/Plugin/Transaction/TransactionOutput.h>
#include <SDK/Plugin/Transaction/Program.h>
#include <SDK/Plugin/Transaction/Attribute.h>

namespace Elastos {
	namespace ElaWallet {


		SubAccount::SubAccount(const AccountPtr &parent, uint32_t coinIndex) :
			_parent(parent),
			_coinIndex(coinIndex) {

			bytes_ptr ownerPubKey = _parent->OwnerPubKey();
			if (!ownerPubKey->empty()) {
				_depositAddress = Address(PrefixDeposit, *ownerPubKey);
				_ownerAddress = Address(PrefixStandard, *ownerPubKey);
			}

			if (_parent->GetSignType() != Account::MultiSign) {
				HDKeychain mpk = _parent->MasterPubKey();
				_crDepositAddress = Address(PrefixDeposit, mpk.getChild(0).getChild(0).pubkey());
			}

		}

		nlohmann::json SubAccount::GetBasicInfo() const {
			nlohmann::json j;
			j["Account"] = _parent->GetBasicInfo();
			j["CoinIndex"] = _coinIndex;
			return j;
		}

		void SubAccount::Init(const std::vector<TransactionPtr> &tx, Lockable *lock) {
			_lock = lock;

			for (size_t i = 0; i < tx.size(); i++) {
				if (tx[i]->IsSigned()) {
					for (size_t j = 0; j < tx[i]->GetOutputs().size(); ++j)
						AddUsedAddrs(tx[i]->GetOutputs()[j]->Addr());
				}
			}

			UnusedAddresses(SEQUENCE_GAP_LIMIT_EXTERNAL + 100, 0);
			UnusedAddresses(SEQUENCE_GAP_LIMIT_INTERNAL + 100, 1);
		}

		bool SubAccount::IsSingleAddress() const {
			return _parent->SingleAddress();
		}

		bool SubAccount::IsDepositAddress(const Address &address) const {
			if (!_depositAddress.Valid()) {
				return false;
			}

			return _depositAddress == address;
		}

		bool SubAccount::IsOwnerAddress(const Address &address) const {
			if (!_ownerAddress.Valid()) {
				return false;
			}

			return _ownerAddress == address;
		}

		bool SubAccount::IsCRDepositAddress(const Address &address) const {
			if (!_crDepositAddress.Valid()) {
				return false;
			}

			return _crDepositAddress == address;
		}

		void SubAccount::AddUsedAddrs(const Address &address) {
			_usedAddrs.insert(address);
		}

		size_t SubAccount::GetAllAddresses(std::vector<Address> &addr, uint32_t start, size_t count, bool containInternal) const {
			addr.clear();
			if (_parent->GetSignType() == Account::MultiSign) {
				addr.push_back(_parent->GetAddress());
				return 1;
			}

			size_t maxCount = _externalChain.size() + (containInternal ? _internalChain.size() : 0);

			if ((!containInternal && start >= _externalChain.size()) ||
				(containInternal && start >= _externalChain.size() + _internalChain.size())) {
				return maxCount;
			}

			for (size_t i = start; i < _externalChain.size() && addr.size() < count; i++) {
				addr.push_back(_externalChain[i]);
			}

			if (containInternal) {
				for (size_t i = start + addr.size(); addr.size() < count && i < _externalChain.size() + _internalChain.size(); i++) {
					addr.push_back(_internalChain[i - _externalChain.size()]);
				}
			}

			return maxCount;
		}

		std::vector<Address> SubAccount::UnusedAddresses(uint32_t gapLimit, bool internal) {
			std::vector<Address> addrs;
			if (_parent->SingleAddress()) {
				if (_parent->GetSignType() == Account::MultiSign) {
					addrs = {_parent->GetAddress()};
					return addrs;
				}

				if (_externalChain.empty()) {
					bytes_t pubkey = _parent->MasterPubKey().getChild("0/0").pubkey();
					_externalChain.push_back(Address(PrefixStandard, pubkey));
					_allAddrs.insert(_externalChain[0]);
				}
				addrs = _externalChain;
				return addrs;
			}

			size_t i, j = 0, count, startCount;
			uint32_t chain = (internal) ? SEQUENCE_INTERNAL_CHAIN : SEQUENCE_EXTERNAL_CHAIN;

			assert(gapLimit > 0);

			std::vector<Address> &addrChain = internal ? _internalChain : _externalChain;
			i = count = startCount = addrChain.size();

			// keep only the trailing contiguous block of addresses with no transactions
			while (i > 0 && _usedAddrs.find(addrChain[i - 1]) == _usedAddrs.end()) i--;

			HDKeychain mpk = _parent->MasterPubKey();

			while (i + gapLimit > count) { // generate new addresses up to gapLimit
				bytes_t pubKey = mpk.getChild(chain).getChild(count).pubkey();

				Address address(PrefixStandard, pubKey);

				if (!address.Valid()) break;

				addrChain.push_back(address);
				count++;
				if (_usedAddrs.find(address) != _usedAddrs.end()) i = count;
			}

			if (i + gapLimit <= count) {
				for (j = 0; j < gapLimit; j++) {
					addrs.push_back(addrChain[i + j]);
				}
			}

			for (i = startCount; i < count; i++) {
				_allAddrs.insert(addrChain[i]);
			}

			return addrs;
		}

		bytes_ptr SubAccount::OwnerPubKey() const {
			return _parent->OwnerPubKey();
		}

		void SubAccount::SignTransaction(const TransactionPtr &tx, const std::string &payPasswd) {
			std::string addr;
			Key key;
			bytes_t signature;
			ByteStream stream;

			ErrorChecker::CheckParam(tx->IsSigned(), Error::AlreadySigned, "Transaction signed");
			ErrorChecker::CheckParam(tx->GetPrograms().empty(), Error::InvalidTransaction,
			                         "Invalid transaction program");

			uint256 md = tx->GetShaData();

			const std::vector<ProgramPtr> &programs = tx->GetPrograms();
			for (size_t i = 0; i < programs.size(); ++i) {
				std::vector<bytes_t> publicKeys = programs[i]->DecodePublicKey();
				ErrorChecker::CheckLogic(publicKeys.empty(), Error::InvalidRedeemScript, "Invalid redeem script");
				ErrorChecker::CheckLogic(programs[i]->GetPath().empty(), Error::UnSupportOldTx, "Unsupport old tx");

				HDKeychain rootKey = _parent->RootKey(payPasswd);
				key = rootKey.getChild(programs[i]->GetPath());
				bool found = false;
				for (size_t k = 0; k < publicKeys.size(); ++k) {
					if (publicKeys[k] == key.PubKey()) {
						found = true;
						break;
					}
				}
				ErrorChecker::CheckLogic(!found, Error::PrivateKeyNotFound, "Private key not found");

				stream.Reset();
				if (programs[i]->GetParameter().size() > 0) {
					ByteStream verifyStream(programs[i]->GetParameter());
					while (verifyStream.ReadVarBytes(signature)) {
						ErrorChecker::CheckLogic(key.Verify(md, signature), Error::AlreadySigned, "Already signed");
					}
					stream.WriteBytes(programs[i]->GetParameter());
				}

				signature = key.Sign(md);
				stream.WriteVarBytes(signature);
				programs[i]->SetParameter(stream.GetBytes());
			}
		}

		Key SubAccount::DeriveOwnerKey(const std::string &payPasswd) {
			// 44'/coinIndex'/account'/change/index
			return _parent->RootKey(payPasswd).getChild("44'/0'/1'/0/0");
		}

		bool SubAccount::FindKey(Key &key, const bytes_t &pubKey, const std::string &payPasswd) {
			if (*_parent->OwnerPubKey() == pubKey) {
				key = DeriveOwnerKey(payPasswd);
				return true;
			}

			if (_parent->RequestPubKey() == pubKey) {
				key = _parent->RequestPrivKey(payPasswd);
				return true;
			}

			bool found = false;
			Address addr = Address(PrefixStandard, pubKey);
			uint32_t index, change;

			_lock->Lock();

			for (index = (uint32_t) _internalChain.size(); index > 0; --index) {
				if (addr == _internalChain[index - 1]) {
					found = true;
					change = SEQUENCE_INTERNAL_CHAIN;
					break;
				}
			}

			if (!found) {
				for (index = (uint32_t) _externalChain.size(); index > 0; --index) {
					if (addr == _externalChain[index - 1]) {
						found = true;
						change = SEQUENCE_EXTERNAL_CHAIN;
						break;
					}
				}
			}

			_lock->Unlock();

			if (!found)
				return false;

			HDKeychain rootKey = _parent->RootKey(payPasswd);

			HDKeychain masterKey = rootKey.getChild("44'/0'/0'");
			key = masterKey.getChild(change).getChild(index - 1);

			return true;
		}

		bool SubAccount::ContainsAddress(const Address &address) const {
			if (IsDepositAddress(address)) {
				return true;
			}

			if (IsOwnerAddress(address)) {
				return true;
			}

			if (IsCRDepositAddress(address)) {
				return true;
			}

			if (_parent->GetSignType() == Account::MultiSign) {
				return _parent->GetAddress() == address;
			}

			return _allAddrs.find(address) != _allAddrs.end();
		}

		void SubAccount::ClearUsedAddresses() {
			_usedAddrs.clear();
		}

		bool SubAccount::GetCodeAndPath(const Address &addr, bytes_t &code, std::string &path) const {
			uint32_t index;
			bytes_t pubKey;
			if (IsDepositAddress(addr)) {
				code = _depositAddress.RedeemScript();
				path = "44'/0'/1'/0/0";
				return true;
			}

			if (IsOwnerAddress(addr)) {
				code = _ownerAddress.RedeemScript();
				path = "44'/0'/1'/0/0";
				return true;
			}

			if (IsCRDepositAddress(addr)) {
				code = _crDepositAddress.RedeemScript();
				path = "44'/0'/0'/0/0";
				return true;
			}

			if (_parent->GetSignType() == Account::MultiSign) {
				ErrorChecker::CheckLogic(_parent->GetAddress() != addr, Error::Address,
										 "Can't generate code for addr " + addr.String());

				code = _parent->GetAddress().RedeemScript();
				path = "1'/0";
				return true;
			}

			HDKeychain mpk = _parent->MasterPubKey();

			path = "44'/0'/0'/";

			for (index = _internalChain.size(); index > 0; index--) {
				if (_internalChain[index - 1] == addr) {
					pubKey = mpk.getChild(SEQUENCE_INTERNAL_CHAIN).getChild(index - 1).pubkey();

					path = path + "1/" + std::to_string(index - 1);
					break;
				}
			}

			for (index = _externalChain.size(); index > 0 && pubKey.empty(); index--) {
				if (_externalChain[index - 1] == addr) {
					pubKey = mpk.getChild(SEQUENCE_EXTERNAL_CHAIN).getChild(index - 1).pubkey();

					path = path + "0/" + std::to_string(index - 1);
					break;
				}
			}

			if (pubKey.empty()) {
				ErrorChecker::ThrowLogicException(Error::Address, "Can't found code and path for address " + addr.String());
				return false;
			}

			code = Address(PrefixStandard, pubKey).RedeemScript();

			return true;
		}

		size_t SubAccount::InternalChainIndex(const TransactionPtr &tx) const {
			if (_parent->GetSignType() == Account::MultiSign) {
				for (size_t i = 0; i < tx->GetOutputs().size(); ++i) {
					if (tx->GetOutputs()[i]->Addr() == _parent->GetAddress())
						return 0;
				}
			}

			for (size_t i = _internalChain.size(); i > 0; i--) {
				for (size_t j = 0; j < tx->GetOutputs().size(); j++) {
					if (tx->GetOutputs()[j]->Addr() == _internalChain[i - 1])
						return i - 1;
				}
			}

			return -1;
		}

		size_t SubAccount::ExternalChainIndex(const TransactionPtr &tx) const {
			if (_parent->GetSignType() == Account::MultiSign) {
				for (size_t i = 0; i < tx->GetOutputs().size(); ++i) {
					if (tx->GetOutputs()[i]->Addr() == _parent->GetAddress())
						return 0;
				}
			}

			for (size_t i = _externalChain.size(); i > 0; i--) {
				for (size_t j = 0; j < tx->GetOutputs().size(); j++) {
					if (tx->GetOutputs()[j]->Addr() == _externalChain[i - 1])
						return i - 1;
				}
			}

			return -1;
		}

	}
}
