// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "Account.h"

#include <SDK/Common/Utils.h>
#include <SDK/Common/Log.h>
#include <SDK/Common/ErrorChecker.h>
#include <SDK/Common/ByteStream.h>
#include <SDK/WalletCore/BIPs/Base58.h>
#include <SDK/WalletCore/BIPs/Address.h>
#include <SDK/WalletCore/Crypto/AES.h>
#include <SDK/WalletCore/BIPs/Mnemonic.h>
#include <SDK/WalletCore/BIPs/BIP39.h>
#include <SDK/WalletCore/KeyStore/CoinInfo.h>


namespace Elastos {
	namespace ElaWallet {

		void Account::Init() const {
			_cosignerIndex = -1;
			_ownerPubKey.setHex(_localstore->GetOwnerPubKey());
			_requestPubKey.setHex(_localstore->GetRequestPubKey());

			bytes_t bytes;

			if (!(_localstore->GetN() > 1 && _localstore->Readonly())) {
				ErrorChecker::CheckParam(_localstore->GetxPubKey().empty(), Error::PubKeyFormat,
										 "xpub should not be empty");
				ErrorChecker::CheckParam(!Base58::CheckDecode(_localstore->GetxPubKey(), bytes),
										 Error::PubKeyFormat, "xpub decode error");
				_xpub = HDKeychainPtr(new HDKeychain(bytes));

				if (_localstore->GetxPubKeyHDPM().empty())
					Log::warn("xpubHDPM is empty");
				else
					ErrorChecker::CheckParam(!Base58::CheckDecode(_localstore->GetxPubKeyHDPM(), bytes),
											 Error::PubKeyFormat, "xpubHDPM decode error");
				_curMultiSigner = HDKeychainPtr(new HDKeychain(bytes));
			}

			if (_localstore->GetN() > 1) {
				if (_localstore->DerivationStrategy() == "BIP44") {
					_curMultiSigner = _xpub;
					for (size_t i = 0; i < _localstore->GetPublicKeyRing().size(); ++i) {
						bytes_t xpubBytes;
						ErrorChecker::CheckParam(!Base58::CheckDecode(_localstore->GetPublicKeyRing()[i].GetxPubKey(), xpubBytes),
												 Error::PubKeyFormat, "xpub decode error");
						HDKeychainPtr xpub(new HDKeychain(xpubBytes));
						_allMultiSigners.push_back(xpub);
					}
				} else if (_localstore->DerivationStrategy() == "BIP45") {
					HDKeychainArray sortedSigners;
					for (size_t i = 0; i < _localstore->GetPublicKeyRing().size(); ++i) {
						ErrorChecker::CheckLogic(
							!Base58::CheckDecode(_localstore->GetPublicKeyRing()[i].GetxPubKey(), bytes),
							Error::PubKeyFormat, "xpub HDPM decode error");
						HDKeychainPtr xpub(new HDKeychain(bytes));
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

			HDKeychain rootkey(bytes);

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
			HDSeed seed(BIP39::DeriveSeed(mnemonic, passphrase).bytes());
			HDKeychain rootkey(seed.getExtendedKey(true));

			std::string encryptedMnemonic = AES::EncryptCCM(bytes_t(mnemonic.data(), mnemonic.size()), payPasswd);
			std::string encryptedxPrvKey = AES::EncryptCCM(rootkey.extkey(), payPasswd);
			std::string xPubKey = Base58::CheckEncode(rootkey.getChild("44'/0'/0'").getPublic().extkey());

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
			HDSeed seed(BIP39::DeriveSeed(mnemonic, passphrase).bytes());
			HDKeychain rootkey(seed.getExtendedKey(true));

			std::string encryptedMnemonic = AES::EncryptCCM(bytes_t(mnemonic.data(), mnemonic.size()), payPasswd);
			std::string encryptedxPrvKey = AES::EncryptCCM(rootkey.extkey(), payPasswd);

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

			Init();
		}

		Account::Account(const std::string &path, const nlohmann::json &walletJSON) {
			_localstore = LocalStorePtr(new LocalStore(path));
			ErrorChecker::CheckParam(!ImportReadonlyWallet(walletJSON), Error::InvalidArgument,
									 "Invalid readonly wallet json");
			Init();
		}

		Account::Account(const std::string &path, const KeyStore &ks, const std::string &payPasswd) {
			const ElaNewWalletJson &json = ks.WalletJson();

			_localstore = LocalStorePtr(new LocalStore(path));
			bytes_t bytes;
			std::string str;

			_localstore->SetReadonly(true);
			if (!json.xPrivKey().empty()) {
				Base58::CheckDecode(json.xPrivKey(), bytes);
				HDKeychain rootkey(bytes);
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
			}

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

			Init();
		}

		bytes_t Account::RequestPubKey() const {
			return _requestPubKey;
		}

		HDKeychainPtr Account::RootKey(const std::string &payPasswd) const {
			if (_localstore->Readonly()) {
				ErrorChecker::ThrowLogicException(Error::Key, "Readonly wallet without prv key");
			}

			if (_localstore->GetxPrivKey().empty() || _localstore->GetxPubKeyHDPM().empty()) {
				RegenerateKey(payPasswd);
				Init();
			}

			bytes_t extkey = AES::DecryptCCM(_localstore->GetxPrivKey(), payPasswd);

			HDKeychainPtr key(new HDKeychain(extkey));

			extkey.clean();

			return key;
		}

		Key Account::RequestPrivKey(const std::string &payPassword) const {
			if (_localstore->Readonly()) {
				ErrorChecker::ThrowLogicException(Error::Key, "Readonly wallet without prv key");
			}

			if (_localstore->GetRequestPrivKey().empty() || _localstore->GetxPubKeyHDPM().empty()) {
				RegenerateKey(payPassword);
				Init();
			}

			bytes_t bytes = AES::DecryptCCM(_localstore->GetRequestPrivKey(), payPassword);

			Key key;
			key.SetPrvKey(bytes);

			bytes.clean();

			return key;
		}

		HDKeychainPtr Account::MasterPubKey() const {
			ErrorChecker::CheckLogic(!_xpub, Error::Key, "Read-only wallet do not contain master public key");
			return _xpub;
		}

		std::string Account::GetxPrvKeyString(const std::string &payPasswd) const {
			if (!_localstore->Readonly() && (_localstore->GetxPrivKey().empty() || _localstore->GetxPubKeyHDPM().empty())) {
				RegenerateKey(payPasswd);
				Init();
			}

			bytes_t bytes = AES::DecryptCCM(_localstore->GetxPrivKey(), payPasswd);
			return Base58::CheckEncode(bytes);
		}

		const std::string &Account::MasterPubKeyString() const {
			return _localstore->GetxPubKey();
		}

		const std::string &Account::MasterPubKeyHDPMString() const {
			return _localstore->GetxPubKeyHDPM();
		}

		const std::vector<PublicKeyRing> &Account::MasterPubKeyRing() const {
			return _localstore->GetPublicKeyRing();
		}

		bytes_t Account::OwnerPubKey() const {
			ErrorChecker::CheckLogic(_ownerPubKey.empty(), Error::Key, "This account unsupport owner public key");

			return _ownerPubKey;
		}

		void Account::ChangePassword(const std::string &oldPasswd, const std::string &newPasswd) {
			if (!_localstore->Readonly()) {
				ErrorChecker::CheckPassword(newPasswd, "New");

				if (_localstore->GetxPrivKey().empty() || _localstore->GetxPubKeyHDPM().empty()) {
					RegenerateKey(oldPasswd);
					Init();
				}

				_localstore->ChangePasswd(oldPasswd, newPasswd);

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

		bool Account::Equal(const Account &account) const {
			if (GetSignType() != account.GetSignType() || Readonly() != account.Readonly())
				return false;

			if (GetSignType() == MultiSign) {
				if (_allMultiSigners.size() != account._allMultiSigners.size())
					return false;

				for (size_t i = 0; i < _allMultiSigners.size(); ++i) {
					if (*_allMultiSigners[i] != *account._allMultiSigners[i])
						return false;
				}

				return true;
			}

			return *_xpub == *account._xpub;
		}

		int Account::GetM() const {
			return _localstore->GetM();
		}

		int Account::GetN() const {
			return _localstore->GetN();
		}

		const std::string &Account::DerivationStrategy() const {
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

		const std::vector<CoinInfoPtr> &Account::SubWalletInfoList() const {
			return _localstore->GetSubWalletInfoList();
		}

		void Account::AddSubWalletInfoList(const CoinInfoPtr &info) {
			_localstore->AddSubWalletInfoList(info);
		}

		void Account::SetSubWalletInfoList(const std::vector<CoinInfoPtr> &info) {
			_localstore->SetSubWalletInfoList(info);
		}

		void Account::RemoveSubWalletInfo(const CoinInfoPtr &info) {
			_localstore->RemoveSubWalletInfo(info);
		}

		KeyStore Account::ExportKeyStore(const std::string &payPasswd) {
			if (!_localstore->Readonly() && (_localstore->GetxPrivKey().empty() || _localstore->GetxPubKeyHDPM().empty())) {
				RegenerateKey(payPasswd);
				Init();
			}

			ElaNewWalletJson json;
			bytes_t bytes = AES::DecryptCCM(_localstore->GetxPrivKey(), payPasswd);
			if (bytes.empty()) {
				json.SetxPrivKey("");
			} else {
				json.SetxPrivKey(Base58::CheckEncode(bytes));
			}

			bytes = AES::DecryptCCM(_localstore->GetMnemonic(), payPasswd);
			json.SetMnemonic(std::string((char *)bytes.data(), bytes.size()));
			if (bytes.empty()) {
				json.SetHasPassPhrase(false);
			}

			bytes = AES::DecryptCCM(_localstore->GetRequestPrivKey(), payPasswd);
			json.SetRequestPrivKey(bytes.getHex());

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

			return KeyStore(json);
		}

		nlohmann::json Account::ExportReadonlyWallet() const {
			nlohmann::json j;
			bytes_t tmp;
			ByteStream stream;

			stream.WriteUint8((uint8_t)(_localstore->SingleAddress() ? 1 : 0));
			stream.WriteUint8((uint8_t)(_localstore->HasPassPhrase() ? 1 : 0));
			stream.WriteUint32(_localstore->GetM());
			stream.WriteUint32(_localstore->GetN());
			stream.WriteUint32(_localstore->Account());
			stream.WriteVarString(_localstore->DerivationStrategy());

			if (_localstore->GetN() > 1) {
				tmp.clear();
				// request pubkey
				stream.WriteVarBytes(tmp);
				// owner pubkey
				stream.WriteVarBytes(tmp);
				// xpub
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
			for(size_t i = 0; i < info.size(); ++i) {
				stream.WriteUint64((uint64_t)info[i]->GetEarliestPeerTime());
				stream.WriteVarString(info[i]->GetChainID());
			}

			const bytes_t &bytes = stream.GetBytes();

			j["Data"] = bytes.getBase64();

			return j;
		}

		bool Account::ImportReadonlyWallet(const nlohmann::json &walletJSON) {
			if (walletJSON.find("Data") == walletJSON.end()) {
				Log::error("Import read-only wallet: json format error");
				return false;
			}

			bytes_t bytes;
			bytes.setBase64(walletJSON["Data"].get<std::string>());
			ByteStream stream(bytes);

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
				Log::error("Import read-only wallet: request pubkey");
				return false;
			}
			_localstore->SetRequestPubKey(bytes.getHex());

			if (!stream.ReadVarBytes(bytes)) {
				Log::error("Import read-only wallet: owner pubkey");
				return false;
			}
			_localstore->SetOwnerPubKey(bytes.getHex());

			if (!stream.ReadVarBytes(bytes)) {
				Log::error("Import read-only wallet: xpub");
				return false;
			}
			if (bytes.empty())
				_localstore->SetxPubKey("");
			else
				_localstore->SetxPubKey(Base58::CheckEncode(bytes));

			if (!stream.ReadVarBytes(bytes)) {
				Log::error("Import read-only wallet: xpubHDPM");
				return false;
			}
			if (bytes.empty())
				_localstore->SetxPubKey("");
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
			uint64_t t;
			for (size_t i = 0; i < len; ++i) {
				if (!stream.ReadUint64(t)) {
					Log::error("Import read-only wallet: earliest peer t");
					return false;
				}

				std::string chainID;
				if (!stream.ReadVarString(chainID)) {
					Log::error("Import read-only wallet: chainID");
					return false;
				}

				CoinInfoPtr info(new CoinInfo());
				info->SetEaliestPeerTime(t);
				info->SetChainID(chainID);

				infoList.push_back(info);
			}
			_localstore->SetSubWalletInfoList(infoList);

			_localstore->SetReadonly(true);
			return true;
		}

		std::string Account::GetDecryptedMnemonic(const std::string &payPasswd) const {
			if (_localstore->Readonly()) {
				ErrorChecker::ThrowLogicException(Error::Key, "Readonly wallet do not contain mnemonic");
			}

			if (_localstore->GetxPrivKey().empty() || _localstore->GetxPubKeyHDPM().empty()) {
				RegenerateKey(payPasswd);
				Init();
			}

			std::string encryptedMnemonic = _localstore->GetMnemonic();
			bytes_t bytes = AES::DecryptCCM(encryptedMnemonic, payPasswd);
			return std::string((char *)bytes.data(), bytes.size());
		}

		void Account::RegenerateKey(const std::string &payPasswd) const {
			Log::info("Doing regenerate pubkey...");
			std::vector<PublicKeyRing> pubkeyRing = _localstore->GetPublicKeyRing();

			HDKeychain rootkey;
			bytes_t bytes;

			if (_localstore->GetxPrivKey().empty()) {
				bytes = AES::DecryptCCM(_localstore->GetMnemonic(), payPasswd);
				std::string mnemonic((char *) &bytes[0], bytes.size());

				bytes = AES::DecryptCCM(_localstore->GetPassPhrase(), payPasswd);
				std::string passphrase((char *) &bytes[0], bytes.size());

				_localstore->SetHasPassPhrase(!passphrase.empty());

				HDSeed seed(BIP39::DeriveSeed(mnemonic, passphrase).bytes());
				rootkey = HDKeychain(seed.getExtendedKey(true));

				_localstore->SetPassPhrase("");

				// encrypt private key
				_localstore->SetxPrivKey(AES::EncryptCCM(rootkey.extkey(), payPasswd));
			} else {
				bytes = AES::DecryptCCM(_localstore->GetxPrivKey(), payPasswd);
				rootkey = HDKeychain(bytes);
			}

			HDKeychain requestKey = rootkey.getChild("1'/0");
			bytes = requestKey.privkey();
			_localstore->SetRequestPrivKey(AES::EncryptCCM(bytes, payPasswd));
			_localstore->SetRequestPubKey(requestKey.pubkey().getHex());

			// master public key
			HDKeychain xpub = rootkey.getChild("44'/0'/0'").getPublic();
			_localstore->SetxPubKey(Base58::CheckEncode(xpub.extkey()));

			HDKeychain xpubHDPM = rootkey.getChild("45'").getPublic();
			_localstore->SetxPubKeyHDPM(Base58::CheckEncode(xpubHDPM.extkey()));

			for (std::vector<PublicKeyRing>::iterator it = pubkeyRing.begin(); it != pubkeyRing.end(); ++it) {
				Base58::CheckDecode((*it).GetxPubKey(), bytes);
				HDKeychain tmp(bytes);
				if (xpub.pubkey() == tmp.pubkey() || xpubHDPM == tmp) {
					pubkeyRing.erase(it);
					break;
				}
			}

			if (_localstore->GetN() > 1 && _localstore->DerivationStrategy() == "BIP44") {
				pubkeyRing.push_back(PublicKeyRing(_localstore->GetRequestPubKey(), _localstore->GetxPubKey()));
			} else {
				pubkeyRing.push_back(PublicKeyRing(_localstore->GetRequestPubKey(), _localstore->GetxPubKeyHDPM()));
			}
			_localstore->SetPublicKeyRing(pubkeyRing);

			// 44'/coinIndex'/account'/change/index
			bytes = rootkey.getChild("44'/0'/1'/0/0").pubkey();
			_localstore->SetOwnerPubKey(bytes.getHex());

			_localstore->Save();
		}

		bool Account::VerifyPrivateKey(const std::string &mnemonic, const std::string &passphrase) const {
			if (!_localstore->Readonly()) {
				HDSeed seed(BIP39::DeriveSeed(mnemonic, passphrase).bytes());
				HDKeychain rootkey = HDKeychain(seed.getExtendedKey(true));

				HDKeychain xpub = rootkey.getChild("44'/0'/0'").getPublic();
				if (xpub.pubkey() == _xpub->pubkey())
					return true;
			}

			return false;
		}

		bool Account::VerifyPassPhrase(const std::string &passphrase, const std::string &payPasswd) const {
			if (!_localstore->Readonly()) {
				if (_localstore->GetxPrivKey().empty() || _localstore->GetxPubKeyHDPM().empty()) {
					RegenerateKey(payPasswd);
					Init();
				}
				bytes_t bytes = AES::DecryptCCM(_localstore->GetMnemonic(), payPasswd);
				std::string mnemonic((char *) &bytes[0], bytes.size());

				HDSeed seed(BIP39::DeriveSeed(mnemonic, passphrase).bytes());
				HDKeychain rootkey = HDKeychain(seed.getExtendedKey(true));

				HDKeychain xpub = rootkey.getChild("44'/0'/0'").getPublic();
				if (xpub.pubkey() == _xpub->pubkey())
					return true;
			}

			return false;
		}

		bool Account::VerifyPayPassword(const std::string &payPasswd) const {
			if (!_localstore->Readonly()) {
				if (_localstore->GetxPrivKey().empty() || _localstore->GetxPubKeyHDPM().empty()) {
					RegenerateKey(payPasswd);
					Init();
				}

				try {
					AES::DecryptCCM(_localstore->GetRequestPrivKey(), payPasswd);
				} catch (const std::exception &e) {
					return false;
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

		const std::string &Account::GetDataPath() const {
			return _localstore->GetDataPath();
		}

	}
}
