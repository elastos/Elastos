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
#include "Account.h"

#include <Common/Utils.h>
#include <Common/Log.h>
#include <Common/ErrorChecker.h>
#include <Common/ByteStream.h>
#include <WalletCore/Base58.h>
#include <WalletCore/AES.h>
#include <WalletCore/Mnemonic.h>
#include <WalletCore/CoinInfo.h>
#include <WalletCore/HDKeychain.h>
#include <WalletCore/Key.h>
#include <support/BRInt.h>
#include <ethereum/ewm/BREthereumAccount.h>

namespace Elastos {
	namespace ElaWallet {

		void Account::Init() const {
			_cosignerIndex = -1;

			if (!_localstore->GetOwnerPubKey().empty())
                _ownerPubKey.setHex(_localstore->GetOwnerPubKey());
			if (!_localstore->GetRequestPubKey().empty())
                _requestPubKey.setHex(_localstore->GetRequestPubKey());

			bytes_t bytes;

			const std::vector<PublicKeyRing> &xpubRing = _localstore->GetPublicKeyRing();
			for (int i = 0; i < (int)xpubRing.size() - 1; ++i) {
				for (int j = i + 1; j < xpubRing.size(); ++j) {
					if (xpubRing[i].GetxPubKey() == xpubRing[j].GetxPubKey()) {
						ErrorChecker::ThrowParamException(Error::PubKeyFormat, "Contain same xpub in PublicKeyRing");
					}
				}
			}

            if (!_localstore->GetxPubKey().empty()) {
                ErrorChecker::CheckParam(!Base58::CheckDecode(_localstore->GetxPubKey(), bytes),
                                         Error::PubKeyFormat, "xpub decode error");
                _xpub = HDKeychainPtr(new HDKeychain(CTElastos, bytes));
            } else {
                _xpub = nullptr;
                Log::warn("xpub is empty");
            }

            if (!_localstore->GetxPubKeyBitcoin().empty()) {
                ErrorChecker::CheckParam(!Base58::CheckDecode(_localstore->GetxPubKeyBitcoin(), bytes),
                                         Error::PubKeyFormat, "xpubkeyBitcoin decode error");
                _btcMasterPubKey = HDKeychainPtr(new HDKeychain(CTBitcoin, bytes));
            } else {
                _btcMasterPubKey = nullptr;
                Log::warn("btcMasterPubKey is empty");
            }

            if (!_localstore->GetxPubKeyHDPM().empty()) {
                ErrorChecker::CheckParam(!Base58::CheckDecode(_localstore->GetxPubKeyHDPM(), bytes),
                                         Error::PubKeyFormat, "xpubHDPM decode error");
                _curMultiSigner = HDKeychainPtr(new HDKeychain(CTElastos, bytes));
            } else {
                Log::warn("xpubHDPM is empty");
            }


			if (_localstore->GetN() > 1) {
				if (_localstore->DerivationStrategy() == "BIP44") {
					_curMultiSigner = _xpub;
					for (size_t i = 0; i < _localstore->GetPublicKeyRing().size(); ++i) {
						bytes_t xpubBytes;
						ErrorChecker::CheckParam(!Base58::CheckDecode(_localstore->GetPublicKeyRing()[i].GetxPubKey(), xpubBytes),
												 Error::PubKeyFormat, "xpub decode error");
						HDKeychainPtr xpub(new HDKeychain(CTElastos, xpubBytes));
						_allMultiSigners.push_back(xpub);
					}
				} else if (_localstore->DerivationStrategy() == "BIP45") {
					HDKeychainArray sortedSigners;
					for (size_t i = 0; i < _localstore->GetPublicKeyRing().size(); ++i) {
						ErrorChecker::CheckLogic(
							!Base58::CheckDecode(_localstore->GetPublicKeyRing()[i].GetxPubKey(), bytes),
							Error::PubKeyFormat, "xpub HDPM decode error");
						HDKeychainPtr xpub(new HDKeychain(CTElastos, bytes));
						sortedSigners.push_back(xpub);
					}

					std::sort(sortedSigners.begin(), sortedSigners.end(),
							  [](const HDKeychainPtr &a, const HDKeychainPtr &b) {
								  return a->pubkey().getHex() < b->pubkey().getHex();
							  });

					for (size_t i = 0; i < sortedSigners.size(); ++i) {
						HDKeychainPtr tmp(new HDKeychain(sortedSigners[i]->getChild((uint32_t) i)));
						if (_curMultiSigner && _cosignerIndex == -1 && *_curMultiSigner == *sortedSigners[i]) {
							_curMultiSigner = tmp;
							_cosignerIndex = (int)i;
						}
						_allMultiSigners.push_back(tmp);
					}
				}
			}
		}

		Account::~Account() {

		}

		Account::Account(const LocalStorePtr &store) :
			_localstore(store) {
			Init();
		}

		Account::Account(const std::string &path) {
			_localstore = LocalStorePtr(new LocalStore(path));
			_localstore->Load();
			Init();
		}

		Account::Account(const std::string &path, const std::vector<PublicKeyRing> &cosigners, int m,
						 bool singleAddress, bool compatible) {
			ErrorChecker::CheckParam(cosigners.size() > MAX_MULTISIGN_COSIGNERS, Error::MultiSign, "Too much signers");

			_localstore = LocalStorePtr(new LocalStore(path));
			_localstore->SetM(m);
			_localstore->SetN((int)cosigners.size());
			_localstore->SetSingleAddress(singleAddress);
			_localstore->SetReadonly(true);
			_localstore->SetHasPassPhrase(false);
			_localstore->SetPublicKeyRing(cosigners);
			_localstore->SetMnemonic("");
			_localstore->SetxPrivKey("");
			_localstore->SetxPubKey("");
			_localstore->SetxPubKeyHDPM("");
			_localstore->SetRequestPubKey("");
			_localstore->SetRequestPrivKey("");
			_localstore->SetOwnerPubKey("");
			_localstore->SetSeed("");
			_localstore->SetETHSCPrimaryPubKey("");
            _localstore->SetxPubKeyBitcoin("");
            _localstore->SetSinglePrivateKey("");
            _localstore->SetRipplePrimaryPubKey("");

			if (compatible) {
				_localstore->SetDerivationStrategy("BIP44");
			} else {
				_localstore->SetDerivationStrategy("BIP45");
			}

			Init();
		}

