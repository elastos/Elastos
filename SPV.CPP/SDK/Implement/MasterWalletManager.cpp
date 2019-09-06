// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "MasterWallet.h"

#include <SDK/Common/Utils.h>
#include <SDK/Common/Log.h>
#include <SDK/Common/ByteStream.h>
#include <SDK/Common/ErrorChecker.h>
#include <SDK/Common/Base64.h>
#include <SDK/WalletCore/BIPs/Mnemonic.h>
#include <SDK/WalletCore/BIPs/Base58.h>
#include <SDK/Plugin/Registry.h>
#include <SDK/Plugin/Block/SidechainMerkleBlock.h>
#include <SDK/Plugin/Block/MerkleBlock.h>
#include <SDK/Plugin/ELAPlugin.h>
#include <SDK/Plugin/IDPlugin.h>
#include <Interface/MasterWalletManager.h>
#include <CMakeConfig.h>


#include <boost/filesystem.hpp>

using namespace boost::filesystem;

#define MASTER_WALLET_STORE_FILE "MasterWalletStore.json"
#define LOCAL_STORE_FILE "LocalStore.json"

namespace Elastos {
	namespace ElaWallet {

		MasterWalletManager::MasterWalletManager(const std::string &rootPath, const std::string &dataPath) :
				_rootPath(rootPath),
				_dataPath(dataPath),
				_p2pEnable(true) {
			ErrorChecker::CheckParamNotEmpty(rootPath, "rootPath");
			initMasterWallets();
		}

		MasterWalletManager::MasterWalletManager(const MasterWalletMap &walletMap, const std::string &rootPath, const std::string &dataPath) :
				_masterWalletMap(walletMap),
				_rootPath(rootPath),
				_dataPath(dataPath),
				_p2pEnable(true) {
		}

		MasterWalletManager::~MasterWalletManager() {
			for (MasterWalletMap::iterator it = _masterWalletMap.begin(); it != _masterWalletMap.end();) {
				MasterWallet *masterWallet = static_cast<MasterWallet *>(it->second);
				std::string id = masterWallet->GetID();
				Log::info("closing master wallet (ID = {})...", id);
				masterWallet->CloseAllSubWallets();
				it = _masterWalletMap.erase(it);

				delete masterWallet;
				Log::info("closed master wallet (ID = {})", id);
			}
		}

		std::string MasterWalletManager::GenerateMnemonic(const std::string &language, int wordCount) const {
			ArgInfo("{}", GetFunName());
			ArgInfo("language: {}", language);
			ArgInfo("wordCount: {}", wordCount);

			Mnemonic::WordCount count = Mnemonic::WordCount(wordCount);
			std::string mnemonic = MasterWallet::GenerateMnemonic(language, _rootPath, count);

			ArgInfo("r => *");
			return mnemonic;
		}

		IMasterWallet *MasterWalletManager::CreateMasterWallet(
				const std::string &masterWalletId,
				const std::string &mnemonic,
				const std::string &phrasePassword,
				const std::string &payPassword,
				bool singleAddress) {

			ArgInfo("{}", GetFunName());
			ArgInfo("masterWalletID: {}", masterWalletId);
			ArgInfo("mnemonic: *");
			ArgInfo("passphrase: *, empty: {}", phrasePassword.empty());
			ArgInfo("payPasswd: *");
			ArgInfo("singleAddress: {}", singleAddress);

			ErrorChecker::CheckParamNotEmpty(masterWalletId, "Master wallet ID");
			ErrorChecker::CheckParamNotEmpty(mnemonic, "mnemonic");
			ErrorChecker::CheckPassword(payPassword, "Pay");
			ErrorChecker::CheckPasswordWithNullLegal(phrasePassword, "Phrase");

			if (_masterWalletMap.find(masterWalletId) != _masterWalletMap.end()) {
				ArgInfo("r => already exist");
				return _masterWalletMap[masterWalletId];
			}

			MasterWallet *masterWallet = new MasterWallet(masterWalletId, mnemonic, phrasePassword, payPassword,
														  singleAddress, _p2pEnable, _rootPath, _dataPath,
														  0, CreateNormal);
			checkRedundant(masterWallet);
			_masterWalletMap[masterWalletId] = masterWallet;

			ArgInfo("r => create master wallet");
			masterWallet->GetBasicInfo();

			return masterWallet;
		}

