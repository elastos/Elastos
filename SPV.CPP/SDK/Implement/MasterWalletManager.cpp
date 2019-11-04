// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "MasterWallet.h"

#include <SpvService/Config.h>
#include <Common/Log.h>
#include <Common/ErrorChecker.h>
#include <WalletCore/Mnemonic.h>
#include <WalletCore/Base58.h>
#include <Plugin/Registry.h>
#include <Plugin/ELAPlugin.h>
#include <Plugin/IDPlugin.h>

#include <MasterWalletManager.h>
#include <CMakeConfig.h>

#include <boost/filesystem.hpp>

using namespace boost::filesystem;

#define MASTER_WALLET_STORE_FILE "MasterWalletStore.json"
#define LOCAL_STORE_FILE "LocalStore.json"

namespace Elastos {
	namespace ElaWallet {

		MasterWalletManager::MasterWalletManager(const std::string &rootPath, const std::string &netType,
												 const nlohmann::json &config, const std::string &dataPath) :
			_rootPath(rootPath),
			_dataPath(dataPath),
			_p2pEnable(true) {

			if (_dataPath.empty())
				_dataPath = _rootPath;

			ErrorChecker::CheckPathExists(_rootPath);
			ErrorChecker::CheckPathExists(_dataPath);

			Log::registerMultiLogger(_dataPath);

			Log::setLevel(spdlog::level::from_str(SPVSDK_SPDLOG_LEVEL));
			Log::info("spvsdk version {}", SPVSDK_VERSION_MESSAGE);

#ifdef SPV_ENABLE_STATIC
			Log::info("Registering plugin ...");
			REGISTER_MERKLEBLOCKPLUGIN(ELA, getELAPluginComponent);
			REGISTER_MERKLEBLOCKPLUGIN(IDChain, getIDPluginComponent);
#endif
			if (netType != CONFIG_MAINNET && netType != CONFIG_TESTNET &&
				netType != CONFIG_REGTEST && netType != CONFIG_PRVNET) {
				ErrorChecker::ThrowParamException(Error::InvalidArgument, "invalid NetType");
			}

			_config = new Config(_dataPath, netType, config);
			if (_config->GetNetType() != CONFIG_MAINNET) {
				_dataPath = _dataPath + "/" + _config->GetNetType();
				if (!boost::filesystem::exists(_dataPath))
					boost::filesystem::create_directory(_dataPath);
			}

			Load();
		}

		MasterWalletManager::MasterWalletManager(const MasterWalletMap &walletMap, const std::string &rootPath,
												 const std::string &dataPath) :
			_masterWalletMap(walletMap),
			_rootPath(rootPath),
			_dataPath(dataPath),
			_p2pEnable(false) {

			if (_dataPath.empty())
				_dataPath = _rootPath;

			ErrorChecker::CheckPathExists(_rootPath);
			ErrorChecker::CheckPathExists(_dataPath);

			Log::registerMultiLogger(_dataPath);

			Log::setLevel(spdlog::level::from_str(SPVSDK_SPDLOG_LEVEL));
			Log::info("spvsdk version {}", SPVSDK_VERSION_MESSAGE);

#ifdef SPV_ENABLE_STATIC
			Log::info("Registering plugin ...");
			REGISTER_MERKLEBLOCKPLUGIN(ELA, getELAPluginComponent);
			REGISTER_MERKLEBLOCKPLUGIN(IDChain, getIDPluginComponent);
#endif

			_config = new Config(_dataPath, CONFIG_MAINNET);

			if (_config->GetNetType() != CONFIG_MAINNET)
				_dataPath = _dataPath + "/" + _config->GetNetType();

			Load();
		}

		MasterWalletManager::~MasterWalletManager() {
			for (MasterWalletMap::iterator it = _masterWalletMap.begin(); it != _masterWalletMap.end();) {
				MasterWallet *masterWallet = static_cast<MasterWallet *>(it->second);
				std::string id = masterWallet->GetID();
				Log::info("closing master wallet (ID = {})...", id);
				masterWallet->CloseAllSubWallets();
				it = _masterWalletMap.erase(it);

				delete masterWallet;
				masterWallet = nullptr;
				Log::info("closed master wallet (ID = {})", id);
			}
			delete _config;
			_config = nullptr;
		}

