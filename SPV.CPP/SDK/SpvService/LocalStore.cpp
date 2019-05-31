// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "LocalStore.h"

#include <SDK/Common/ByteStream.h>
#include <SDK/Common/ErrorChecker.h>
#include <SDK/Common/Utils.h>
#include <SDK/WalletCore/BIPs/Base58.h>
#include <SDK/WalletCore/BIPs/BIP39.h>
#include <SDK/WalletCore/Crypto/AES.h>
#include <SDK/WalletCore/KeyStore/CoinInfo.h>

#include <fstream>

namespace fs = boost::filesystem;

namespace Elastos {
	namespace ElaWallet {
#define MASTER_WALLET_STORE_FILE "MasterWalletStore.json"
#define LOCAL_STORE_FILE "LocalStore.json"

		LocalStore::LocalStore(const std::string &path) :
			_path(path) {

			fs::path filepath = _path;
			filepath /= LOCAL_STORE_FILE;
			if (!fs::exists(filepath)) {
				filepath = _path;
				filepath /= MASTER_WALLET_STORE_FILE;
				if (!fs::exists(filepath)) {
					ErrorChecker::ThrowLogicException(Error::MasterWalletNotExist, "master wallet " +
						filepath.parent_path().filename().string() + " not exist");
				}
			}

			std::ifstream is(filepath.string());
			nlohmann::json j;
			is >> j;

			ErrorChecker::CheckLogic(j.is_null() || j.empty(), Error::InvalidLocalStore, "local store file is empty");

			from_json(j, *this);
		}

		LocalStore::LocalStore(const std::string &path, const std::string &mnemonic, const std::string &passphrase,
							   bool singleAddress, const std::string &payPasswd) :
			_path(path),
			_account(0),
			_derivationStrategy("BIP44") {

			HDSeed seed(BIP39::DeriveSeed(mnemonic, passphrase).bytes());
			HDKeychain rootkey(seed.getExtendedKey(true));

			_xPrivKey = AES::EncryptCCM(rootkey.extkey(), payPasswd);
			_xPubKey = Base58::CheckEncode(rootkey.getChild("44'/0'/0'").getPublic().extkey());

			HDKeychain requestKey = rootkey.getChild("1'/0");
			_requestPrivKey = AES::EncryptCCM(requestKey.privkey(), payPasswd);
			_requestPubKey = requestKey.pubkey().getHex();

			_publicKeyRing.emplace_back(_requestPubKey, _xPubKey);

			_m = 1;
			_n = 1;

			if (!passphrase.empty())
				_mnemonicHasPassphrase = true;
			else
				_mnemonicHasPassphrase = false;

			_mnemonic = AES::EncryptCCM(bytes_t(mnemonic.data(), mnemonic.size()), payPasswd);
			_passphrase.clear();
			_ownerPubKey = rootkey.getChild("44'/0'/1'/0/0").pubkey().getHex();
			_singleAddress = singleAddress;
			_readonly = false;
		}