		Account::Account(const std::string &path, const std::string &xprv, const std::string &payPasswd,
						 const std::vector<PublicKeyRing> &cosigners, int m, bool singleAddress, bool compatible) {

			ErrorChecker::CheckParam(cosigners.size() + 1 > MAX_MULTISIGN_COSIGNERS, Error::MultiSign,
									 "Too much signers");
			bytes_t bytes;
			ErrorChecker::CheckLogic(!Base58::CheckDecode(xprv, bytes), Error::InvalidArgument, "Invalid xprv");

			HDKeychain rootkey(CTElastos, bytes);

			std::string encryptedxPrvKey = AES::EncryptCCM(bytes, payPasswd);
			std::string xPubKey = Base58::CheckEncode(rootkey.getChild("44'/0'/0'").getPublic().extkey());

			HDKeychain requestKey = rootkey.getChild("1'/0");
			std::string encryptedRequestPrvKey = AES::EncryptCCM(requestKey.privkey(), payPasswd);
			std::string requestPubKey = requestKey.pubkey().getHex();

			_localstore = LocalStorePtr(new LocalStore(path));
			_localstore->SetM(m);
			_localstore->SetN((int)cosigners.size() + 1);
			_localstore->SetSingleAddress(singleAddress);
			_localstore->SetReadonly(false);
			_localstore->SetHasPassPhrase(false);
			_localstore->SetPublicKeyRing(cosigners);
			_localstore->SetMnemonic("");
			_localstore->SetxPrivKey(encryptedxPrvKey);
			_localstore->SetxPubKey(xPubKey);
			_localstore->SetRequestPubKey(requestPubKey);
			_localstore->SetRequestPrivKey(encryptedRequestPrvKey);
			_localstore->SetOwnerPubKey("");
			_localstore->SetSeed("");
			_localstore->SetETHSCPrimaryPubKey("");
			_localstore->SetRipplePrimaryPubKey("");
            _localstore->SetxPubKeyBitcoin("");
            _localstore->SetSinglePrivateKey("");

			if (compatible) {
				_localstore->SetDerivationStrategy("BIP44");
				_localstore->AddPublicKeyRing(PublicKeyRing("", xPubKey));
				_localstore->SetxPubKeyHDPM(xPubKey);
			} else {
				_localstore->SetDerivationStrategy("BIP45");
				std::string xpubPurpose = Base58::CheckEncode(rootkey.getChild("45'").getPublic().extkey());
				_localstore->AddPublicKeyRing(PublicKeyRing(requestPubKey, xpubPurpose));
				_localstore->SetxPubKeyHDPM(xpubPurpose);
			}

			Init();
		}

		Account::Account(const std::string &path, const std::string &mnemonic, const std::string &passphrase,
						 const std::string &payPasswd, const std::vector<PublicKeyRing> &cosigners, int m,
						 bool singleAddress, bool compatible) {
			ErrorChecker::CheckParam(cosigners.size() + 1 > MAX_MULTISIGN_COSIGNERS, Error::MultiSign,
									 "Too much signers");

			bytes_t xprv;
			uint512 seed = Mnemonic::DeriveSeed(mnemonic, passphrase);
			HDSeed hdseed(seed.bytes());
			HDKeychain rootkey(CTElastos, hdseed.getExtendedKey(CTElastos, true));
            HDKeychain stdrootkey(CTBitcoin, hdseed.getExtendedKey(CTBitcoin, true));
            HDKeychain ethkey = stdrootkey.getChild("44'/60'/0'/0/0");

			std::string encryptedSeed = AES::EncryptCCM(bytes_t(seed.begin(), seed.size()), payPasswd);
            std::string encryptedethPrvKey = AES::EncryptCCM(ethkey.privkey(), payPasswd);
            std::string ethscPubKey = ethkey.uncompressed_pubkey().getHex();
            std::string ripplePubKey = stdrootkey.getChild("44'/144'/0'/0/0").pubkey().getHex();
			std::string encryptedMnemonic = AES::EncryptCCM(bytes_t(mnemonic.data(), mnemonic.size()), payPasswd);
			std::string encryptedxPrvKey = AES::EncryptCCM(rootkey.extkey(), payPasswd);
			std::string xPubKey = Base58::CheckEncode(rootkey.getChild("44'/0'/0'").getPublic().extkey());
            std::string xpubBitcoin = Base58::CheckEncode(stdrootkey.getChild("44'/0'/0'").getPublic().extkey());

			HDKeychain requestKey = rootkey.getChild("1'/0");
			std::string encryptedRequestPrvKey = AES::EncryptCCM(requestKey.privkey(), payPasswd);
			std::string requestPubKey = requestKey.pubkey().getHex();

			_localstore = LocalStorePtr(new LocalStore(path));
			_localstore->SetM(m);
			_localstore->SetN((int)cosigners.size() + 1);
			_localstore->SetSingleAddress(singleAddress);
			_localstore->SetReadonly(false);
			_localstore->SetHasPassPhrase(!passphrase.empty());
			_localstore->SetPublicKeyRing(cosigners);
			_localstore->SetMnemonic(encryptedMnemonic);
			_localstore->SetxPrivKey(encryptedxPrvKey);
			_localstore->SetxPubKey(xPubKey);
			_localstore->SetRequestPubKey(requestPubKey);
			_localstore->SetRequestPrivKey(encryptedRequestPrvKey);
			_localstore->SetOwnerPubKey("");
			_localstore->SetSeed(encryptedSeed);
			_localstore->SetETHSCPrimaryPubKey(ethscPubKey);
            _localstore->SetxPubKeyBitcoin(xpubBitcoin);
            _localstore->SetSinglePrivateKey(encryptedethPrvKey);
            _localstore->SetRipplePrimaryPubKey(ripplePubKey);

			if (compatible) {
				_localstore->SetDerivationStrategy("BIP44");
				_localstore->AddPublicKeyRing(PublicKeyRing("", xPubKey));
				_localstore->SetxPubKeyHDPM(xPubKey);
			} else {
				_localstore->SetDerivationStrategy("BIP45");
				std::string xpubPurpose = Base58::CheckEncode(rootkey.getChild("45'").getPublic().extkey());
				_localstore->AddPublicKeyRing(PublicKeyRing(requestPubKey, xpubPurpose));
				_localstore->SetxPubKeyHDPM(xpubPurpose);
			}

			Init();
		}