		void MasterWalletManager::Load() {
			directory_iterator it{_dataPath};
			while (it != directory_iterator{}) {

				path temp = *it;
				if (!exists(temp) || !is_directory(temp)) {
					++it;
					continue;
				}

				std::string masterWalletID = temp.filename().string();
				if (exists((*it) / LOCAL_STORE_FILE) || exists((*it) / MASTER_WALLET_STORE_FILE)) {
					Log::info("loading {}", masterWalletID);
					MasterWallet *masterWallet = new MasterWallet(masterWalletID, ConfigPtr(new Config(*_config)),
																  _dataPath, _p2pEnable,
																  ImportFromLocalStore);

					checkRedundant(masterWallet);
					_masterWalletMap[masterWalletID] = masterWallet;
					masterWallet->InitSubWallets();
				}
				++it;
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

			Mnemonic m(_rootPath);
			ErrorChecker::CheckLogic(!m.Validate(mnemonic), Error::Mnemonic, "Invalid mnemonic");

			MasterWallet *masterWallet = new MasterWallet(masterWalletId, mnemonic, phrasePassword, payPassword,
														  singleAddress, _p2pEnable, ConfigPtr(new Config(*_config)),
														  _dataPath, 0, CreateNormal);
			checkRedundant(masterWallet);
			_masterWalletMap[masterWalletId] = masterWallet;

			ArgInfo("r => create master wallet");

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
				ErrorChecker::CheckCondition(!(*it).is_string(), Error::Code::PubKeyFormat,
											 "cosigners should be string");
				std::string xpub = (*it).get<std::string>();
				for (int i = 0; i < pubKeyRing.size(); ++i) {
					if (pubKeyRing[i].GetxPubKey() == xpub) {
						ErrorChecker::ThrowParamException(Error::PubKeyFormat, "Contain same xpub");
					}
				}
				pubKeyRing.emplace_back("", xpub);
			}

			if (_masterWalletMap.find(masterWalletID) != _masterWalletMap.end()) {
				ArgInfo("r => already exist");
				return _masterWalletMap[masterWalletID];
			}

			MasterWallet *masterWallet = new MasterWallet(masterWalletID, pubKeyRing, m,
														  ConfigPtr(new Config(*_config)), _dataPath,
														  _p2pEnable, singleAddress, compatible,
														  timestamp, CreateMultiSign);
			checkRedundant(masterWallet);
			_masterWalletMap[masterWalletID] = masterWallet;

			ArgInfo("r => create multi sign wallet");

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
				ErrorChecker::CheckCondition(!(*it).is_string(), Error::Code::PubKeyFormat,
											 "cosigners should be string");
				std::string xpub = (*it).get<std::string>();
				for (int i = 0; i < pubKeyRing.size(); ++i) {
					if (pubKeyRing[i].GetxPubKey() == xpub) {
						ErrorChecker::ThrowParamException(Error::PubKeyFormat, "Contain same xpub");
					}
				}
				pubKeyRing.emplace_back("", xpub);
			}

			if (_masterWalletMap.find(masterWalletID) != _masterWalletMap.end()) {
				ArgInfo("r => already exist");
				return _masterWalletMap[masterWalletID];
			}

			MasterWallet *masterWallet = new MasterWallet(masterWalletID, xprv, payPassword, pubKeyRing,
														  m, ConfigPtr(new Config(*_config)), _dataPath, _p2pEnable,
														  singleAddress,
														  compatible,
														  timestamp, CreateMultiSign);
			checkRedundant(masterWallet);
			_masterWalletMap[masterWalletID] = masterWallet;

			ArgInfo("r => create multi sign wallet");

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
				std::string xpub = (*it).get<std::string>();
				for (int i = 0; i < pubKeyRing.size(); ++i) {
					if (pubKeyRing[i].GetxPubKey() == xpub) {
						ErrorChecker::ThrowParamException(Error::PubKeyFormat, "Contain same xpub");
					}
				}
				pubKeyRing.emplace_back("", xpub);
			}

			MasterWallet *masterWallet = new MasterWallet(masterWalletID, mnemonic, passphrase, payPassword,
														  pubKeyRing, m, ConfigPtr(new Config(*_config)), _dataPath,
														  _p2pEnable,
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
				masterWallet = nullptr;
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
														  payPassword, ConfigPtr(new Config(*_config)), _dataPath,
														  _p2pEnable,
														  ImportFromKeyStore);
			checkRedundant(masterWallet);
			_masterWalletMap[masterWalletID] = masterWallet;
			masterWallet->InitSubWallets();

			ArgInfo("r => import with keystore");

			return masterWallet;
		}

		IMasterWallet *MasterWalletManager::ImportWalletWithMnemonic(const std::string &masterWalletId,
																	 const std::string &mnemonic,
																	 const std::string &phrasePassword,
																	 const std::string &payPassword,
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

			Mnemonic m(_rootPath);
			ErrorChecker::CheckLogic(!m.Validate(mnemonic), Error::Mnemonic, "Invalid mnemonic");

			MasterWallet *masterWallet = new MasterWallet(masterWalletId, mnemonic, phrasePassword, payPassword,
														  singleAddress, _p2pEnable, ConfigPtr(new Config(*_config)),
														  _dataPath, timestamp, ImportFromMnemonic);
			checkRedundant(masterWallet);
			_masterWalletMap[masterWalletId] = masterWallet;

			ArgInfo("r => import with mnemonic");

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

			MasterWallet *masterWallet = new MasterWallet(masterWalletID, walletJson, ConfigPtr(new Config(*_config)),
														  _dataPath, _p2pEnable, ImportFromKeyStore);

			checkRedundant(masterWallet);
			_masterWalletMap[masterWalletID] = masterWallet;
			masterWallet->InitSubWallets();
			ArgInfo("r => import read-only");

			return masterWallet;
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

			MasterWallet *masterWallet = new MasterWallet(masterWalletId, ConfigPtr(new Config(*_config)), _dataPath,
														  _p2pEnable,
														  ImportFromLocalStore);

			checkRedundant(masterWallet);
			_masterWalletMap[masterWalletId] = masterWallet;
			masterWallet->InitSubWallets();

			ArgInfo("r => {}", GetFunName());

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
				masterWallet = nullptr;
			}

			ErrorChecker::CheckCondition(hasRedundant, Error::CreateMasterWalletError,
										 "Master wallet already exist.");
		}
	}
}