// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "SubAccount.h"

#include <Wallet/Wallet.h>
#include <Common/Utils.h>
#include <Common/Log.h>
#include <Common/ErrorChecker.h>
#include <Plugin/Transaction/Transaction.h>
#include <Plugin/Transaction/TransactionOutput.h>
#include <Plugin/Transaction/Program.h>
#include <Plugin/Transaction/Attribute.h>
#include <WalletCore/Key.h>

namespace Elastos {
	namespace ElaWallet {


		SubAccount::SubAccount(const AccountPtr &parent, uint32_t coinIndex) :
			_parent(parent),
			_coinIndex(coinIndex) {

			if (_parent->GetSignType() != Account::MultiSign) {
				bytes_t ownerPubKey = _parent->OwnerPubKey();
				_depositAddress = Address(PrefixDeposit, ownerPubKey);
				_ownerAddress = Address(PrefixStandard, ownerPubKey);

				HDKeychainPtr mpk = _parent->MasterPubKey();
				_crDepositAddress = Address(PrefixDeposit, mpk->getChild(0).getChild(0).pubkey());
			}
		}

		SubAccount::~SubAccount() {

		}

		nlohmann::json SubAccount::GetBasicInfo() const {
			nlohmann::json j;
			j["Account"] = _parent->GetBasicInfo();
			j["CoinIndex"] = _coinIndex;
			return j;
		}

		void SubAccount::Init(const std::vector<TransactionPtr> &tx) {
			for (size_t i = 0; i < tx.size(); i++) {
				const OutputArray &outputs = tx[i]->GetOutputs();
				for (OutputArray::const_iterator o = outputs.cbegin(); o != outputs.cend(); ++o)
					AddUsedAddrs((*o)->Addr());
			}

			UnusedAddresses(SEQUENCE_GAP_LIMIT_EXTERNAL + 100, 0);
			UnusedAddresses(SEQUENCE_GAP_LIMIT_INTERNAL + 100, 1);
		}

		bool SubAccount::IsSingleAddress() const {
			return _parent->SingleAddress();
		}