		Account::Account(const std::string &path, const std::string &mnemonic, const std::string &passphrase,
						 const std::string &payPasswd, bool singleAddress) {
			uint512 seed = Mnemonic::DeriveSeed(mnemonic, passphrase);
            HDSeed hdseed(seed.bytes());
			HDKeychain rootkey(CTElastos, hdseed.getExtendedKey(CTElastos, true));
            HDKeychain stdrootkey(CTBitcoin, hdseed.getExtendedKey(CTBitcoin, true));
            HDKeychain ethkey = stdrootkey.getChild("44'/60'/0'/0/0");

			std::string encryptedSeed = AES::EncryptCCM(bytes_t(seed.begin(), seed.size()), payPasswd);
			std::string encryptedethPrvKey = AES::EncryptCCM(ethkey.privkey(), payPasswd);
            std::string ethscPubKey = ethkey.uncompressed_pubkey().getHex();
            std::string ripplePubKey = stdrootkey.getChild("44'/144'/0'/0/0").pubkey().getHex();
			std::string encryptedMnemonic = AES::EncryptCCM(bytes_t(mnemonic.data(), mnemonic.size()), payPasswd);
			std::string encryptedxPrvKey = AES::EncryptCCM(rootkey.extkey(), payPasswd);

            std::string xpubBitcoin = Base58::CheckEncode(stdrootkey.getChild("44'/0'/0'").getPublic().extkey());
			std::string xPubKey = Base58::CheckEncode(rootkey.getChild("44'/0'/0'").getPublic().extkey());
			std::string xpubHDPM = Base58::CheckEncode(rootkey.getChild("45'").getPublic().extkey());

			HDKeychain requestKey = rootkey.getChild("1'/0");
			std::string encryptedRequestPrvKey = AES::EncryptCCM(requestKey.privkey(), payPasswd);
			std::string requestPubKey = requestKey.pubkey().getHex();

			std::string ownerPubKey = rootkey.getChild("44'/0'/1'/0/0").pubkey().getHex();

			_localstore = LocalStorePtr(new LocalStore(path));
			_localstore->SetDerivationStrategy("BIP44");
			_localstore->SetM(1);
			_localstore->SetN(1);
			_localstore->SetSingleAddress(singleAddress);
			_localstore->SetReadonly(false);
			_localstore->SetHasPassPhrase(!passphrase.empty());
			_localstore->SetPublicKeyRing({PublicKeyRing(requestPubKey, xpubHDPM)});
			_localstore->SetMnemonic(encryptedMnemonic);
			_localstore->SetxPrivKey(encryptedxPrvKey);
			_localstore->SetxPubKey(xPubKey);
			_localstore->SetxPubKeyHDPM(xpubHDPM);
			_localstore->SetRequestPubKey(requestPubKey);
			_localstore->SetRequestPrivKey(encryptedRequestPrvKey);
			_localstore->SetOwnerPubKey(ownerPubKey);
			_localstore->SetSeed(encryptedSeed);
			_localstore->SetETHSCPrimaryPubKey(ethscPubKey);
            _localstore->SetxPubKeyBitcoin(xpubBitcoin);
            _localstore->SetSinglePrivateKey(encryptedethPrvKey);
            _localstore->SetRipplePrimaryPubKey(ripplePubKey);

			Init();
		}

        Account::Account(const std::string &path, const std::string singlePrivateKey, const std::string &passwd) {
		    bytes_t singlePrvKey;
		    singlePrvKey.setHex(singlePrivateKey);

		    std::string encryptedSinglePrvKey = AES::EncryptCCM(singlePrvKey, passwd);
		    Key k(CTBitcoin, singlePrvKey);
		    std::string ethscPubKey = k.PubKey(false).getHex();

            _localstore = LocalStorePtr(new LocalStore(path));
            _localstore->SetDerivationStrategy("BIP44");
            _localstore->SetM(1);
            _localstore->SetN(1);
            _localstore->SetSingleAddress(true);
            _localstore->SetReadonly(false);
            _localstore->SetHasPassPhrase(false);
            _localstore->SetPublicKeyRing({});
            _localstore->SetMnemonic("");
            _localstore->SetxPrivKey("");
            _localstore->SetxPubKey("");
            _localstore->SetxPubKeyHDPM("");
            _localstore->SetRequestPubKey("");
            _localstore->SetRequestPrivKey("");
            _localstore->SetOwnerPubKey("");
            _localstore->SetSeed("");
            _localstore->SetETHSCPrimaryPubKey(ethscPubKey);
            _localstore->SetxPubKeyBitcoin("");
            _localstore->SetSinglePrivateKey(encryptedSinglePrvKey);
            _localstore->SetRipplePrimaryPubKey("");

            Init();
		}

#if 0
		Account::Account(const std::string &path, const nlohmann::json &walletJSON) {
			_localstore = LocalStorePtr(new LocalStore(path));
			ErrorChecker::CheckParam(!ImportReadonlyWallet(walletJSON), Error::InvalidArgument,
									 "Invalid readonly wallet json");
			Init();
		}
#endif