		IMasterWallet *MasterWalletManager::CreateMultiSignMasterWallet(
			const std::string &masterWalletID,
			const nlohmann::json &cosigners,
			uint32_t m,
			bool singleAddress,
			bool compatible,
			time_t timestamp) {
			ArgInfo("{}", GetFunName());
			ArgInfo("masterWalletID: {}", masterWalletID);
			ArgInfo("cosigners: {}", cosigners.dump());
			ArgInfo("m: {}", m);
			ArgInfo("singleAddress: {}", singleAddress);
			ArgInfo("compatible: {}", compatible);
			ArgInfo("timestamp: {}", timestamp);

			ErrorChecker::CheckParamNotEmpty(masterWalletID, "Master wallet ID");
			ErrorChecker::CheckParam(!cosigners.is_array(), Error::PubKeyFormat, "cosigners should be JOSN array");
			ErrorChecker::CheckParam(cosigners.size() < 2, Error::PubKeyFormat,
									 "cosigners should at least contain 2 elements");
			ErrorChecker::CheckParam(m < 1, Error::InvalidArgument, "Invalid m");

			std::vector<PublicKeyRing> pubKeyRing;
			bytes_t bytes;
			for (nlohmann::json::const_iterator it = cosigners.begin(); it != cosigners.end(); ++it) {
				ErrorChecker::CheckCondition(!(*it).is_string(), Error::Code::PubKeyFormat, "cosigners should be string");
				pubKeyRing.emplace_back("", *it);
			}

			if (_masterWalletMap.find(masterWalletID) != _masterWalletMap.end()) {
				ArgInfo("r => already exist");
				return _masterWalletMap[masterWalletID];
			}

			MasterWallet *masterWallet = new MasterWallet(masterWalletID, pubKeyRing, m, _rootPath, _dataPath,
														  _p2pEnable, singleAddress, compatible,
														  timestamp, CreateMultiSign);
			checkRedundant(masterWallet);
			_masterWalletMap[masterWalletID] = masterWallet;

			ArgInfo("r => create multi sign wallet");
			masterWallet->GetBasicInfo();

			return masterWallet;
		}

		IMasterWallet *MasterWalletManager::CreateMultiSignMasterWallet(
			const std::string &masterWalletID,
			const std::string &xprv,
			const std::string &payPassword,
			const nlohmann::json &cosigners,
			uint32_t m,
			bool singleAddress,
			bool compatible,
			time_t timestamp) {
			ArgInfo("{}", GetFunName());
			ArgInfo("masterWalletID: {}", masterWalletID);
			ArgInfo("xprv: *");
			ArgInfo("payPasswd: *");
			ArgInfo("cosigners: {}", cosigners.dump());
			ArgInfo("m: {}", m);
			ArgInfo("singleAddress: {}", singleAddress);
			ArgInfo("compatible: {}", compatible);
			ArgInfo("timestamp: {}", timestamp);

			ErrorChecker::CheckParamNotEmpty(masterWalletID, "Master wallet ID");
			ErrorChecker::CheckPassword(payPassword, "Pay");
			ErrorChecker::CheckParam(!cosigners.is_array(), Error::PubKeyFormat, "cosigners should be JOSN array");
			ErrorChecker::CheckParam(cosigners.empty(), Error::PubKeyFormat,
									 "cosigners should at least contain 1 elements");
			ErrorChecker::CheckParam(m < 1, Error::InvalidArgument, "Invalid m");

			std::vector<PublicKeyRing> pubKeyRing;
			bytes_t bytes;
			for (nlohmann::json::const_iterator it = cosigners.begin(); it != cosigners.end(); ++it) {
				ErrorChecker::CheckCondition(!(*it).is_string(), Error::Code::PubKeyFormat, "cosigners should be string");
				pubKeyRing.push_back(PublicKeyRing("", *it));
			}

			if (_masterWalletMap.find(masterWalletID) != _masterWalletMap.end()) {
				ArgInfo("r => already exist");
				return _masterWalletMap[masterWalletID];
			}

			MasterWallet *masterWallet = new MasterWallet(masterWalletID, xprv, payPassword, pubKeyRing,
														  m, _rootPath, _dataPath, _p2pEnable, singleAddress, compatible,
														  timestamp, CreateMultiSign);
			checkRedundant(masterWallet);
			_masterWalletMap[masterWalletID] = masterWallet;

			ArgInfo("r => create multi sign wallet");
			masterWallet->GetBasicInfo();

			return masterWallet;
		}

