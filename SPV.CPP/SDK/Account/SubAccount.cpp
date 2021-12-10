/*
 * Copyright (c) 2019 Elastos Foundation
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */
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


		SubAccount::SubAccount(const AccountPtr &parent) :
			_parent(parent) {

            _chainAddressCached[SEQUENCE_EXTERNAL_CHAIN] = {};
            _chainAddressCached[SEQUENCE_INTERNAL_CHAIN] = {};

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
			return j;
		}

		bool SubAccount::IsSingleAddress() const {
			return _parent->SingleAddress();
		}

		bool SubAccount::IsProducerDepositAddress(const Address &address) const {
			return _depositAddress.Valid() && _depositAddress == address;
		}

		bool SubAccount::IsOwnerAddress(const Address &address) const {
			return _ownerAddress.Valid() && _ownerAddress == address;
		}

		bool SubAccount::IsCRDepositAddress(const Address &address) const {
			return _crDepositAddress.Valid() && _crDepositAddress == address;
		}

		void SubAccount::GetCID(AddressArray &cids, uint32_t index, size_t count, bool internal) const {
			if (_parent->GetSignType() != IAccount::MultiSign) {
			    AddressArray addresses;
			    GetAddresses(addresses, index, count, internal);

                for (Address addr : addresses) {
                    Address cid(addr);
                    cid.ChangePrefix(PrefixIDChain);
                    cids.push_back(cid);
                }
			}
		}

        void SubAccount::GetPublickeys(nlohmann::json &jout, uint32_t index, size_t count, bool internal) const {
            if (_parent->SingleAddress()) {
                index = 0;
                count = 1;
                internal = false;
            }

            uint32_t chain = internal ? SEQUENCE_INTERNAL_CHAIN : SEQUENCE_EXTERNAL_CHAIN;
            if (_parent->GetSignType() == Account::MultiSign) {
                std::vector<HDKeychain> allKeychains;
                HDKeychain mineKeychain;

                if (count > 0) {
                    for (const HDKeychainPtr &keychain : _parent->MultiSignCosigner())
                        allKeychains.push_back(keychain->getChild(chain));
                    mineKeychain = _parent->MultiSignSigner()->getChild(chain);
                }

                if (allKeychains.empty()) {
                    count = 0;
                    Log::error("keychains is empty when derivate address");
                }

                jout["m"] = _parent->GetM();
                nlohmann::json jpubkeys;
                while (count--) {
                    std::vector<std::string> pubkeys;
                    nlohmann::json j;
                    for (const HDKeychain &signer : allKeychains)
                        pubkeys.push_back(signer.getChild(index).pubkey().getHex());

                    j["me"] = mineKeychain.getChild(index).pubkey().getHex();
                    j["all"] = pubkeys;
                    jpubkeys.push_back(j);
                    index++;
                }
                jout["pubkeys"] = jpubkeys;
            } else {
                HDKeychain keychain = _parent->MasterPubKey()->getChild(chain);

                while (count--)
                    jout.push_back(keychain.getChild(index++).pubkey().getHex());
            }
        }

        void SubAccount::GetAddresses(AddressArray &addresses, uint32_t index, uint32_t count, bool internal) const {
            if (_parent->SingleAddress()) {
                index = 0;
                count = 1;
                internal = false;
            }

            uint32_t chain = internal ? SEQUENCE_INTERNAL_CHAIN : SEQUENCE_EXTERNAL_CHAIN;
            AddressArray &addrChain = _chainAddressCached[chain];
            uint32_t derivateCount = (index + count > addrChain.size()) ? (index + count - addrChain.size()) : 0;

            if (_parent->GetSignType() == Account::MultiSign) {
                std::vector<HDKeychain> keychains;

                if (derivateCount > 0)
                    for (const HDKeychainPtr &keychain : _parent->MultiSignCosigner())
                        keychains.push_back(keychain->getChild(chain));

                if (keychains.empty()) {
                    derivateCount = 0;
                    Log::error("keychains is empty when derivate address");
                }

                while (derivateCount--) {
                    std::vector<bytes_t> pubkeys;
                    for (const HDKeychain &signer : keychains)
                        pubkeys.push_back(signer.getChild(addrChain.size()).pubkey());

                    Address addr(PrefixMultiSign, pubkeys, _parent->GetM());
                    if (!addr.Valid()) {
                        Log::error("derivate invalid multi-sig address");
                        break;
                    }
                    addrChain.push_back(addr);
                }
            } else {
                HDKeychain keychain = _parent->MasterPubKey()->getChild(chain);

                while (derivateCount--) {
                    Address addr(PrefixStandard, keychain.getChild(addrChain.size()).pubkey());
                    if (!addr.Valid()) {
                        Log::error("derivate invalid address");
                        break;
                    }
                    addrChain.push_back(addr);
                }
            }
            addresses.assign(addrChain.begin() + index, addrChain.begin() + index + count);
		}

		bytes_t SubAccount::OwnerPubKey() const {
			return _parent->OwnerPubKey();
		}

		bytes_t SubAccount::DIDPubKey() const {
			return _parent->MasterPubKey()->getChild(0).getChild(0).pubkey();
		}

        bool SubAccount::FindPrivateKey(Key &key, SignType type, const std::vector<bytes_t> &pubkeys, const std::string &payPasswd) const {
            HDKeychainPtr root = _parent->RootKey(payPasswd);
            // for special path
            if (_parent->GetSignType() != Account::MultiSign) {
                for (const bytes_t &pubkey : pubkeys) {
                    if (pubkey == _parent->OwnerPubKey()) {
                        key = root->getChild("44'/0'/1'/0/0");
                        return true;
                    }
                }
            }

		    std::vector<HDKeychain> bipkeys;
		    bipkeys.push_back(root->getChild("44'/0'/0'"));
		    if (type == SignTypeMultiSign) {
                HDKeychain bip45 = root->getChild("45'");
		        if (_parent->GetSignType() == Account::MultiSign) {
                    bipkeys.push_back(bip45.getChild(_parent->CosignerIndex()));
		        } else {
                    for (uint32_t index = 0; index < MAX_MULTISIGN_COSIGNERS; ++index)
                        bipkeys.push_back(bip45.getChild(index));
		        }
		    }

		    for (HDKeychain &bipkey : bipkeys) {
                for (auto & it : _chainAddressCached) {
                    uint32_t chain = it.first;
                    HDKeychain bipkeyChain = bipkey.getChild(chain);
                    for (uint32_t index = it.second.size(); index > 0; index--) {
                        HDKeychain bipkeyIndex = bipkeyChain.getChild(index - 1);
                        for (const bytes_t &p : pubkeys) {
                            if (bipkeyIndex.pubkey() == p) {
                                key = bipkeyIndex;
                                return true;
                            }
                        }
                    }
                }
            }

		    return false;
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

			const std::vector<ProgramPtr> &programs = tx->GetPrograms();
			for (size_t i = 0; i < programs.size(); ++i) {
                std::vector<bytes_t> publicKeys;
				SignType type = programs[i]->DecodePublicKey(publicKeys);
				ErrorChecker::CheckLogic(type != SignTypeMultiSign && type != SignTypeStandard, Error::InvalidArgument,
										 "Invalid redeem script");

				bool found = FindPrivateKey(key, type, publicKeys, payPasswd);
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

		Key SubAccount::GetKeyWithAddress(const Address &addr, const std::string &payPasswd) const {
			if (_parent->GetSignType() != IAccount::MultiSign) {
			    for (auto it = _chainAddressCached.begin(); it != _chainAddressCached.end(); ++it) {
			        uint32_t chain = it->first;
			        AddressArray &chainAddr = it->second;
                    for (uint32_t i = 0; i < chainAddr.size(); ++i) {
                        Address cid(chainAddr[i]);
                        cid.ChangePrefix(PrefixIDChain);

                        Address did(cid);
                        did.ConvertToDID();

                        if (addr == chainAddr[i] || addr == cid || addr == did) {
                            return _parent->RootKey(payPasswd)->getChild("44'/0'/0'").getChild(chain).getChild(i);
                        }
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

		bool SubAccount::GetCode(const Address &addr, bytes_t &code) const {
			uint32_t index;
			bytes_t pubKey;

			if (IsProducerDepositAddress(addr)) {
                // "44'/0'/1'/0/0";
				code = _depositAddress.RedeemScript();
				return true;
			}

			if (IsOwnerAddress(addr)) {
                // "44'/0'/1'/0/0";
				code = _ownerAddress.RedeemScript();
				return true;
			}

			if (IsCRDepositAddress(addr)) {
                // "44'/0'/0'/0/0";
				code = _crDepositAddress.RedeemScript();
				return true;
			}

			for (auto it = _chainAddressCached.begin(); it != _chainAddressCached.end(); ++it) {
			    AddressArray &chainAddr = it->second;
                for (index = chainAddr.size(); index > 0; index--) {
                    if (chainAddr[index - 1] == addr) {
                        code = chainAddr[index - 1].RedeemScript();
                        return true;
                    }

                    if (_parent->GetSignType() != IAccount::MultiSign) {
                        Address cid(chainAddr[index - 1]);
                        cid.ChangePrefix(PrefixIDChain);
                        if (addr == cid) {
                            code = cid.RedeemScript();
                            return true;
                        }

                        Address did(cid);
                        did.ConvertToDID();
                        if (addr == did) {
                            code = did.RedeemScript();
                            return true;
                        }
                    }
                }
            }

			Log::error("Can't found code and path for address {}", addr.String());

			return false;
		}

		AccountPtr SubAccount::Parent() const {
			return _parent;
		}

	}
}