		Account::Account(const std::string &path, const KeyStore &ks, const std::string &payPasswd) {
			const ElaNewWalletJson &json = ks.WalletJson();

			_localstore = LocalStorePtr(new LocalStore(path));
			bytes_t bytes;
			std::string str;

			_localstore->SetReadonly(true);
			if (!json.xPrivKey().empty()) {
				Base58::CheckDecode(json.xPrivKey(), bytes);
				HDKeychain rootkey(CTElastos, bytes);
				std::string encrypedPrivKey = AES::EncryptCCM(bytes, payPasswd);
				_localstore->SetxPrivKey(encrypedPrivKey);
				_localstore->SetReadonly(false);
			}

			if (!json.Mnemonic().empty()) {
				std::string encryptedMnemonic = AES::EncryptCCM(bytes_t(json.Mnemonic().data(), json.Mnemonic().size()), payPasswd);
				_localstore->SetMnemonic(encryptedMnemonic);
				_localstore->SetReadonly(false);
			}

			if (!json.RequestPrivKey().empty()) {
				bytes.setHex(json.RequestPrivKey());
				_localstore->SetRequestPrivKey(AES::EncryptCCM(bytes, payPasswd));
                _localstore->SetReadonly(false);
			}

			if (!json.GetSeed().empty()) {
				bytes.setHex(json.GetSeed());
				_localstore->SetSeed(AES::EncryptCCM(bytes, payPasswd));
                _localstore->SetReadonly(false);
			}

			if (!json.GetSinglePrivateKey().empty()) {
			    bytes.setHex(json.GetSinglePrivateKey());
			    _localstore->SetSinglePrivateKey(AES::EncryptCCM(bytes, payPasswd));
			    _localstore->SetReadonly(false);
			}

            _localstore->SetxPubKeyBitcoin(json.GetxPubKeyBitcoin());
			_localstore->SetxPubKey(json.xPubKey());
			_localstore->SetRequestPubKey(json.RequestPubKey());
			_localstore->SetPublicKeyRing(json.GetPublicKeyRing());
			_localstore->SetM(json.GetM());
			_localstore->SetN(json.GetN());
			_localstore->SetHasPassPhrase(json.HasPassPhrase());
			_localstore->SetSingleAddress(json.SingleAddress());
			_localstore->SetDerivationStrategy(json.DerivationStrategy());
			_localstore->SetxPubKeyHDPM(json.xPubKeyHDPM());
			_localstore->SetOwnerPubKey(json.OwnerPubKey());
			_localstore->SetSubWalletInfoList(json.GetCoinInfoList());
			_localstore->SetETHSCPrimaryPubKey(json.GetETHSCPrimaryPubKey());
			_localstore->SetRipplePrimaryPubKey(json.GetRipplePrimaryPubKey());

			Init();
		}

		bytes_t Account::RequestPubKey() const {
			return _requestPubKey;
		}

		HDKeychainPtr Account::RootKey(const std::string &payPasswd) const {
			if (_localstore->Readonly()) {
				ErrorChecker::ThrowLogicException(Error::Key, "Readonly wallet without prv key");
			}

            if (_localstore->GetxPubKeyBitcoin().empty()) {
                RegenerateKey(payPasswd);
                Init();
            }

			bytes_t extkey = AES::DecryptCCM(_localstore->GetxPrivKey(), payPasswd);

			HDKeychainPtr key(new HDKeychain(CTElastos, extkey));

			extkey.clean();

			return key;
		}

		Key Account::RequestPrivKey(const std::string &payPassword) const {
			if (_localstore->Readonly()) {
				ErrorChecker::ThrowLogicException(Error::Key, "Readonly wallet without prv key");
			}

            if (_localstore->GetxPubKeyBitcoin().empty()) {
                RegenerateKey(payPassword);
                Init();
            }

			bytes_t bytes = AES::DecryptCCM(_localstore->GetRequestPrivKey(), payPassword);

			Key key;
			key.SetPrvKey(CTElastos, bytes);

			bytes.clean();

			return key;
		}

		HDKeychainPtr Account::MasterPubKey() const {
			return _xpub;
		}

        HDKeychainPtr Account::BitcoinMasterPubKey() const {
            return _btcMasterPubKey;
        }

		std::string Account::GetxPrvKeyString(const std::string &payPasswd) const {
			if (_localstore->Readonly()) {
				ErrorChecker::ThrowLogicException(Error::UnsupportOperation, "Readonly wallet can not export private key");
			}

            if (_localstore->GetxPubKeyBitcoin().empty()) {
                RegenerateKey(payPasswd);
                Init();
            }

			bytes_t bytes = AES::DecryptCCM(_localstore->GetxPrivKey(), payPasswd);
			return Base58::CheckEncode(bytes);
		}

		std::string Account::MasterPubKeyString() const {
			return _localstore->GetxPubKey();
		}

		std::string Account::MasterPubKeyHDPMString() const {
			return _localstore->GetxPubKeyHDPM();
		}

		std::vector<PublicKeyRing> Account::MasterPubKeyRing() const {
			return _localstore->GetPublicKeyRing();
		}

		bytes_t Account::OwnerPubKey() const {
			ErrorChecker::CheckLogic(_ownerPubKey.empty(), Error::Key, "This account unsupport owner public key");

			return _ownerPubKey;
		}

		void Account::ChangePassword(const std::string &oldPasswd, const std::string &newPasswd) {
			if (!_localstore->Readonly()) {
				ErrorChecker::CheckPassword(newPasswd, "New");

                if (_localstore->GetxPubKeyBitcoin().empty()) {
                    RegenerateKey(oldPasswd);
                    Init();
                }

				_localstore->ChangePasswd(oldPasswd, newPasswd);

				_localstore->Save();
			}
		}

		void Account::ResetPassword(const std::string &mnemonic, const std::string &passphrase, const std::string &newPassword) {
			if (!_localstore->Readonly()) {
				ErrorChecker::CheckPassword(newPassword, "New");

				uint512 seed = Mnemonic::DeriveSeed(mnemonic, passphrase);
				HDSeed hdseed(seed.bytes());
				HDKeychain rootkey(CTElastos, hdseed.getExtendedKey(CTElastos, true));
				HDKeychain stdrootkey(CTBitcoin, hdseed.getExtendedKey(CTBitcoin, true));
                HDKeychain ethkey = stdrootkey.getChild("44'/60'/0'/0/0");
				std::string xPubKey = Base58::CheckEncode(rootkey.getChild("44'/0'/0'").getPublic().extkey());
				if (xPubKey != _localstore->GetxPubKey())
					ErrorChecker::ThrowParamException(Error::InvalidArgument, "xpub not match");

				std::string encryptedSinglePrivateKey = AES::EncryptCCM(ethkey.privkey(), newPassword);
				std::string encryptedSeed = AES::EncryptCCM(bytes_t(seed.begin(), seed.size()), newPassword);
				std::string encryptedMnemonic = AES::EncryptCCM(bytes_t(mnemonic.data(), mnemonic.size()), newPassword);
				std::string encryptedxPrvKey = AES::EncryptCCM(rootkey.extkey(), newPassword);
				HDKeychain requestKey = rootkey.getChild("1'/0");
				std::string encryptedRequestPrvKey = AES::EncryptCCM(requestKey.privkey(), newPassword);

				_localstore->SetSeed(encryptedSeed);
				_localstore->SetMnemonic(encryptedMnemonic);
				_localstore->SetxPrivKey(encryptedxPrvKey);
				_localstore->SetRequestPrivKey(encryptedRequestPrvKey);
				_localstore->SetSinglePrivateKey(encryptedSinglePrivateKey);

				_localstore->Save();
			}
		}

