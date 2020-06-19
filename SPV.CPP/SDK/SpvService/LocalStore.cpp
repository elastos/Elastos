// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "LocalStore.h"

#include <Common/ByteStream.h>
#include <Common/ErrorChecker.h>
#include <Common/Utils.h>
#include <Common/JsonSerializer.h>
#include <WalletCore/Base58.h>
#include <WalletCore/BIP39.h>
#include <WalletCore/AES.h>
#include <WalletCore/CoinInfo.h>
#include <WalletCore/HDKeychain.h>

#include <fstream>

namespace fs = boost::filesystem;

namespace Elastos {
	namespace ElaWallet {
#define MASTER_WALLET_STORE_FILE "MasterWalletStore.json"
#define LOCAL_STORE_FILE "LocalStore.json"

		nlohmann::json LocalStore::ToJson() const {
			nlohmann::json j;
			j["xPrivKey"] = _xPrivKey;
			j["xPubKey"] = _xPubKey;
			j["xPubKeyHDPM"] = _xPubKeyHDPM;
			j["requestPrivKey"] = _requestPrivKey;
			j["requestPubKey"] = _requestPubKey;
			j["publicKeyRing"] = _publicKeyRing;
			j["m"] = _m;
			j["n"] = _n;
			j["mnemonicHasPassphrase"] = _mnemonicHasPassphrase;
			j["derivationStrategy"] = _derivationStrategy;
			j["account"] = _account;
			j["mnemonic"] = _mnemonic;
			j["passphrase"] = _passphrase;
			j["ownerPubKey"] = _ownerPubKey;
			j["singleAddress"] = _singleAddress;
			j["readonly"] = _readonly;
			j["coinInfo"] = _subWalletsInfoList;
			j["seed"] = _seed;
			j["ethscPrimaryPubKey"] = _ethscPrimaryPubKey;
			return j;
		}

		void LocalStore::FromJson(const nlohmann::json &j) {
			try {
				if (j.find("publicKeyRing") != j.end()) {
					// new version of localstore
					_xPrivKey = j["xPrivKey"].get<std::string>();
					_mnemonic = j["mnemonic"].get<std::string>();
					_xPubKey = j["xPubKey"].get<std::string>();
					_requestPrivKey = j["requestPrivKey"].get<std::string>();
					_requestPubKey = j["requestPubKey"].get<std::string>();
					_publicKeyRing = j["publicKeyRing"].get<std::vector<PublicKeyRing>>();
					_m = j["m"].get<int>();
					_n = j["n"].get<int>();
					_mnemonicHasPassphrase = j["mnemonicHasPassphrase"].get<bool>();
					_derivationStrategy = j["derivationStrategy"].get<std::string>();
					_account = j["account"].get<int>();
					_passphrase = j["passphrase"].get<std::string>();
					_ownerPubKey = j["ownerPubKey"].get<std::string>();
					_singleAddress = j["singleAddress"].get<bool>();
					_readonly = j["readonly"].get<bool>();

					if (j.find("xPubKeyHDPM") != j.end()) {
						_xPubKeyHDPM = j["xPubKeyHDPM"].get<std::string>();
					} else {
						_xPubKeyHDPM.clear();
					}

					if (j.find("seed") != j.end()) {
						_seed = j["seed"].get<std::string>();
					} else {
						_seed.clear();
					}

					if (j.find("ethscPrimaryPubKey") != j.end()) {
						_ethscPrimaryPubKey = j["ethscPrimaryPubKey"].get<std::string>();
					} else {
						_ethscPrimaryPubKey.clear();
					}

					_subWalletsInfoList = j["coinInfo"].get<std::vector<CoinInfoPtr>>();
				} else {
					// old version of localstore
					bytes_t bytes;
					nlohmann::json mpk = j["MasterPubKey"];

					_derivationStrategy = "BIP44";
					_account = 0;
					_xPrivKey.clear();
					_requestPubKey.clear();
					_requestPrivKey.clear();
					_ownerPubKey.clear();
					_xPubKey.clear();
					_xPubKeyHDPM.clear();
					_seed.clear();
					_ethscPrimaryPubKey.clear();

					if (mpk.is_object()) {
						bytes.setHex(mpk["ELA"]);
						if (!bytes.isZero()) {
							ByteStream stream(bytes);
							stream.Skip(4);
							bytes_t pubKey, chainCode;
							stream.ReadBytes(chainCode, 32);
							stream.ReadBytes(pubKey, 33);

							bytes = HDKeychain(pubKey, chainCode).extkey();
							_xPubKey = Base58::CheckEncode(bytes);
						}
					}

					nlohmann::json jaccount = j["Account"];

					if (j.find("SubWallets") != j.end()) {
						_subWalletsInfoList = j["SubWallets"].get<std::vector<CoinInfoPtr>>();
					}


					if (jaccount.find("CoSigners") != jaccount.end()) {
						// 1. multi sign
						ErrorChecker::ThrowLogicException(Error::InvalidLocalStore, "Localstore too old, re-import please");
					} else {
						// 2. standard hd
						_readonly = false;
						_mnemonic = jaccount["Mnemonic"].get<std::string>();
						_passphrase = jaccount["PhrasePassword"].get<std::string>();

						bytes.setBase64(_passphrase);
						if (bytes.size() <= 8) {
							_mnemonicHasPassphrase = false;
							_passphrase.clear();
						} else {
							_mnemonicHasPassphrase = true;
						}

						_m = _n = 1;
						_requestPubKey = jaccount["PublicKey"].get<std::string>();
						if (!_xPubKey.empty())
							_publicKeyRing.emplace_back(_requestPubKey, _xPubKey);

						nlohmann::json votePubkey = j["VotePublicKey"];
						if (votePubkey.is_object() && votePubkey["ELA"].get<std::string>() != "") {
							_ownerPubKey = votePubkey["ELA"].get<std::string>();
						}
						_singleAddress = j["IsSingleAddress"].get<bool>();
					}
				}
			} catch (const nlohmann::detail::exception &e) {
				ErrorChecker::ThrowLogicException(Error::InvalidLocalStore, "invalid localstore: " + std::string(e.what()));
			}
		}

		LocalStore::LocalStore(const nlohmann::json &store) {
			FromJson(store);
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

			bytes = AES::DecryptCCM(_seed, oldPasswd);
			_seed = AES::EncryptCCM(bytes, newPasswd);

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

			FromJson(j);

			return true;
		}

		void LocalStore::Save() {

			nlohmann::json j = ToJson();

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

		void LocalStore::SetSeed(const std::string &seed) {
			_seed = seed;
		}

		const std::string &LocalStore::GetSeed() const {
			return _seed;
		}

		void LocalStore::SetETHSCPrimaryPubKey(const std::string &pubkey) {
			_ethscPrimaryPubKey = pubkey;
		}

		const std::string &LocalStore::GetETHSCPrimaryPubKey() const {
			return _ethscPrimaryPubKey;
		}

	}
}