		LocalStore::LocalStore(const std::string &path, const ElaNewWalletJson &json, const std::string &payPasswd) :
			_path(path),
			_account(0),
			_derivationStrategy("BIP44") {

			if (!json.Old()) {
				bytes_t bytes;
				std::string str;
				_readonly = true;
				if (!json.xPrivKey().empty()) {
					Base58::CheckDecode(json.xPrivKey(), bytes);
					HDKeychain rootkey(bytes);
					_ownerPubKey = rootkey.getChild("44'/0'/1'/0/0").pubkey().getHex();
					_xPrivKey = AES::EncryptCCM(bytes, payPasswd);
					_readonly = false;
				}
				if (!json.Mnemonic().empty()) {
					_mnemonic = AES::EncryptCCM(bytes_t(json.Mnemonic().data(), json.Mnemonic().size()), payPasswd);
				}
				_xPubKey = json.xPubKey();
				_requestPubKey = json.RequestPubKey();
				bytes.setHex(json.RequestPrivKey());
				_requestPrivKey = AES::EncryptCCM(bytes, payPasswd);
				_publicKeyRing = json.GetPublicKeyRing();
				_m = json.GetM();
				_n = json.GetN();
				_mnemonicHasPassphrase = json.HasPassPhrase();
				_passphrase.clear();
			} else {
				if (!json.Mnemonic().empty()) {
					_readonly = false;
					_mnemonic = AES::EncryptCCM(bytes_t(json.Mnemonic().data(), json.Mnemonic().size()), payPasswd);
					HDSeed seed(BIP39::DeriveSeed(json.Mnemonic(), json.PassPhrase()).bytes());
					HDKeychain rootkey(seed.getExtendedKey(true));
					_xPrivKey = AES::EncryptCCM(rootkey.extkey(), payPasswd);
					_xPubKey = Base58::CheckEncode(rootkey.getChild("44'/0'/0'").getPublic().extkey());

					HDKeychain requestKey = rootkey.getChild("1'/0");
					_requestPrivKey = AES::EncryptCCM(requestKey.privkey(), payPasswd);
					_requestPubKey = requestKey.pubkey().getHex();

					_publicKeyRing = json.GetPublicKeyRing();
					_publicKeyRing.emplace_back(_requestPubKey, _xPubKey);
					_m = json.GetM() > 0 ? json.GetM() : 1;
					_n = _publicKeyRing.size();

					if (!json.PassPhrase().empty())
						_mnemonicHasPassphrase = true;
				} else {
					_readonly = true;
					_publicKeyRing = json.GetPublicKeyRing();
					_m = json.GetM();
					_n = _publicKeyRing.size();
					_mnemonicHasPassphrase = false;
				}
			}
			_singleAddress = json.SingleAddress();
			_subWalletsInfoList = json.GetCoinInfoList();
		}

		LocalStore::LocalStore(const std::string &path, const std::vector<std::string> &pubkeys, int m) :
			_path(path),
			_account(0),
			_derivationStrategy("BIP44"),
			_m(m),
			_singleAddress(true),
			_readonly(true),
			_mnemonicHasPassphrase(false) {

			for (size_t i = 0; i < pubkeys.size(); ++i) {
				_publicKeyRing.emplace_back(pubkeys[i]);
			}

			_n = _publicKeyRing.size();
		}

		LocalStore::~LocalStore() {

		}

		void LocalStore::GetWalletJson(ElaNewWalletJson &json, const std::string &payPasswd) {
			if (!_readonly && _xPrivKey.empty()) {
				RegenerateKey(payPasswd);
			}

			bytes_t bytes = AES::DecryptCCM(_xPrivKey, payPasswd);
			std::string str;
			if (bytes.empty()) {
				json.SetxPrivKey("");
			} else {
				json.SetxPrivKey(Base58::CheckEncode(bytes));
			}

			bytes = AES::DecryptCCM(_mnemonic, payPasswd);
			json.SetMnemonic(std::string((char *)bytes.data(), bytes.size()));
			if (bytes.empty()) {
				json.SetHasPassPhrase(false);
			}

			bytes = AES::DecryptCCM(_requestPrivKey, payPasswd);
			json.SetRequestPrivKey(bytes.getHex());

			json.SetxPubKey(_xPubKey);
			json.SetRequestPubKey(_requestPubKey);
			json.SetPublicKeyRing(_publicKeyRing);
			json.SetM(_m);
			json.SetN(_n);
			json.SetHasPassPhrase(_mnemonicHasPassphrase);
			json.SetDerivationStrategy("BIP44");
			json.SetAccount(0);
			json.SetPassPhrase("");
			json.SetSingleAddress(_singleAddress);
			json.SetCoinInfoList(_subWalletsInfoList);
		}