		nlohmann::json Account::GetBasicInfo() const {
			nlohmann::json j;
			if (GetSignType() == MultiSign)
				j["Type"] = "MultiSign";
			else
				j["Type"] = "Standard";

			j["Readonly"] = _localstore->Readonly();
			j["SingleAddress"] = _localstore->SingleAddress();
			j["M"] = _localstore->GetM();
			j["N"] = _localstore->GetN();
			j["HasPassPhrase"] = _localstore->HasPassPhrase();
			return j;
		}

		Account::SignType Account::GetSignType() const {
			if (_localstore->GetN() > 1)
				return MultiSign;

			return Standard;
		}

		bool Account::Readonly() const {
			return _localstore->Readonly();
		}

		bool Account::SingleAddress() const {
			return _localstore->SingleAddress();
		}

		bool Account::Equal(const AccountPtr &account) const {
			if (GetSignType() != account->GetSignType())
				return false;

            if (_xpub == nullptr && account->MasterPubKey() != nullptr ||
                _xpub != nullptr && account->MasterPubKey() == nullptr)
                return false;

			if (_xpub == nullptr && account->MasterPubKey() == nullptr)
			    return GetETHSCPubKey() == account->GetETHSCPubKey();

			if (GetSignType() == MultiSign) {
				if (_allMultiSigners.size() != account->MultiSignCosigner().size())
					return false;

				for (size_t i = 0; i < _allMultiSigners.size(); ++i) {
					if (*_allMultiSigners[i] != *account->MultiSignCosigner()[i])
						return false;
				}

				return true;
			}

			return *_xpub == *account->MasterPubKey();
		}

		int Account::GetM() const {
			return _localstore->GetM();
		}

		int Account::GetN() const {
			return _localstore->GetN();
		}

		std::string Account::DerivationStrategy() const {
			return _localstore->DerivationStrategy();
		}

		nlohmann::json Account::GetPubKeyInfo() const {
			nlohmann::json j, jCosigners;

			j["m"] = _localstore->GetM();
			j["n"] = _localstore->GetN();
			j["derivationStrategy"] = _localstore->DerivationStrategy();

			if (_localstore->GetN() > 1 && _localstore->Readonly()) {
				j["xPubKey"] = nlohmann::json();
				j["xPubKeyHDPM"] = nlohmann::json();
			} else {
				j["xPubKey"] = _localstore->GetxPubKey();
				j["xPubKeyHDPM"] = _localstore->GetxPubKeyHDPM();
			}

			for (size_t i = 0; i < _localstore->GetPublicKeyRing().size(); ++i)
				jCosigners.push_back(_localstore->GetPublicKeyRing()[i].GetxPubKey());

			j["publicKeyRing"] = jCosigners;

			return j;
		}

		HDKeychainPtr Account::MultiSignSigner() const {
			ErrorChecker::CheckLogic(!_xpub, Error::Key, "Read-only wallet do not contain current multisigner");
			return _curMultiSigner;
		}

		HDKeychainArray Account::MultiSignCosigner() const {
			return _allMultiSigners;
		}

		int Account::CosignerIndex() const {
			return _cosignerIndex;
		}

		std::vector<CoinInfoPtr> Account::SubWalletInfoList() const {
			return _localstore->GetSubWalletInfoList();
		}

		void Account::AddSubWalletInfoList(const CoinInfoPtr &info) {
			_localstore->AddSubWalletInfoList(info);
		}

		void Account::SetSubWalletInfoList(const std::vector<CoinInfoPtr> &info) {
			_localstore->SetSubWalletInfoList(info);
		}

		void Account::RemoveSubWalletInfo(const std::string &chainID) {
			_localstore->RemoveSubWalletInfo(chainID);
		}

