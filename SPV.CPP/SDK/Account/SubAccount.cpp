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
				_depositAddress = AddressPtr(new Address(PrefixDeposit, ownerPubKey));
				_ownerAddress = AddressPtr(new Address(PrefixStandard, ownerPubKey));

				HDKeychainPtr mpk = _parent->MasterPubKey();
				_crDepositAddress = AddressPtr(new Address(PrefixDeposit, mpk->getChild(0).getChild(0).pubkey()));
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

		void SubAccount::Init() {
			UnusedAddresses(SEQUENCE_GAP_LIMIT_EXTERNAL + 100, 0);
			UnusedAddresses(SEQUENCE_GAP_LIMIT_INTERNAL + 100, 1);
		}

		void SubAccount::InitDID() {
			if (_parent->GetSignType() != IAccount::MultiSign) {
				for (AddressArray::iterator it = _externalChain.begin(); it != _externalChain.end(); ++it) {
					AddressPtr did(new Address(**it));
					did->ChangePrefix(PrefixIDChain);
					_did.push_back(did);
					_allDID.insert(did);
				}
			}
		}

		bool SubAccount::IsSingleAddress() const {
			return _parent->SingleAddress();
		}

		bool SubAccount::IsProducerDepositAddress(const AddressPtr &address) const {
			return _depositAddress && _depositAddress->Valid() && *_depositAddress == *address;
		}

		bool SubAccount::IsOwnerAddress(const AddressPtr &address) const {
			return _ownerAddress && _ownerAddress->Valid() && *_ownerAddress == *address;
		}

		bool SubAccount::IsCRDepositAddress(const AddressPtr &address) const {
			return _crDepositAddress && _crDepositAddress->Valid() && *_crDepositAddress == *address;
		}

		bool SubAccount::AddUsedAddrs(const AddressPtr &address) {
			if (_allAddrs.find(address) != _allAddrs.end()) {
				_usedAddrs.insert(address);
				return true;
			}
			return false;
		}

		size_t SubAccount::GetAllAddresses(AddressArray &addr, uint32_t start, size_t count, bool internal) const {
			addr.clear();
			size_t maxCount = 0;

			if (internal) {
				maxCount = _internalChain.size();

				for (size_t i = start, cnt = 0; i < _internalChain.size() && cnt < count; ++i, ++cnt)
					addr.push_back(_internalChain[i]);
			} else {
				maxCount = _externalChain.size();

				for (size_t i = start, cnt = 0; i < _externalChain.size() && cnt < count; ++i, ++cnt)
					addr.push_back(_externalChain[i]);

			}

			return maxCount;
		}

		size_t SubAccount::GetAllDID(AddressArray &did, uint32_t start, size_t count) const {
			size_t maxCount = 0;

			if (_parent->GetSignType() != IAccount::MultiSign) {
				maxCount = _did.size();
				for (size_t i = start, cnt = 0; i < maxCount && cnt < count; ++i, ++cnt) {
					did.push_back(_did[i]);
				}
			}

			return maxCount;
		}

		AddressArray SubAccount::UnusedAddresses(uint32_t gapLimit, bool internal) {
			AddressArray addrs;
			std::vector<bytes_t> pubkeys;
			bytes_t pubkey;

			if (_parent->SingleAddress()) {
				if (_externalChain.empty()) {
					if (_parent->GetSignType() == Account::MultiSign) {
						for (size_t i = 0; i < _parent->MultiSignCosigner().size(); ++i)
							pubkeys.push_back(_parent->MultiSignCosigner()[i]->getChild("0/0").pubkey());
						_externalChain.push_back(AddressPtr(new Address(PrefixMultiSign, pubkeys, _parent->GetM())));
						_allAddrs.insert(_externalChain[0]);
					} else {
						pubkey = _parent->MasterPubKey()->getChild("0/0").pubkey();
						_externalChain.push_back(AddressPtr(new Address(PrefixStandard, pubkey)));
						_allAddrs.insert(_externalChain[0]);
					}
				}
				addrs = _externalChain;
				return addrs;
			}

			size_t i, j = 0, count, startCount;
			uint32_t chain = (internal) ? SEQUENCE_INTERNAL_CHAIN : SEQUENCE_EXTERNAL_CHAIN;

			assert(gapLimit > 0);

			AddressArray &addrChain = internal ? _internalChain : _externalChain;
			std::vector<HDKeychain> keychains;
			if (_parent->GetSignType() == Account::MultiSign) {
				for (const HDKeychainPtr &keychain : _parent->MultiSignCosigner())
					keychains.push_back(keychain->getChild(chain));
			} else {
				keychains.push_back(_parent->MasterPubKey()->getChild(chain));
			}

			i = count = startCount = addrChain.size();

			// keep only the trailing contiguous block of addresses with no transactions
			while (i > 0 && _usedAddrs.find(addrChain[i - 1]) == _usedAddrs.end()) i--;

			while (i + gapLimit > count) { // generate new addresses up to gapLimit
				AddressPtr address;
				if (_parent->GetSignType() == Account::MultiSign) {
					pubkeys.clear();
					for (const HDKeychain &signer : keychains) {
						pubkeys.push_back(signer.getChild(count).pubkey());
					}
					address = AddressPtr(new Address(PrefixMultiSign, pubkeys, _parent->GetM()));
				} else {
					pubkey = keychains[0].getChild(count).pubkey();
					address = AddressPtr(new Address(PrefixStandard, pubkey));
				}

				if (!address->Valid()) break;
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

		Key SubAccount::GetKeyWithDID(const AddressPtr &did, const std::string &payPasswd) const {
			if (_parent->GetSignType() != IAccount::MultiSign) {
				for (size_t i = 0; i < _did.size(); ++i) {
					if (*did == *_did[i]) {
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

		bool SubAccount::ContainsAddress(const AddressPtr &address) const {
			if (IsProducerDepositAddress(address) || IsCRDepositAddress(address)) {
				return true;
			}

			if (IsOwnerAddress(address)) {
				return true;
			}

			if (_parent->GetSignType() != IAccount::MultiSign) {
				if (_allDID.find(address) != _allDID.end())
					return true;
			}

			return _allAddrs.find(address) != _allAddrs.end();
		}

		size_t SubAccount::GetAllPublickeys(std::vector<bytes_t> &pubkeys, uint32_t start, size_t count,
		                                    bool containInternal) const {

			pubkeys.clear();
			if (_parent->GetSignType() == Account::MultiSign)
				return 0;

			AddressArray allAddress;
			allAddress = _externalChain;
			if (containInternal)
				allAddress.insert(allAddress.end(), _internalChain.begin(), _internalChain.end());

			size_t maxCount = allAddress.size();
			bytes_t pubkey;

			for (size_t i = start, cnt = 0; i < maxCount && cnt < count; ++i, ++cnt) {
				ByteStream stream(allAddress[i]->RedeemScript());
				stream.ReadVarBytes(pubkey);
				pubkeys.push_back(pubkey);
			}

			return maxCount;
		}

		bool SubAccount::GetCodeAndPath(const AddressPtr &addr, bytes_t &code, std::string &path) const {
			uint32_t index;
			bytes_t pubKey;

			if (IsProducerDepositAddress(addr)) {
				code = _depositAddress->RedeemScript();
				path = "44'/0'/1'/0/0";
				return true;
			}

			if (IsOwnerAddress(addr)) {
				code = _ownerAddress->RedeemScript();
				path = "44'/0'/1'/0/0";
				return true;
			}

			if (IsCRDepositAddress(addr)) {
				code = _crDepositAddress->RedeemScript();
				path = "44'/0'/0'/0/0";
				return true;
			}

			if (_parent->GetSignType() != IAccount::MultiSign) {
				for (index = _did.size(); index > 0; index--) {
					if (*_did[index - 1] == *addr) {
						code = _did[index - 1]->RedeemScript();
						path = "44'/0'/0'/0/" + std::to_string(index - 1);
						return true;
					}
				}
			}

			for (index = _internalChain.size(); index > 0; index--) {
				if (*_internalChain[index - 1] == *addr) {
					code = _internalChain[index - 1]->RedeemScript();
					if (_parent->GetSignType() == Account::MultiSign) {
						path = "1/" + std::to_string(index - 1);
					} else {
						path = "44'/0'/0'/1/" + std::to_string(index - 1);
					}
					return true;
				}
			}

			for (index = _externalChain.size(); index > 0; index--) {
				if (*_externalChain[index - 1] == *addr) {
					code = _externalChain[index - 1]->RedeemScript();
					if (_parent->GetSignType() == Account::MultiSign) {
						path = "0/" + std::to_string(index - 1);
					} else {
						path = "44'/0'/0'/0/" + std::to_string(index - 1);
					}
					return true;
				}
			}

			ErrorChecker::ThrowLogicException(Error::Address, "Can't found code and path for address " + addr->String());

			return false;
		}

		size_t SubAccount::InternalChainIndex(const TransactionPtr &tx) const {
			const OutputArray &outputs = tx->GetOutputs();

			for (size_t i = _internalChain.size(); i > 0; i--) {
				for (OutputArray::const_iterator o = outputs.cbegin(); o != outputs.cend(); ++o) {
					if (*(*o)->Addr() == *_internalChain[i - 1])
						return i - 1;
				}
			}

			return -1;
		}

		size_t SubAccount::ExternalChainIndex(const TransactionPtr &tx) const {
			const OutputArray &outputs = tx->GetOutputs();

			for (size_t i = _externalChain.size(); i > 0; i--) {
				for (OutputArray::const_iterator o = outputs.cbegin(); o != outputs.cend(); ++o) {
					if (*(*o)->Addr() == *_externalChain[i - 1])
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