		void LocalStore::RegenerateKey(const std::string &payPasswd) {
			bytes_t bytes = AES::DecryptCCM(_mnemonic, payPasswd);
			std::string mnemonic((char *)&bytes[0], bytes.size());

			bytes = AES::DecryptCCM(_passphrase, payPasswd);
			std::string passphrase((char *)&bytes[0], bytes.size());

			if (passphrase == "") {
				_mnemonicHasPassphrase = false;
			} else {
				_mnemonicHasPassphrase = true;
			}

			HDSeed seed(BIP39::DeriveSeed(mnemonic, passphrase).bytes());
			HDKeychain rootkey(seed.getExtendedKey(true));

			_passphrase.clear();

			// encrypt private key
			_xPrivKey = AES::EncryptCCM(rootkey.extkey(), payPasswd);

			bytes = rootkey.getChild("1'/0").privkey();
			_requestPrivKey = AES::EncryptCCM(bytes, payPasswd);

			// master public key
			bytes = rootkey.getChild("44'/0'/0'").getPublic().extkey();
			_xPubKey = Base58::CheckEncode(bytes);

			for (size_t i = 0; i < _publicKeyRing.size(); ++i) {
				if (_publicKeyRing[i].GetRequestPubKey() == _requestPubKey) {
					_publicKeyRing[i].SetxPubKey(_xPubKey);
				}
			}

			// 44'/coinIndex'/account'/change/index
			bytes = rootkey.getChild("44'/0'/1'/0/0").pubkey();
			_ownerPubKey = bytes.getHex();

			Save();
		}

		void LocalStore::ChangePasswd(const std::string &oldPasswd, const std::string &newPasswd) {
			bytes_t bytes = AES::DecryptCCM(_mnemonic, oldPasswd);
			_mnemonic = AES::EncryptCCM(bytes, newPasswd);

			bytes = AES::DecryptCCM(_xPrivKey, oldPasswd);
			_xPrivKey = AES::EncryptCCM(bytes, newPasswd);

			bytes = AES::DecryptCCM(_requestPrivKey, oldPasswd);
			_requestPrivKey = AES::EncryptCCM(bytes, newPasswd);

			bytes.clean();

			Save();
		}

		void LocalStore::Save() {

			nlohmann::json j;
			to_json(j, *this);

			if (!j.is_null() && !j.empty()) {
				boost::filesystem::path path = _path;
				if (!boost::filesystem::exists(path))
					boost::filesystem::create_directory(path);

				path /= LOCAL_STORE_FILE;
				std::ofstream o(path.string());
				o << j;
				o.flush();
			}
		}

		void to_json(nlohmann::json &j, const LocalStore &p) {
			j["xPrivKey"] = p._xPrivKey;
			j["xPubKey"] = p._xPubKey;
			j["requestPrivKey"] = p._requestPrivKey;
			j["requestPubKey"] = p._requestPubKey;
			j["publicKeyRing"] = p._publicKeyRing;
			j["m"] = p._m;
			j["n"] = p._n;
			j["mnemonicHasPassphrase"] = p._mnemonicHasPassphrase;
			j["derivationStrategy"] = p._derivationStrategy;
			j["account"] = p._account;
			j["mnemonic"] = p._mnemonic;
			j["passphrase"] = p._passphrase;
			j["ownerPubKey"] = p._ownerPubKey;
			j["singleAddress"] = p._singleAddress;
			j["readonly"] = p._readonly;

			nlohmann::json jCoinInfo;
			for (size_t i = 0; i < p._subWalletsInfoList.size(); ++i) {
				jCoinInfo.push_back(p._subWalletsInfoList[i]->ToJson());
			}
			j["coinInfo"] = jCoinInfo;
		}