		IMasterWallet *MasterWalletManager::CreateMultiSignMasterWallet(
			const std::string &masterWalletID,
			const std::string &mnemonic,
			const std::string &passphrase,
			const std::string &payPassword,
			const nlohmann::json &cosigners,
			uint32_t m,
			bool singleAddress,
			bool compatible,
			time_t timestamp) {

			ArgInfo("{}", GetFunName());
			ArgInfo("masterWalletID: {}", masterWalletID);
			ArgInfo("mnemonic: *");
			ArgInfo("passphrase: *, empty: {}", passphrase.empty());
			ArgInfo("payPasswd: *");
			ArgInfo("cosigners: {}", cosigners.dump());
			ArgInfo("m: {}", m);
			ArgInfo("singleAddress: {}", singleAddress);
			ArgInfo("compatible: {}", compatible);
			ArgInfo("timestamp: {}", timestamp);

			ErrorChecker::CheckParamNotEmpty(masterWalletID, "Master wallet ID");
			ErrorChecker::CheckParamNotEmpty(mnemonic, "Mnemonic");
			ErrorChecker::CheckPassword(payPassword, "Pay");
			ErrorChecker::CheckPasswordWithNullLegal(passphrase, "Phrase");
			ErrorChecker::CheckParam(!cosigners.is_array(), Error::PubKeyFormat, "cosigners should be JOSN array");
			ErrorChecker::CheckParam(m < 1, Error::InvalidArgument, "Invalid m");

			if (_masterWalletMap.find(masterWalletID) != _masterWalletMap.end()) {
				ArgInfo("r => already exist");
				return _masterWalletMap[masterWalletID];
			}

			std::vector<PublicKeyRing> pubKeyRing;
			bytes_t bytes;
			for (nlohmann::json::const_iterator it = cosigners.begin(); it != cosigners.end(); ++it) {
				ErrorChecker::CheckCondition(!(*it).is_string() || !Base58::CheckDecode(*it, bytes),
											 Error::Code::PubKeyFormat, "cosigners format error");
				pubKeyRing.push_back(PublicKeyRing("", *it));
			}

			MasterWallet *masterWallet = new MasterWallet(masterWalletID, mnemonic, passphrase, payPassword,
			                                              pubKeyRing, m, _rootPath, _dataPath, _p2pEnable,
														  singleAddress, compatible,
														  timestamp, CreateMultiSign);
			checkRedundant(masterWallet);
			_masterWalletMap[masterWalletID] = masterWallet;
			ArgInfo("r => create multi sign wallet");
			masterWallet->GetBasicInfo();
			return masterWallet;
		}

		std::vector<IMasterWallet *> MasterWalletManager::GetAllMasterWallets() const {
			ArgInfo("{}", GetFunName());

			std::vector<IMasterWallet *> result;
			for (MasterWalletMap::const_iterator it = _masterWalletMap.cbegin(); it != _masterWalletMap.cend(); ++it) {
				result.push_back(it->second);
			}

			ArgInfo("r => all master wallet count: {}", result.size());

			for (int i = 0; i < result.size(); ++i)
				result[i]->GetBasicInfo();

			return result;
		};

		void MasterWalletManager::DestroyWallet(const std::string &masterWalletID) {
			ArgInfo("{}", GetFunName());
			ArgInfo("masterWalletID: {}", masterWalletID);

			if (_masterWalletMap.find(masterWalletID) != _masterWalletMap.end()) {
				MasterWallet *masterWallet = static_cast<MasterWallet *>(_masterWalletMap[masterWalletID]);
				masterWallet->RemoveLocalStore();

				masterWallet->CloseAllSubWallets();
				_masterWalletMap.erase(masterWallet->GetWalletID());
				delete masterWallet;
			} else {
				Log::warn("Master wallet is not exist");
			}

			ArgInfo("r => {} done", GetFunName());
		}

