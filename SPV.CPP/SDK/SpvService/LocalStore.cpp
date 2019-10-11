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
#include <SDK/WalletCore/BIPs/HDKeychain.h>

#include <fstream>

namespace fs = boost::filesystem;

namespace Elastos {
	namespace ElaWallet {
#define MASTER_WALLET_STORE_FILE "MasterWalletStore.json"
#define LOCAL_STORE_FILE "LocalStore.json"

		void to_json(nlohmann::json &j, const LocalStore &p) {
			j["xPrivKey"] = p._xPrivKey;
			j["xPubKey"] = p._xPubKey;
			j["xPubKeyHDPM"] = p._xPubKeyHDPM;
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
					nlohmann::json jpubkeyRing = j["publicKeyRing"];
					for (nlohmann::json::iterator it = jpubkeyRing.begin(); it != jpubkeyRing.end(); ++it) {
						PublicKeyRing pubkeyRing;
						pubkeyRing.FromJson(*it);
						p._publicKeyRing.push_back(pubkeyRing);
					}
					p._m = j["m"].get<int>();
					p._n = j["n"].get<int>();
					p._mnemonicHasPassphrase = j["mnemonicHasPassphrase"].get<bool>();
					p._derivationStrategy = j["derivationStrategy"].get<std::string>();
					p._account = j["account"].get<int>();
					p._passphrase = j["passphrase"].get<std::string>();
					p._ownerPubKey = j["ownerPubKey"].get<std::string>();
					p._singleAddress = j["singleAddress"].get<bool>();
					p._readonly = j["readonly"].get<bool>();

					if (j.find("xPubKeyHDPM") != j.end()) {
						p._xPubKeyHDPM = j["xPubKeyHDPM"].get<std::string>();
					} else {
						p._xPubKeyHDPM.clear();
					}

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
					p._xPubKeyHDPM.clear();

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
						ErrorChecker::ThrowLogicException(Error::InvalidLocalStore, "Localstore too old, re-import please");
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

		LocalStore::LocalStore(const nlohmann::json &store) {
			from_json(store, *this);
		}

		LocalStore::LocalStore(const std::string &path) :
			_path(path),
			_account(0) {

		}

		LocalStore::~LocalStore() {

		}

		void LocalStore::ChangePasswd(const std::string &oldPasswd, const std::string &newPasswd) {
			bytes_t bytes = AES::DecryptCCM(_mnemonic, oldPasswd);
			_mnemonic = AES::EncryptCCM(bytes, newPasswd);

			bytes = AES::DecryptCCM(_xPrivKey, oldPasswd);
			_xPrivKey = AES::EncryptCCM(bytes, newPasswd);

			bytes = AES::DecryptCCM(_requestPrivKey, oldPasswd);
			_requestPrivKey = AES::EncryptCCM(bytes, newPasswd);

			bytes.clean();
		}

		bool LocalStore::Load() {
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

			return true;
		}

		void LocalStore::Save() {

			nlohmann::json j;
			to_json(j, *this);

			if (!j.is_null() && !j.empty() && !_path.empty()) {
				boost::filesystem::path path = _path;
				if (!boost::filesystem::exists(path))
					boost::filesystem::create_directory(path);

				path /= LOCAL_STORE_FILE;
				std::ofstream o(path.string());
				o << j;
				o.flush();
			}
		}

		void LocalStore::Remove() {
			boost::filesystem::path path(_path);
			if (boost::filesystem::exists(path))
				boost::filesystem::remove_all(path);
		}

		const std::string &LocalStore::GetDataPath() const {
			return _path;
		}

		void LocalStore::SaveTo(const std::string &path) {
			_path = path;
			Save();
		}

		bool LocalStore::SingleAddress() const {
			return _singleAddress;
		}

		void LocalStore::SetSingleAddress(bool status) {
			_singleAddress = status;
		}

		const std::string &LocalStore::GetxPrivKey() const {
			return _xPrivKey;
		}

		void LocalStore::SetxPrivKey(const std::string &xprvkey) {
			_xPrivKey = xprvkey;
		}

		const std::string &LocalStore::GetRequestPrivKey() const {
			return _requestPrivKey;
		}

		void LocalStore::SetRequestPrivKey(const std::string &prvkey) {
			_requestPrivKey = prvkey;
		}

		const std::string &LocalStore::GetMnemonic() const {
			return _mnemonic;
		}

		void LocalStore::SetMnemonic(const std::string &mnemonic) {
			_mnemonic = mnemonic;
		}

		const std::string &LocalStore::GetPassPhrase() const {
			return _passphrase;
		}

		void LocalStore::SetPassPhrase(const std::string &passphrase) {
			_passphrase = passphrase;
		}

		const std::string &LocalStore::GetxPubKey() const {
			return _xPubKey;
		}

		void LocalStore::SetxPubKey(const std::string &xpubkey) {
			_xPubKey = xpubkey;
		}

		const std::string &LocalStore::GetxPubKeyHDPM() const {
			return _xPubKeyHDPM;
		}

		void LocalStore::SetxPubKeyHDPM(const std::string &xpub) {
			_xPubKeyHDPM = xpub;
		}

		const std::string &LocalStore::GetRequestPubKey() const {
			return _requestPubKey;
		}

		void LocalStore::SetRequestPubKey(const std::string &pubkey) {
			_requestPubKey = pubkey;
		}

		const std::string &LocalStore::GetOwnerPubKey() const {
			return _ownerPubKey;
		}

		void LocalStore::SetOwnerPubKey(const std::string &ownerPubKey) {
			_ownerPubKey = ownerPubKey;
		}

		const std::string &LocalStore::DerivationStrategy() const {
			return _derivationStrategy;
		}

		void LocalStore::SetDerivationStrategy(const std::string &strategy) {
			_derivationStrategy = strategy;
		}

		const std::vector<PublicKeyRing> &LocalStore::GetPublicKeyRing() const {
			return _publicKeyRing;
		}

		void LocalStore::AddPublicKeyRing(const PublicKeyRing &ring) {
			_publicKeyRing.push_back(ring);
		}

		void LocalStore::SetPublicKeyRing(const std::vector<PublicKeyRing> &pubKeyRing) {
			_publicKeyRing = pubKeyRing;
		}

		int LocalStore::GetM() const {
			return _m;
		}

		void LocalStore::SetM(int m) {
			_m = m;
		}

		int LocalStore::GetN() const {
			return _n;
		}

		void LocalStore::SetN(int n) {
			_n = n;
		}

		bool LocalStore::HasPassPhrase() const {
			return _mnemonicHasPassphrase;
		}

		void LocalStore::SetHasPassPhrase(bool has) {
			_mnemonicHasPassphrase = has;
		}

		bool LocalStore::Readonly() const {
			return _readonly;
		}

		void LocalStore::SetReadonly(bool status) {
			_readonly = status;
		}

		int LocalStore::Account() const {
			return _account;
		}

		void LocalStore::SetAccount(int account) {
			_account = account;
		}

		const std::vector<CoinInfoPtr> &LocalStore::GetSubWalletInfoList() const {
			return _subWalletsInfoList;
		}

		void LocalStore::AddSubWalletInfoList(const CoinInfoPtr &info) {
			_subWalletsInfoList.push_back(info);
		}

		void LocalStore::RemoveSubWalletInfo(const CoinInfoPtr &info) {
			for (std::vector<CoinInfoPtr>::iterator it = _subWalletsInfoList.begin(); it != _subWalletsInfoList.end(); ++it) {
				if (info->GetChainID() == (*it)->GetChainID()) {
					_subWalletsInfoList.erase(it);
					break;
				}
			}
		}

		void LocalStore::SetSubWalletInfoList(const std::vector<CoinInfoPtr> &infoList) {
			_subWalletsInfoList = infoList;
		}

		void LocalStore::ClearSubWalletInfoList() {
			_subWalletsInfoList.clear();
		}

	}
}