		bool SubAccount::IsProducerDepositAddress(const Address &address) const {
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

		size_t SubAccount::GetAllDID(std::vector<Address> &did, uint32_t start, size_t count) const {
			size_t size = 0;

			if (_parent->GetSignType() != IAccount::MultiSign) {
				size = _externalChain.size();
				for (size_t i = start; i < _externalChain.size() && did.size() < count; ++i) {
					Address addr = _externalChain[i];
					addr.ChangePrefix(PrefixIDChain);
					did.push_back(addr);
				}
			}

			return size;
		}

		std::vector<Address> SubAccount::UnusedAddresses(uint32_t gapLimit, bool internal) {
			std::vector<Address> addrs;
			std::vector<bytes_t> pubkeys;
			bytes_t pubkey;

			if (_parent->SingleAddress()) {
				if (_externalChain.empty()) {
					if (_parent->GetSignType() == Account::MultiSign) {
						for (size_t i = 0; i < _parent->MultiSignCosigner().size(); ++i)
							pubkeys.push_back(_parent->MultiSignCosigner()[i]->getChild("0/0").pubkey());
						_externalChain.push_back(Address(PrefixMultiSign, pubkeys, _parent->GetM()));
						_allAddrs.insert(_externalChain[0]);
					} else {
						pubkey = _parent->MasterPubKey()->getChild("0/0").pubkey();
						_externalChain.push_back(Address(PrefixStandard, pubkey));
						_allAddrs.insert(_externalChain[0]);
					}
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

			while (i + gapLimit > count) { // generate new addresses up to gapLimit
				Address address;
				if (_parent->GetSignType() == Account::MultiSign) {
					pubkeys.clear();
					for (size_t i = 0; i < _parent->MultiSignCosigner().size(); ++i)
						pubkeys.push_back(_parent->MultiSignCosigner()[i]->getChild(chain).getChild(count).pubkey());
					address = Address(PrefixMultiSign, pubkeys, _parent->GetM());
				} else {
					HDKeychainPtr mpk = _parent->MasterPubKey();
					pubkey = mpk->getChild(chain).getChild(count).pubkey();
					address = Address(PrefixStandard, pubkey);
				}

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

		bytes_t SubAccount::OwnerPubKey() const {
			return _parent->OwnerPubKey();
		}

		bytes_t SubAccount::DIDPubKey() const {
			return _parent->MasterPubKey()->getChild(0).getChild(0).pubkey();
		}

		void SubAccount::SignTransaction(const TransactionPtr &tx, const std::string &payPasswd) const {
			std::string addr;
			Key key;
			bytes_t signature;
			ByteStream stream;

			ErrorChecker::CheckParam(_parent->Readonly(), Error::Sign, "Readonly wallet can not sign tx");
			ErrorChecker::CheckParam(tx->IsSigned(), Error::AlreadySigned, "Transaction signed");
			ErrorChecker::CheckParam(tx->GetPrograms().empty(), Error::InvalidTransaction,
			                         "Invalid transaction program");

			uint256 md = tx->GetShaData();

			HDKeychainPtr rootKey = _parent->RootKey(payPasswd);
			std::vector<bytes_t> publicKeys;
			const std::vector<ProgramPtr> &programs = tx->GetPrograms();
			for (size_t i = 0; i < programs.size(); ++i) {
				publicKeys.clear();
				SignType type = programs[i]->DecodePublicKey(publicKeys);
				ErrorChecker::CheckLogic(type != SignTypeMultiSign && type != SignTypeStandard, Error::InvalidArgument,
										 "Invalid redeem script");
				ErrorChecker::CheckLogic(programs[i]->GetPath().empty(), Error::UnSupportOldTx, "Unsupport old tx");

				bool found = false;
				if (type == SignTypeStandard) {
					key = rootKey->getChild(programs[i]->GetPath());
					for (size_t k = 0; !found && k < publicKeys.size(); ++k) {
						if (publicKeys[k] == key.PubKey()) {
							found = true;
						}
					}
				} else if (type == SignTypeMultiSign) {
					if (_parent->GetSignType() == Account::MultiSign) {
						if (_parent->DerivationStrategy() == "BIP44")
							key = rootKey->getChild("44'/0'/0'").getChild(programs[i]->GetPath());
						else
							key = rootKey->getChild("45'").getChild((uint32_t) _parent->CosignerIndex()).getChild(programs[i]->GetPath());
						for (size_t k = 0; !found && k < publicKeys.size(); ++k) {
							if (publicKeys[k] == key.PubKey()) {
								found = true;
							}
						}
					} else {
						key = rootKey->getChild("44'/0'/0'").getChild(programs[i]->GetPath());
						for (size_t k = 0; !found && k < publicKeys.size(); ++k) {
							if (publicKeys[k] == key.PubKey()) {
								found = true;
							}
						}
						for (uint32_t idx = 0; !found && idx < MAX_MULTISIGN_COSIGNERS; ++idx) {
							key = rootKey->getChild("45'").getChild(idx).getChild(programs[i]->GetPath());
							for (size_t k = 0; !found && k < publicKeys.size(); ++k) {
								if (publicKeys[k] == key.PubKey()) {
									found = true;
								}
							}
						}
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

		Key SubAccount::GetKeyWithDID(const Address &did, const std::string &payPasswd) const {
			if (_parent->GetSignType() != IAccount::MultiSign) {
				for (size_t i = 0; i < _externalChain.size(); ++i) {
					Address didAddr = _externalChain[i];
					didAddr.ChangePrefix(PrefixIDChain);
					if (did == didAddr) {
						return _parent->RootKey(payPasswd)->getChild("44'/0'/0'/0").getChild(i);
					}
				}
			}

			ErrorChecker::ThrowLogicException(Error::PrivateKeyNotFound, "private key not found");
			return Key();
		}

		Key SubAccount::DeriveOwnerKey(const std::string &payPasswd) {
			// 44'/coinIndex'/account'/change/index
			return _parent->RootKey(payPasswd)->getChild("44'/0'/1'/0/0");
		}

		Key SubAccount::DeriveDIDKey(const std::string &payPasswd) {
			return _parent->RootKey(payPasswd)->getChild("44'/0'/0'/0/0");
		}

		bool SubAccount::FindKey(Key &key, const bytes_t &pubKey, const std::string &payPasswd) {
			if (_parent->OwnerPubKey() == pubKey) {
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

			if (!found)
				return false;

			HDKeychainPtr rootKey = _parent->RootKey(payPasswd);

			HDKeychain masterKey = rootKey->getChild("44'/0'/0'");
			key = masterKey.getChild(change).getChild(index - 1);

			return true;
		}

		bool SubAccount::ContainsAddress(const Address &address) const {
			if (IsProducerDepositAddress(address) || IsCRDepositAddress(address)) {
				return true;
			}

			if (IsOwnerAddress(address)) {
				return true;
			}

			if (_parent->GetSignType() != IAccount::MultiSign) {
				for (std::vector<Address>::const_iterator it = _externalChain.cbegin();
					 it != _externalChain.cend(); ++it) {
					Address did = *it;
					did.ChangePrefix(PrefixIDChain);
					if (did == address)
						return true;
				}
			}

			return _allAddrs.find(address) != _allAddrs.end();
		}

		void SubAccount::ClearUsedAddresses() {
			_usedAddrs.clear();
		}

		size_t SubAccount::GetAllPublickeys(std::vector<bytes_t> &pubkeys, uint32_t start, size_t count,
		                                    bool containInternal) const {
			if (_parent->GetSignType() == Account::MultiSign) {
				return 0;
			}

			size_t maxCount = _externalChain.size() + (containInternal ? _internalChain.size() : 0);

			if ((!containInternal && start >= _externalChain.size()) ||
			    (containInternal && start >= _externalChain.size() + _internalChain.size())) {
				return maxCount;
			}

			pubkeys.clear();

			for (size_t i = start; i < _externalChain.size() && pubkeys.size() < count; i++) {
				ByteStream stream(_externalChain[i].RedeemScript());
				bytes_t pubKey;
				stream.ReadVarBytes(pubKey);
				pubkeys.push_back(pubKey);
			}

			if (containInternal) {
				for (size_t i = start + pubkeys.size(); pubkeys.size() < count && i < maxCount; ++i) {
					ByteStream stream(_internalChain[i - _externalChain.size()].RedeemScript());
					bytes_t pubKey;
					stream.ReadVarBytes(pubKey);
					pubkeys.push_back(pubKey);
				}
			}
			return maxCount;
		}

		bool SubAccount::GetCodeAndPath(const Address &addr, bytes_t &code, std::string &path) const {
			uint32_t index;
			bytes_t pubKey;

			if (IsProducerDepositAddress(addr)) {
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

			if (_parent->GetSignType() != IAccount::MultiSign) {
				for (index = _externalChain.size(); index > 0; index--) {
					Address did = _externalChain[index - 1];
					did.ChangePrefix(PrefixIDChain);
					if (did == addr) {
						code = did.RedeemScript();
						path = "44'/0'/0'/0/" + std::to_string(index - 1);
						return true;
					}
				}
			}

			for (index = _internalChain.size(); index > 0; index--) {
				if (_internalChain[index - 1] == addr) {
					code = _internalChain[index - 1].RedeemScript();
					if (_parent->GetSignType() == Account::MultiSign) {
						path = "1/" + std::to_string(index - 1);
					} else {
						path = "44'/0'/0'/1/" + std::to_string(index - 1);
					}
					return true;
				}
			}

			for (index = _externalChain.size(); index > 0; index--) {
				if (_externalChain[index - 1] == addr) {
					code = _externalChain[index - 1].RedeemScript();
					if (_parent->GetSignType() == Account::MultiSign) {
						path = "0/" + std::to_string(index - 1);
					} else {
						path = "44'/0'/0'/0/" + std::to_string(index - 1);
					}
					return true;
				}
			}

			ErrorChecker::ThrowLogicException(Error::Address, "Can't found code and path for address " + addr.String());

			return false;
		}

		size_t SubAccount::InternalChainIndex(const TransactionPtr &tx) const {
			const OutputArray &outputs = tx->GetOutputs();

			for (size_t i = _internalChain.size(); i > 0; i--) {
				for (OutputArray::const_iterator o = outputs.cbegin(); o != outputs.cend(); ++o) {
					if ((*o)->Addr() == _internalChain[i - 1])
						return i - 1;
				}
			}

			return -1;
		}

		size_t SubAccount::ExternalChainIndex(const TransactionPtr &tx) const {
			const OutputArray &outputs = tx->GetOutputs();

			for (size_t i = _externalChain.size(); i > 0; i--) {
				for (OutputArray::const_iterator o = outputs.cbegin(); o != outputs.cend(); ++o) {
					if ((*o)->Addr() == _externalChain[i - 1])
						return i - 1;
				}
			}

			return -1;
		}

		AccountPtr SubAccount::Parent() const {
			return _parent;
		}

	}
}