		IMasterWallet *
		MasterWalletManager::ImportWalletWithKeystore(const std::string &masterWalletID,
													  const nlohmann::json &keystoreContent,
													  const std::string &backupPassword,
													  const std::string &payPassword) {
			ArgInfo("{}", GetFunName());
			ArgInfo("masterWalletID: {}", masterWalletID);
			ArgInfo("keystore: *");
			ArgInfo("backupPasswd: *");
			ArgInfo("payPasswd: *");

			ErrorChecker::CheckParamNotEmpty(masterWalletID, "Master wallet ID");
			ErrorChecker::CheckParam(!keystoreContent.is_object(), Error::KeyStore, "key store should be json object");
			ErrorChecker::CheckPassword(backupPassword, "Backup");
			ErrorChecker::CheckPassword(payPassword, "Pay");

			if (_masterWalletMap.find(masterWalletID) != _masterWalletMap.end()) {
				ArgInfo("r => already exist");
				return _masterWalletMap[masterWalletID];
			}


			MasterWallet *masterWallet = new MasterWallet(masterWalletID, keystoreContent, backupPassword,
														  payPassword, _rootPath, _dataPath, _p2pEnable,
														  ImportFromKeyStore);
			checkRedundant(masterWallet);
			_masterWalletMap[masterWalletID] = masterWallet;
			masterWallet->InitSubWallets();

			ArgInfo("r => import with keystore");
			masterWallet->GetBasicInfo();

			return masterWallet;
		}

		IMasterWallet *
		MasterWalletManager::ImportWalletWithMnemonic(const std::string &masterWalletId, const std::string &mnemonic,
													  const std::string &phrasePassword, const std::string &payPassword,
													  bool singleAddress, time_t timestamp) {
			ArgInfo("{}", GetFunName());
			ArgInfo("masterWalletID: {}", masterWalletId);
			ArgInfo("mnemonic: *");
			ArgInfo("passphrase: *, empty: {}", phrasePassword.empty());
			ArgInfo("payPasswd: *");
			ArgInfo("singleAddr: {}", singleAddress);
			ArgInfo("timestamp: {}", timestamp);

			ErrorChecker::CheckParamNotEmpty(masterWalletId, "Master wallet ID");
			ErrorChecker::CheckParamNotEmpty(mnemonic, "Mnemonic");
			ErrorChecker::CheckPasswordWithNullLegal(phrasePassword, "Phrase");
			ErrorChecker::CheckPassword(payPassword, "Pay");

			if (_masterWalletMap.find(masterWalletId) != _masterWalletMap.end()) {
				ArgInfo("r => already exist");
				return _masterWalletMap[masterWalletId];
			}

			MasterWallet *masterWallet = new MasterWallet(masterWalletId, mnemonic, phrasePassword, payPassword,
														  singleAddress, _p2pEnable, _rootPath, _dataPath, timestamp,
														  ImportFromMnemonic);
			checkRedundant(masterWallet);
			_masterWalletMap[masterWalletId] = masterWallet;

			ArgInfo("r => import with mnemonic");
			masterWallet->GetBasicInfo();

			return masterWallet;
		}

		IMasterWallet *MasterWalletManager::ImportReadonlyWallet(
			const std::string &masterWalletID,
			const nlohmann::json &walletJson) {
			ArgInfo("{}", GetFunName());
			ArgInfo("masterWalletID: {}", masterWalletID);
			ArgInfo("walletJson: {}", walletJson.dump());

			ErrorChecker::CheckParam(!walletJson.is_object(), Error::KeyStore, "wallet json should be json object");

			if (_masterWalletMap.find(masterWalletID) != _masterWalletMap.end()) {
				ArgInfo("r => already exist");
				return _masterWalletMap[masterWalletID];
			}

			MasterWallet *masterWallet = new MasterWallet(masterWalletID, walletJson, _rootPath, _dataPath, _p2pEnable, ImportFromKeyStore);

			checkRedundant(masterWallet);
			_masterWalletMap[masterWalletID] = masterWallet;
			masterWallet->InitSubWallets();
			ArgInfo("r => import read-only");
			masterWallet->GetBasicInfo();

			return masterWallet;
		}