		KeyStore Account::ExportKeystore(const std::string &payPasswd) const {
            if (!_localstore->Readonly() && _localstore->GetxPubKeyBitcoin().empty()) {
                RegenerateKey(payPasswd);
                Init();
            }

			bytes_t bytes;
			ElaNewWalletJson json;
			if (!_localstore->Readonly()) {
				bytes = AES::DecryptCCM(_localstore->GetxPrivKey(), payPasswd);
				if (!bytes.empty()) {
					json.SetxPrivKey(Base58::CheckEncode(bytes));
				}

				bytes = AES::DecryptCCM(_localstore->GetMnemonic(), payPasswd);
				json.SetMnemonic(std::string((char *)bytes.data(), bytes.size()));
				if (bytes.empty()) {
					json.SetHasPassPhrase(false);
				}

				bytes = AES::DecryptCCM(_localstore->GetRequestPrivKey(), payPasswd);
				json.SetRequestPrivKey(bytes.getHex());

				bytes = AES::DecryptCCM(_localstore->GetSeed(), payPasswd);
				json.SetSeed(bytes.getHex());

				bytes = AES::DecryptCCM(_localstore->GetSinglePrivateKey(), payPasswd);
				json.SetSinglePrivateKey(bytes.getHex());
			}

			json.SetOwnerPubKey(_localstore->GetOwnerPubKey());
			json.SetxPubKey(_localstore->GetxPubKey());
			json.SetxPubKeyHDPM(_localstore->GetxPubKeyHDPM());
			json.SetRequestPubKey(_localstore->GetRequestPubKey());
			json.SetPublicKeyRing(_localstore->GetPublicKeyRing());
			json.SetM(_localstore->GetM());
			json.SetN(_localstore->GetN());
			json.SetHasPassPhrase(_localstore->HasPassPhrase());
			json.SetDerivationStrategy(_localstore->DerivationStrategy());
			json.SetAccount(0);
			json.SetSingleAddress(_localstore->SingleAddress());
			json.SetCoinInfoList(_localstore->GetSubWalletInfoList());
			json.SetETHSCPrimaryPubKey(_localstore->GetETHSCPrimaryPubKey());
            json.SetxPubKeyBitcoin(_localstore->GetxPubKeyBitcoin());
            json.SetRipplePrimaryPubKey(_localstore->GetRipplePrimaryPubKey());

			return KeyStore(json);
		}

#if 0
#define READONLY_WALLET_VERSION00 0
		nlohmann::json Account::ExportReadonlyWallet() const {
			nlohmann::json j;
			bytes_t tmp;
			ByteStream stream;

			stream.WriteUint8(READONLY_WALLET_VERSION00);
			stream.WriteUint8((uint8_t)(_localstore->SingleAddress() ? 1 : 0));
			stream.WriteUint8((uint8_t)(_localstore->HasPassPhrase() ? 1 : 0));
			stream.WriteUint32(_localstore->GetM());
			stream.WriteUint32(_localstore->GetN());
			stream.WriteUint32(_localstore->Account());
			stream.WriteVarString(_localstore->DerivationStrategy());

			tmp.setHex(_localstore->GetETHSCPrimaryPubKey());
			stream.WriteVarBytes(tmp);

			if (_localstore->GetN() > 1) {
				tmp.clear();
				// request pubkey
				stream.WriteVarBytes(tmp);
				// owner pubkey
				stream.WriteVarBytes(tmp);
				// xpub
                stream.WriteVarBytes(tmp);
                // btc xpub
				stream.WriteVarBytes(tmp);
				// xpub HDPM
				stream.WriteVarBytes(tmp);
			} else {
				tmp.setHex(_localstore->GetRequestPubKey());
				stream.WriteVarBytes(tmp);

				tmp.setHex(_localstore->GetOwnerPubKey());
				stream.WriteVarBytes(tmp);

				if (!Base58::CheckDecode(_localstore->GetxPubKey(), tmp)) {
					Log::error("Decode xpub fail when exoprt read-only wallet");
					return j;
				}
				stream.WriteVarBytes(tmp);

                if (!Base58::CheckDecode(_localstore->GetxPubKeyBitcoin(), tmp)) {
                    Log::error("Decode btc xpub fail when exoprt read-only wallet");
                    return j;
                }
                stream.WriteVarBytes(tmp);

				if (!Base58::CheckDecode(_localstore->GetxPubKeyHDPM(), tmp)) {
					Log::error("Decode xpubHDPM fail when export read-only wallet");
					return j;
				}
				stream.WriteVarBytes(tmp);
			}

			if (_localstore->GetN() > 1) {
				const std::vector<PublicKeyRing> &pubkeyRing = _localstore->GetPublicKeyRing();
				stream.WriteVarUint(pubkeyRing.size());
				for (size_t i = 0; i < pubkeyRing.size(); ++i) {
					tmp.setHex(pubkeyRing[i].GetRequestPubKey());
					stream.WriteVarBytes(tmp);

					if (!Base58::CheckDecode(pubkeyRing[i].GetxPubKey(), tmp)) {
						Log::error("Decode pubkey ring xpub fail when export read-only wallet");
						return j;
					}
					stream.WriteVarBytes(tmp);
				}
			}

			const std::vector<CoinInfoPtr> &info = _localstore->GetSubWalletInfoList();
			stream.WriteVarUint(info.size());
			for(size_t i = 0; i < info.size(); ++i)
				stream.WriteVarString(info[i]->GetChainID());

			const bytes_t &bytes = stream.GetBytes();

			j["Data"] = bytes.getBase64();

			return j;
		}

		bool Account::ImportReadonlyWallet(const nlohmann::json &walletJSON) {
			if (walletJSON.find("Data") == walletJSON.end()) {
				Log::error("Import read-only wallet: json format error");
				return false;
			}

			uint8_t version = 0;
			bytes_t bytes;
			bytes.setBase64(walletJSON["Data"].get<std::string>());
			ByteStream stream(bytes);

			if (!stream.ReadUint8(version)) {
				Log::error("Import read-only wallet: version");
				return false;
			}

			uint8_t byte;
			if (!stream.ReadUint8(byte)) {
				Log::error("Import read-only wallet: single address");
				return false;
			}
			_localstore->SetSingleAddress(byte != 0);

			if (!stream.ReadUint8(byte)) {
				Log::error("Import read-only wallet: has passphrase");
				return false;
			}
			_localstore->SetHasPassPhrase(byte != 0);

			uint32_t tmpUint;
			if (!stream.ReadUint32(tmpUint)) {
				Log::error("Import read-only wallet: M");
				return false;
			}
			_localstore->SetM(tmpUint);

			if (!stream.ReadUint32(tmpUint)) {
				Log::error("Import read-only wallet: N");
				return false;
			}
			_localstore->SetN(tmpUint);

			if (!stream.ReadUint32(tmpUint)) {
				Log::error("Import read-only wallet: account");
				return false;
			}
			_localstore->SetAccount(tmpUint);

			std::string str;
			if (!stream.ReadVarString(str)) {
				Log::error("Import read-only wallet: derivation strategy");
				return false;
			}
			_localstore->SetDerivationStrategy(str);

			if (!stream.ReadVarBytes(bytes)) {
				Log::error("Import read-only wallet: ethsc pubkey");
				return false;
			}
			_localstore->SetETHSCPrimaryPubKey(bytes.getHex());

			if (!stream.ReadVarBytes(bytes)) {
				Log::error("Import read-only wallet: request pubkey");
				return false;
			}
			_localstore->SetRequestPubKey(bytes.getHex());

			if (!stream.ReadVarBytes(bytes)) {
				Log::error("Import read-only wallet: owner pubkey");
				return false;
			}
			_localstore->SetOwnerPubKey(bytes.getHex());

            // xpub
			if (!stream.ReadVarBytes(bytes)) {
				Log::error("Import read-only wallet: xpub");
				return false;
			}
			if (bytes.empty())
				_localstore->SetxPubKey("");
			else
				_localstore->SetxPubKey(Base58::CheckEncode(bytes));

            // btc xpub
            if (!stream.ReadVarBytes(bytes)) {
                Log::error("Import read-only wallet: btc xpub");
                return false;
            }
            if (bytes.empty())
                _localstore->SetxPubKeyBitcoin("");
            else
                _localstore->SetxPubKeyBitcoin(Base58::CheckEncode(bytes));

            // xpub HDPM
			if (!stream.ReadVarBytes(bytes)) {
				Log::error("Import read-only wallet: xpubHDPM");
				return false;
			}
			if (bytes.empty())
                _localstore->SetxPubKeyHDPM("");
			else
				_localstore->SetxPubKeyHDPM(Base58::CheckEncode(bytes));

			uint64_t len;
			if (_localstore->GetN() > 1) {
				if (!stream.ReadVarUint(len)) {
					Log::error("Import read-only wallet: pubkeyRing size");
					return false;
				}

				bytes_t requestPub, xpub;
				for (size_t i = 0; i < len; ++i) {
					if (!stream.ReadVarBytes(requestPub)) {
						Log::error("Import read-only wallet: pubkey ring request pubkey");
						return false;
					}

					if (!stream.ReadVarBytes(xpub)) {
						Log::error("Import read-only wallet: pubkey ring xpub");
						return false;
					}

					if (xpub.empty())
						_localstore->AddPublicKeyRing(PublicKeyRing(requestPub.getHex(), ""));
					else
						_localstore->AddPublicKeyRing(PublicKeyRing(requestPub.getHex(), Base58::CheckEncode(xpub)));
				}
			} else {
				_localstore->AddPublicKeyRing(PublicKeyRing(_localstore->GetRequestPubKey(), _localstore->GetxPubKeyHDPM()));
			}

			if (!stream.ReadVarUint(len)) {
				Log::error("Import read-only wallet: coininfo size");
				return false;
			}

			std::vector<CoinInfoPtr> infoList;
			for (size_t i = 0; i < len; ++i) {
				std::string chainID;
				if (!stream.ReadVarString(chainID)) {
					Log::error("Import read-only wallet: chainID");
					return false;
				}

				CoinInfoPtr info(new CoinInfo());
				info->SetChainID(chainID);

				infoList.push_back(info);
			}
			_localstore->SetSubWalletInfoList(infoList);

			_localstore->SetReadonly(true);
			return true;
		}
#endif