		void from_json(const nlohmann::json &j, LocalStore &p) {
			try {
				if (j.find("publicKeyRing") != j.end()) {
					// new version of localstore
					p._xPrivKey = j["xPrivKey"].get<std::string>();
					p._mnemonic = j["mnemonic"].get<std::string>();
					p._xPubKey = j["xPubKey"].get<std::string>();
					p._requestPrivKey = j["requestPrivKey"].get<std::string>();
					p._requestPubKey = j["requestPubKey"].get<std::string>();
					p._publicKeyRing = j["publicKeyRing"].get<std::vector<PublicKeyRing>>();
					p._m = j["m"].get<int>();
					p._n = j["n"].get<int>();
					p._mnemonicHasPassphrase = j["mnemonicHasPassphrase"].get<bool>();
					p._derivationStrategy = j["derivationStrategy"].get<std::string>();
					p._account = j["account"].get<int>();
					p._passphrase = j["passphrase"].get<std::string>();
					p._ownerPubKey = j["ownerPubKey"].get<std::string>();
					p._singleAddress = j["singleAddress"].get<bool>();
					p._readonly = j["readonly"].get<bool>();

					p._subWalletsInfoList.clear();
					nlohmann::json jCoinInfo = j["coinInfo"];
					for (nlohmann::json::iterator it = jCoinInfo.begin(); it != jCoinInfo.end(); ++it) {
						CoinInfoPtr coinInfo(new CoinInfo());
						coinInfo->FromJson((*it));
						p._subWalletsInfoList.push_back(coinInfo);
					}
				} else {
					// old version of localstore
					bytes_t bytes;
					nlohmann::json mpk = j["MasterPubKey"];

					p._derivationStrategy = "BIP44";
					p._account = 0;
					p._xPrivKey.clear();
					p._requestPubKey.clear();
					p._requestPrivKey.clear();
					p._ownerPubKey.clear();
					p._xPubKey.clear();

					if (mpk.is_object()) {
						bytes.setHex(mpk["ELA"]);
						if (!bytes.isZero()) {
							ByteStream stream(bytes);
							stream.Skip(4);
							bytes_t pubKey, chainCode;
							stream.ReadBytes(chainCode, 32);
							stream.ReadBytes(pubKey, 33);

							bytes = HDKeychain(pubKey, chainCode).extkey();
							p._xPubKey = Base58::CheckEncode(bytes);
						}
					}

					nlohmann::json jaccount = j["Account"];

					if (j.find("SubWallets") != j.end()) {
						p._subWalletsInfoList.clear();
						nlohmann::json jCoinInfo = j["SubWallets"];
						for (nlohmann::json::iterator it = jCoinInfo.begin(); it != jCoinInfo.end(); ++it) {
							CoinInfoPtr coinInfo(new CoinInfo());
							coinInfo->FromJson((*it));
							p._subWalletsInfoList.push_back(coinInfo);
						}
					}


					if (jaccount.find("CoSigners") != jaccount.end()) {
						// 1. multi sign
						std::vector<std::string> coSigners = jaccount["CoSigners"];
						p._publicKeyRing.clear();
						for (size_t i = 0; i < coSigners.size(); ++i) {
							p._publicKeyRing.emplace_back(coSigners[i]);
						}
						p._m = jaccount["RequiredSignCount"];

						if (jaccount.find("InnerAccount") != jaccount.end()) {
							nlohmann::json innerAccount = jaccount["InnerAccount"];
							p._requestPubKey = innerAccount["PublicKey"].get<std::string>();
							p._publicKeyRing.emplace_back(p._requestPubKey);

							p._mnemonic = innerAccount["Mnemonic"].get<std::string>();
							p._passphrase = innerAccount["PhrasePassword"].get<std::string>();

							bytes_t bytes;
							bytes.setBase64(p._passphrase);
							if (bytes.size() <= 8) {
								p._mnemonicHasPassphrase = false;
								p._passphrase.clear();
							} else {
								p._mnemonicHasPassphrase = true;
							}
							p._readonly = false;
						} else {
							p._mnemonic.clear();
							p._passphrase.clear();
							p._mnemonicHasPassphrase = false;
							p._readonly = true;
						}
						p._n = p._publicKeyRing.size();
						p._singleAddress = true;

					} else {
						// 2. standard hd
						p._readonly = false;
						p._mnemonic = jaccount["Mnemonic"].get<std::string>();
						p._passphrase = jaccount["PhrasePassword"].get<std::string>();

						bytes.setBase64(p._passphrase);
						if (bytes.size() <= 8) {
							p._mnemonicHasPassphrase = false;
							p._passphrase.clear();
						} else {
							p._mnemonicHasPassphrase = true;
						}

						p._m = p._n = 1;
						p._requestPubKey = jaccount["PublicKey"].get<std::string>();
						if (!p._xPubKey.empty())
							p._publicKeyRing.emplace_back(p._requestPubKey, p._xPubKey);
						else
							p._publicKeyRing.emplace_back(p._requestPubKey);

						nlohmann::json votePubkey = j["VotePublicKey"];
						if (votePubkey.is_object() && votePubkey["ELA"].get<std::string>() != "") {
							p._ownerPubKey = votePubkey["ELA"].get<std::string>();
						}
						p._singleAddress = j["IsSingleAddress"].get<bool>();
					}
				}
			} catch (const nlohmann::detail::exception &e) {
				ErrorChecker::ThrowLogicException(Error::InvalidLocalStore, "invalid localstore: " + std::string(e.what()));
			}
		}

	}
}