		nlohmann::json
		MasterWalletManager::ExportWalletWithKeystore(IMasterWallet *masterWallet, const std::string &backupPassword,
													  const std::string &payPassword) const {

			ArgInfo("{}", GetFunName());
			ErrorChecker::CheckParam(masterWallet == nullptr, Error::InvalidArgument, "Master wallet is null");
			ErrorChecker::CheckPassword(backupPassword, "Backup");
			ArgInfo("masterWallet: {}", static_cast<MasterWallet *>(masterWallet)->GetWalletID());
			ArgInfo("backupPasswd: *");
			ArgInfo("payPasswd: *");

			MasterWallet *wallet = static_cast<MasterWallet *>(masterWallet);
			nlohmann::json keystore = wallet->ExportKeyStore(backupPassword, payPassword);

			ArgInfo("r => *");
			return keystore;
		}

		std::string
		MasterWalletManager::ExportWalletWithMnemonic(IMasterWallet *masterWallet, const std::string &payPassword) const {
			ArgInfo("{}", GetFunName());
			ErrorChecker::CheckParam(masterWallet == nullptr, Error::InvalidArgument, "Master wallet is null");
			ErrorChecker::CheckPassword(payPassword, "Pay");
			ArgInfo("masterWallet: {}", static_cast<MasterWallet *>(masterWallet)->GetWalletID());
			ArgInfo("payPasswd: *");

			MasterWallet *wallet = static_cast<MasterWallet *>(masterWallet);

			std::string mnemonic = wallet->ExportMnemonic(payPassword);

			ArgInfo("r => *");

			return mnemonic;
		}

		nlohmann::json MasterWalletManager::ExportReadonlyWallet(IMasterWallet *masterWallet) const {
			ArgInfo("{}", GetFunName());

			ErrorChecker::CheckParam(masterWallet == nullptr, Error::InvalidArgument, "master wallet is null");
			MasterWallet *wallet = static_cast<MasterWallet *>(masterWallet);
			ArgInfo("masterWallet: {}", wallet->GetWalletID());

			nlohmann::json keystore = wallet->ExportReadonlyKeyStore();

			ArgInfo("r => {}", keystore.dump());
			return keystore;
		}

		std::string MasterWalletManager::ExportxPrivateKey(IMasterWallet *masterWallet,
														   const std::string &payPasswd) const {
			ArgInfo("{}", GetFunName());

			ErrorChecker::CheckParam(masterWallet == nullptr, Error::InvalidArgument, "master wallet is null");
			MasterWallet *wallet = static_cast<MasterWallet *>(masterWallet);
			ArgInfo("masterWallet: {}", wallet->GetWalletID());
			ArgInfo("payPasswd: *");
			ArgInfo("r => *");

			return wallet->ExportxPrivateKey(payPasswd);
		}

		std::string MasterWalletManager::ExportMasterPublicKey(IMasterWallet *masterWallet) const {
			ArgInfo("{}", GetFunName());

			ErrorChecker::CheckParam(masterWallet == nullptr, Error::InvalidArgument, "master wallet is null");
			MasterWallet *wallet = static_cast<MasterWallet *>(masterWallet);
			ArgInfo("masterWallet: {}", wallet->GetWalletID());

			std::string xpub = wallet->ExportMasterPublicKey();
			ArgInfo("r => {}", xpub);

			return xpub;
		}

		std::string MasterWalletManager::GetVersion() const {
			ArgInfo("{}", GetFunName());
			ArgInfo("r => {}", SPVSDK_VERSION_MESSAGE);
			return SPVSDK_VERSION_MESSAGE;
		}

		void MasterWalletManager::FlushData() {
			std::for_each(_masterWalletMap.begin(), _masterWalletMap.end(),
						  [](const MasterWalletMap::value_type &item) {
							  if (item.second != nullptr) {
								  MasterWallet *masterWallet = dynamic_cast<MasterWallet *>(item.second);
								  masterWallet->FlushData();
							  }
						  });
		}