		std::string Account::ExportMnemonic(const std::string &payPasswd) const {
			if (_localstore->Readonly()) {
				ErrorChecker::ThrowLogicException(Error::UnsupportOperation, "Readonly wallet can not export mnemonic");
			}

            if (_localstore->GetxPubKeyBitcoin().empty()) {
                RegenerateKey(payPasswd);
                Init();
            }

            std::string m;

			std::string encryptedMnemonic = _localstore->GetMnemonic();
            if (!encryptedMnemonic.empty()) {
                bytes_t bytes = AES::DecryptCCM(encryptedMnemonic, payPasswd);
                m = std::string((char *) bytes.data(), bytes.size());
            }

			return m;
		}

        void Account::RegenerateKey(const std::string &payPasswd) const {
            Log::info("Doing regenerate pubkey...");
            std::vector<PublicKeyRing> pubkeyRing = _localstore->GetPublicKeyRing();

            HDKeychain rootkey, stdrootkey;
            bytes_t bytes;
            uint512 seed;
            std::string tmpstr;
            bool haveSeed = false, haveRootkey = false, havestdrootkey = false;

            if (_localstore->GetSeed().empty() && !_localstore->GetMnemonic().empty() &&
                (!_localstore->HasPassPhrase() || (_localstore->HasPassPhrase() && !_localstore->GetPassPhrase().empty()))) {

                bytes = AES::DecryptCCM(_localstore->GetMnemonic(), payPasswd);
                std::string mnemonic = std::string((char *) &bytes[0], bytes.size());
                bytes = AES::DecryptCCM(_localstore->GetPassPhrase(), payPasswd);
                std::string passphrase = std::string((char *)&bytes[0], bytes.size());
                seed = Mnemonic::DeriveSeed(mnemonic, passphrase);
                _localstore->SetSeed(AES::EncryptCCM(seed.bytes(), payPasswd));
                haveSeed = true;

                if (!_localstore->GetPassPhrase().empty())
                    _localstore->SetHasPassPhrase(true);
                _localstore->SetPassPhrase("");
            } else if (!_localstore->GetSeed().empty()) {
                bytes = AES::DecryptCCM(_localstore->GetSeed(), payPasswd);
                seed = bytes;
                haveSeed = true;
            }

            if (_localstore->GetxPrivKey().empty() && haveSeed) {
                HDSeed hdseed(seed.bytes());
                rootkey = HDKeychain(CTElastos, hdseed.getExtendedKey(CTElastos, true));
                // encrypt private key
                _localstore->SetxPrivKey(AES::EncryptCCM(rootkey.extkey(), payPasswd));
                haveRootkey = true;
            } else if (!_localstore->GetxPrivKey().empty()) {
                bytes = AES::DecryptCCM(_localstore->GetxPrivKey(), payPasswd);
                rootkey = HDKeychain(CTElastos, bytes);
                haveRootkey = true;
            }

            if (_localstore->GetRequestPrivKey().empty() && haveRootkey) {
                HDKeychain requestKey = rootkey.getChild("1'/0");
                bytes = requestKey.privkey();
                _localstore->SetRequestPrivKey(AES::EncryptCCM(bytes, payPasswd));
                _localstore->SetRequestPubKey(requestKey.pubkey().getHex());
            }

            // master public key
            if (_localstore->GetxPubKey().empty() && haveRootkey) {
                HDKeychain xpub = rootkey.getChild("44'/0'/0'").getPublic();
                _localstore->SetxPubKey(Base58::CheckEncode(xpub.extkey()));
            }

            if (!havestdrootkey && haveSeed) {
                HDSeed hdseed(seed.bytes());
                stdrootkey = HDKeychain(CTBitcoin, hdseed.getExtendedKey(CTBitcoin, true));
                havestdrootkey = true;
            }
            // bitcoin master public key
            if (_localstore->GetxPubKeyBitcoin().empty() && havestdrootkey) {
                tmpstr = Base58::CheckEncode(stdrootkey.getChild("44'/0'/0'").getPublic().extkey());
                _localstore->SetxPubKeyBitcoin(tmpstr);
            }
            // eth primary public key
            if ((_localstore->GetETHSCPrimaryPubKey().empty() || _localstore->GetSinglePrivateKey().empty()) && havestdrootkey) {
                HDKeychain ethkey = stdrootkey.getChild("44'/60'/0'/0/0");
                tmpstr = ethkey.uncompressed_pubkey().getHex();
                _localstore->SetETHSCPrimaryPubKey(tmpstr);
                _localstore->SetSinglePrivateKey(AES::EncryptCCM(ethkey.privkey(), payPasswd));
            }
            // ripple primary public key
            if (_localstore->GetRipplePrimaryPubKey().empty() && havestdrootkey) {
                HDKeychain ripplekey = stdrootkey.getChild("44'/144'/0'/0/0");
                _localstore->SetRipplePrimaryPubKey(ripplekey.pubkey().getHex());
            }

            if (_localstore->GetxPubKeyHDPM().empty() && haveRootkey) {
                HDKeychain xpubHDPM = rootkey.getChild("45'").getPublic();
                _localstore->SetxPubKeyHDPM(Base58::CheckEncode(xpubHDPM.extkey()));
            }

            // 44'/coinIndex'/account'/change/index
            if (_localstore->GetOwnerPubKey().empty() && haveRootkey) {
                bytes = rootkey.getChild("44'/0'/1'/0/0").pubkey();
                _localstore->SetOwnerPubKey(bytes.getHex());
            }

            if (_localstore->GetN() > 1 && !_localstore->GetxPubKey().empty()) {
                for (auto it = pubkeyRing.begin(); it != pubkeyRing.end(); ++it) {
                    if (_localstore->GetxPubKey() == (*it).GetxPubKey() ||
                        _localstore->GetxPubKeyHDPM() == (*it).GetxPubKey()) {
                        pubkeyRing.erase(it);
                        break;
                    }
                }

                if (_localstore->DerivationStrategy() == "BIP44") {
                    pubkeyRing.emplace_back(_localstore->GetRequestPubKey(), _localstore->GetxPubKey());
                } else {
                    pubkeyRing.emplace_back(_localstore->GetRequestPubKey(), _localstore->GetxPubKeyHDPM());
                }
                _localstore->SetPublicKeyRing(pubkeyRing);
            }

            _localstore->Save();
        }