		void MasterWalletManager::initMasterWallets() {

			path rootPath = _rootPath;
			if (_dataPath.empty()) {
				_dataPath = _rootPath;
			}

			ErrorChecker::CheckPathExists(_rootPath);
			ErrorChecker::CheckPathExists(_dataPath);

			Log::registerMultiLogger(_dataPath);

			Log::setLevel(spdlog::level::from_str(SPVSDK_SPDLOG_LEVEL));
			Log::info("spvsdk version {}", SPVSDK_VERSION_MESSAGE);

#ifndef BUILD_SHARED_LIBS
			Log::info("Registering plugin ...");
			REGISTER_MERKLEBLOCKPLUGIN(ELA, getELAPluginComponent);
			REGISTER_MERKLEBLOCKPLUGIN(SideStandard, getIDPluginComponent);
#endif


			directory_iterator it{_dataPath};
			while (it != directory_iterator{}) {

				path temp = *it;
				if (!exists(temp) || !is_directory(temp)) {
					++it;
					continue;
				}

				std::string masterWalletID = temp.filename().string();
				if (exists((*it) / LOCAL_STORE_FILE) || exists((*it) / MASTER_WALLET_STORE_FILE)) {
					MasterWallet *masterWallet = new MasterWallet(masterWalletID, _rootPath, _dataPath, _p2pEnable, ImportFromLocalStore);

					checkRedundant(masterWallet);
					_masterWalletMap[masterWalletID] = masterWallet;
					masterWallet->InitSubWallets();
				}
				++it;
			}
		}

		std::vector<std::string> MasterWalletManager::GetAllMasterWalletID() const {
			ArgInfo("{}", GetFunName());

			std::vector<std::string> result;
			std::for_each(_masterWalletMap.begin(), _masterWalletMap.end(),
						  [&result](const MasterWalletMap::value_type &item) {
							  result.push_back(item.first);
						  });

			std::string chainID = "";
			for (size_t i = 0; i < result.size(); ++i)
				chainID += result[i] + ", ";

			ArgInfo("r => {}: {}", GetFunName(), chainID);

			return result;
		}

		IMasterWallet *MasterWalletManager::GetMasterWallet(const std::string &masterWalletId) const {
			ArgInfo("{}", GetFunName());
			ArgInfo("masterWalletID: {}", masterWalletId);

			if (_masterWalletMap.find(masterWalletId) != _masterWalletMap.cend() &&
				_masterWalletMap[masterWalletId] != nullptr) {
				ArgInfo("r => get master wallet");
				_masterWalletMap[masterWalletId]->GetBasicInfo();
				return _masterWalletMap[masterWalletId];
			}

			MasterWallet *masterWallet = new MasterWallet(masterWalletId, _rootPath, _dataPath, _p2pEnable, ImportFromLocalStore);

			checkRedundant(masterWallet);
			_masterWalletMap[masterWalletId] = masterWallet;
			masterWallet->InitSubWallets();

			ArgInfo("r => {}", GetFunName());
			ExportReadonlyWallet(masterWallet);
			masterWallet->GetBasicInfo();

			return masterWallet;
		}

		void MasterWalletManager::checkRedundant(IMasterWallet *wallet) const {

			MasterWallet *masterWallet = static_cast<MasterWallet *>(wallet);

			bool hasRedundant = false;
			std::for_each(_masterWalletMap.begin(), _masterWalletMap.end(),
						  [masterWallet, &hasRedundant](const MasterWalletMap::value_type &item) {
							  if (item.second != nullptr) {
								  const MasterWallet *createdWallet = static_cast<const MasterWallet *>(item.second);
								  if (!hasRedundant)
									  hasRedundant = masterWallet->IsEqual(*createdWallet);
							  }
						  });

			if (hasRedundant) {
				Log::info("{} Destroying redundant wallet", masterWallet->GetWalletID());

				std::vector<ISubWallet *> subWallets = masterWallet->GetAllSubWallets();
				for (int i = 0; i < subWallets.size(); ++i) {
					masterWallet->DestroyWallet(subWallets[i]);
				}

				if (masterWallet->_initFrom == ImportFromLocalStore) {
					boost::filesystem::path filepath = _dataPath;
					filepath /= LOCAL_STORE_FILE;
					if (boost::filesystem::exists(filepath)) {
						Log::info("rename {}", filepath.string());
						boost::filesystem::rename(filepath, filepath / ".bak");
					}

					filepath = _dataPath;
					filepath /= MASTER_WALLET_STORE_FILE;
					if (boost::filesystem::exists(filepath)) {
						Log::info("rename {}", filepath.string());
						boost::filesystem::rename(filepath, filepath / ".bak");
					}
				} else {
					Log::info("Clearing local", masterWallet->GetID());
					masterWallet->RemoveLocalStore();
				}

				delete masterWallet;
			}

			ErrorChecker::CheckCondition(hasRedundant, Error::CreateMasterWalletError,
										 "Master wallet already exist.");
		}
	}
}