		uint512 Account::GetSeed(const std::string &payPasswd) const {
			uint512 seed;
			bytes_t bytes = AES::DecryptCCM(_localstore->GetSeed(), payPasswd);
			memcpy(seed.begin(), bytes.data(), MIN(bytes.size(), seed.size()));
			return seed;
		}

		bytes_t Account::GetETHSCPubKey() const {
			bytes_t pubkey;
			pubkey.setHex(_localstore->GetETHSCPrimaryPubKey());
			return pubkey;
		}

        bytes_t Account::GetRipplePubKey() const {
            bytes_t pubkey;
            pubkey.setHex(_localstore->GetRipplePrimaryPubKey());
            return pubkey;
		}

        bytes_t Account::GetSinglePrivateKey(const std::string &passwd) const {
		    return AES::DecryptCCM(_localstore->GetSinglePrivateKey(), passwd);
		}

        bool Account::HasMnemonic() const {
			return !_localstore->GetMnemonic().empty();
		}

		bool Account::HasPassphrase() const {
			return _localstore->HasPassPhrase();
		}

		bool Account::VerifyPrivateKey(const std::string &mnemonic, const std::string &passphrase) const {
			if (!_localstore->Readonly() && _xpub != nullptr) {
				HDSeed seed(Mnemonic::DeriveSeed(mnemonic, passphrase).bytes());
				HDKeychain rootkey = HDKeychain(CTElastos, seed.getExtendedKey(CTElastos, true));

				HDKeychain xpub = rootkey.getChild("44'/0'/0'").getPublic();
				if (xpub.pubkey() == _xpub->pubkey())
					return true;
			}

			return false;
		}

		bool Account::VerifyPassPhrase(const std::string &passphrase, const std::string &payPasswd) const {
			if (!_localstore->Readonly() && _xpub != nullptr) {
                if (_localstore->GetxPubKeyBitcoin().empty()) {
                    RegenerateKey(payPasswd);
                    Init();
                }
				bytes_t bytes = AES::DecryptCCM(_localstore->GetMnemonic(), payPasswd);
				std::string mnemonic((char *) &bytes[0], bytes.size());

				uint512 seed = Mnemonic::DeriveSeed(mnemonic, passphrase);
				HDSeed hdseed(seed.bytes());
				HDKeychain rootkey(CTElastos, hdseed.getExtendedKey(CTElastos, true));

				HDKeychain xpub = rootkey.getChild("44'/0'/0'").getPublic();
				if (xpub.pubkey() != _xpub->pubkey())
					return false;

				return true;
			}

			return false;
		}

		bool Account::VerifyPayPassword(const std::string &payPasswd) const {
			if (!_localstore->Readonly()) {
                if (_localstore->GetxPubKeyBitcoin().empty()) {
                    RegenerateKey(payPasswd);
                    Init();
                }

                if (!_localstore->GetRequestPrivKey().empty())
                    AES::DecryptCCM(_localstore->GetRequestPrivKey(), payPasswd);
                else if (!_localstore->GetSeed().empty()) {
                    AES::DecryptCCM(_localstore->GetSeed(), payPasswd);
                } else if (!_localstore->GetMnemonic().empty()) {
                    AES::DecryptCCM(_localstore->GetMnemonic(), payPasswd);
                } else if (!_localstore->GetSinglePrivateKey().empty()) {
                    AES::DecryptCCM(_localstore->GetSinglePrivateKey(), payPasswd);
                } else if (!_localstore->GetxPrivKey().empty()) {
                    AES::DecryptCCM(_localstore->GetxPrivKey(), payPasswd);
                }

				return true;
			}

			return false;
		}

		void Account::Save() {
			_localstore->Save();
		}

		void Account::Remove() {
			_localstore->Remove();
		}

		std::string Account::GetDataPath() const {
			return _localstore->GetDataPath();
		}

	}
}
