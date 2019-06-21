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

		MasterWalletManager::MasterWalletManager(const std::string &rootPath) :
				_rootPath(rootPath),
				_p2pEnable(true) {
			ErrorChecker::CheckParamNotEmpty(rootPath, "rootPath");
			initMasterWallets();
		}

		MasterWalletManager::MasterWalletManager(const MasterWalletMap &walletMap, const std::string &rootPath) :
				_masterWalletMap(walletMap),
				_rootPath(rootPath),
				_p2pEnable(true) {
		}

		MasterWalletManager::~MasterWalletManager() {
			std::vector<std::string> masterWalletIds;
			std::for_each(_masterWalletMap.begin(), _masterWalletMap.end(),
						  [&masterWalletIds](const MasterWalletMap::value_type &item) {
							  masterWalletIds.push_back(item.first);
						  });
			std::for_each(masterWalletIds.begin(), masterWalletIds.end(), [this](const std::string &id) {
				this->removeWallet(id);
			});
		}

		void MasterWalletManager::SaveConfigs() {
			std::for_each(_masterWalletMap.begin(), _masterWalletMap.end(),
						  [](const MasterWalletMap::value_type &item) {
							  MasterWallet *masterWallet = static_cast<MasterWallet *>(item.second);
							  masterWallet->Save();
						  });
		}

		std::string MasterWalletManager::GenerateMnemonic(const std::string &language) const {
			return MasterWallet::GenerateMnemonic(language, _rootPath);
		}

		std::string MasterWalletManager::GetMultiSignPubKey(const std::string &phrase, const std::string &phrasePassword) const {
			ErrorChecker::CheckPasswordWithNullLegal(phrasePassword, "Phrase");

			Mnemonic mnemonic = Mnemonic(boost::filesystem::path(_rootPath));

			uint512 seed = mnemonic.DeriveSeed(phrase, phrasePassword);

			HDSeed hdseed(seed.bytes());
			HDKeychain rootKey(hdseed.getExtendedKey(true));

			bytes_t pubkey = rootKey.getChild("1'/0").pubkey();

			seed = 0;

			return pubkey.getHex();
		}

		std::string MasterWalletManager::GetMultiSignPubKey(const std::string &privKey) const {
			bytes_t prvkey(privKey);

			ErrorChecker::CheckCondition(prvkey.size() != 32, Error::PubKeyFormat,
										 "Private key length do not as expected");
			Key key(prvkey);

			prvkey.clean();

			return key.PubKey().getHex();
		}

		IMasterWallet *MasterWalletManager::CreateMasterWallet(
				const std::string &masterWalletId,
				const std::string &mnemonic,
				const std::string &phrasePassword,
				const std::string &payPassword,
				bool singleAddress) {

			ErrorChecker::CheckParamNotEmpty(masterWalletId, "Master wallet ID");
			ErrorChecker::CheckParamNotEmpty(mnemonic, "mnemonic");
			ErrorChecker::CheckPassword(payPassword, "Pay");
			ErrorChecker::CheckPasswordWithNullLegal(phrasePassword, "Phrase");

			if (_masterWalletMap.find(masterWalletId) != _masterWalletMap.end())
				return _masterWalletMap[masterWalletId];

			MasterWallet *masterWallet = new MasterWallet(masterWalletId, mnemonic, phrasePassword, payPassword,
														  singleAddress, _p2pEnable, _rootPath, CreateNormal);
			checkRedundant(masterWallet);
			_masterWalletMap[masterWalletId] = masterWallet;

			return masterWallet;
		}

		IMasterWallet *MasterWalletManager::CreateMultiSignMasterWallet(const std::string &masterWalletId,
																		const nlohmann::json &coSigners,
																		uint32_t requiredSignCount) {
			ErrorChecker::CheckParamNotEmpty(masterWalletId, "Master wallet ID");
			ErrorChecker::CheckPubKeyJsonArray(coSigners, requiredSignCount, "Signers");

			if (_masterWalletMap.find(masterWalletId) != _masterWalletMap.end())
				return _masterWalletMap[masterWalletId];

			MasterWallet *masterWallet = new MasterWallet(masterWalletId, coSigners, requiredSignCount,
														  _rootPath, _p2pEnable, CreateMultiSign);
			checkRedundant(masterWallet);
			_masterWalletMap[masterWalletId] = masterWallet;

			return masterWallet;
		}

		IMasterWallet *MasterWalletManager::CreateMultiSignMasterWallet(const std::string &masterWalletID,
																		const std::string &privKey,
																		const std::string &payPassword,
																		const nlohmann::json &coSigners,
																		uint32_t requiredSignCount) {
			ErrorChecker::CheckParamNotEmpty(masterWalletID, "Master wallet ID");
			ErrorChecker::CheckPrivateKey(privKey);
			ErrorChecker::CheckPassword(payPassword, "Pay");
			ErrorChecker::CheckPubKeyJsonArray(coSigners, requiredSignCount - 1, "Signers");

			if (_masterWalletMap.find(masterWalletID) != _masterWalletMap.end())
				return _masterWalletMap[masterWalletID];

			MasterWallet *masterWallet = new MasterWallet(masterWalletID, privKey, payPassword, coSigners,
														  requiredSignCount, _rootPath, _p2pEnable,
														  CreateMultiSign);
			checkRedundant(masterWallet);
			_masterWalletMap[masterWalletID] = masterWallet;

			return masterWallet;
		}

		IMasterWallet *MasterWalletManager::CreateMultiSignMasterWallet(const std::string &masterWalletId,
																		const std::string &mnemonic,
																		const std::string &phrasePassword,
																		const std::string &payPassword,
																		const nlohmann::json &coSigners,
																		uint32_t requiredSignCount) {
			ErrorChecker::CheckParamNotEmpty(masterWalletId, "Master wallet ID");
			ErrorChecker::CheckParamNotEmpty(mnemonic, "Mnemonic");
			ErrorChecker::CheckPassword(payPassword, "Pay");
			ErrorChecker::CheckPasswordWithNullLegal(phrasePassword, "Phrase");
			ErrorChecker::CheckPubKeyJsonArray(coSigners, requiredSignCount - 1, "Signers");

			if (_masterWalletMap.find(masterWalletId) != _masterWalletMap.end())
				return _masterWalletMap[masterWalletId];

			MasterWallet *masterWallet = new MasterWallet(masterWalletId, mnemonic, phrasePassword, payPassword,
														  coSigners, requiredSignCount, _p2pEnable,
														  _rootPath, CreateMultiSign);
			checkRedundant(masterWallet);
			_masterWalletMap[masterWalletId] = masterWallet;

			return masterWallet;
		}

		std::vector<IMasterWallet *> MasterWalletManager::GetAllMasterWallets() const {
			std::vector<IMasterWallet *> result;
			for (MasterWalletMap::const_iterator it = _masterWalletMap.cbegin(); it != _masterWalletMap.cend(); ++it) {
				if (GetMasterWallet(it->first))
					result.push_back(GetMasterWallet(it->first));
			}
			return result;
		};

		void MasterWalletManager::removeWallet(const std::string &masterWalletId, bool saveMaster) {
			ErrorChecker::CheckParamNotEmpty(masterWalletId, "Master wallet ID");

			if (_masterWalletMap.find(masterWalletId) == _masterWalletMap.end())
				return;

			Log::info("Master wallet manager remove master wallet: {}", masterWalletId);

			IMasterWallet *masterWallet = _masterWalletMap[masterWalletId];

			MasterWallet *masterWalletInner = static_cast<MasterWallet *>(masterWallet);
			if (saveMaster) {
				std::vector<ISubWallet *> subWallets = masterWallet->GetAllSubWallets();
				for (int i = 0; i < subWallets.size(); ++i) {
					Log::info("Destroying sub wallets: {}", subWallets[i]->GetChainID());
					masterWallet->DestroyWallet(subWallets[i]);
				}
			} else {
				std::vector<ISubWallet *> subWallets = masterWallet->GetAllSubWallets();
				for (int i = 0; i < subWallets.size(); ++i) {
					Log::info("Destroying sub wallets: {}", subWallets[i]->GetChainID());
					masterWallet->DestroyWallet(subWallets[i]);
				}

				Log::info("{} Clearing local", masterWalletId);
				masterWalletInner->ClearLocal();
			}


			Log::info("Removing master wallet {} from map", masterWalletId);
			_masterWalletMap.erase(masterWalletId);

			Log::info("Deleting master wallet");
			delete masterWallet;
		}

		void MasterWalletManager::DestroyWallet(const std::string &masterWalletId) {
			removeWallet(masterWalletId, false);
		}

		IMasterWallet *
		MasterWalletManager::ImportWalletWithKeystore(const std::string &masterWalletID,
													  const nlohmann::json &keystoreContent,
													  const std::string &backupPassword,
													  const std::string &payPassword) {
			ErrorChecker::CheckParamNotEmpty(masterWalletID, "Master wallet ID");
			ErrorChecker::CheckParam(!keystoreContent.is_object(), Error::KeyStore, "key store should be json object");
			ErrorChecker::CheckPassword(backupPassword, "Backup");
			ErrorChecker::CheckPassword(payPassword, "Pay");

			if (_masterWalletMap.find(masterWalletID) != _masterWalletMap.end())
				return _masterWalletMap[masterWalletID];


			MasterWallet *masterWallet = new MasterWallet(masterWalletID, keystoreContent, backupPassword,
														  payPassword, _rootPath, _p2pEnable,
														  ImportFromKeyStore);
			checkRedundant(masterWallet);
			_masterWalletMap[masterWalletID] = masterWallet;
			masterWallet->InitSubWallets();
			return masterWallet;
		}

		IMasterWallet *
		MasterWalletManager::ImportWalletWithMnemonic(const std::string &masterWalletId, const std::string &mnemonic,
													  const std::string &phrasePassword, const std::string &payPassword,
													  bool singleAddress) {
			ErrorChecker::CheckParamNotEmpty(masterWalletId, "Master wallet ID");
			ErrorChecker::CheckParamNotEmpty(mnemonic, "Mnemonic");
			ErrorChecker::CheckPasswordWithNullLegal(phrasePassword, "Phrase");
			ErrorChecker::CheckPassword(payPassword, "Pay");

			if (_masterWalletMap.find(masterWalletId) != _masterWalletMap.end())
				return _masterWalletMap[masterWalletId];

			MasterWallet *masterWallet = new MasterWallet(masterWalletId, mnemonic, phrasePassword, payPassword,
														  singleAddress, _p2pEnable, _rootPath,
														  ImportFromMnemonic);
			checkRedundant(masterWallet);
			_masterWalletMap[masterWalletId] = masterWallet;
			return masterWallet;
		}

		IMasterWallet *MasterWalletManager::ImportReadonlyWallet(
			const std::string &masterWalletID,
			const nlohmann::json &walletJson) {
			ErrorChecker::CheckParam(!walletJson.is_object(), Error::KeyStore, "wallet json should be json object");

			if (_masterWalletMap.find(masterWalletID) != _masterWalletMap.end())
				return _masterWalletMap[masterWalletID];

			MasterWallet *masterWallet = new MasterWallet(masterWalletID, walletJson, _rootPath, _p2pEnable, ImportFromKeyStore);

			checkRedundant(masterWallet);
			_masterWalletMap[masterWalletID] = masterWallet;
			masterWallet->InitSubWallets();

			return masterWallet;
		}

		nlohmann::json
		MasterWalletManager::ExportWalletWithKeystore(IMasterWallet *masterWallet, const std::string &backupPassword,
													  const std::string &payPassword) const {

			ErrorChecker::CheckParam(masterWallet == nullptr, Error::InvalidArgument, "Master wallet is null");
			ErrorChecker::CheckPassword(backupPassword, "Backup");

			MasterWallet *wallet = static_cast<MasterWallet *>(masterWallet);
			return wallet->exportKeyStore(backupPassword, payPassword);
		}

		std::string
		MasterWalletManager::ExportWalletWithMnemonic(IMasterWallet *masterWallet, const std::string &payPassword) const {

			ErrorChecker::CheckParam(masterWallet == nullptr, Error::InvalidArgument, "Master wallet is null");
			ErrorChecker::CheckPassword(payPassword, "Pay");

			MasterWallet *wallet = static_cast<MasterWallet *>(masterWallet);

			return wallet->exportMnemonic(payPassword);
		}

		nlohmann::json MasterWalletManager::ExportReadonlyWallet(
			IMasterWallet *masterWallet) const {
			ErrorChecker::CheckParam(masterWallet == nullptr, Error::InvalidArgument, "master wallet is null");
			MasterWallet *wallet = static_cast<MasterWallet *>(masterWallet);

			return wallet->ExportReadonlyKeyStore();
		}

		std::string MasterWalletManager::GetVersion() const {
			return SPVSDK_VERSION_MESSAGE;
		}

		void MasterWalletManager::initMasterWallets() {
			path rootPath = _rootPath;

			Log::registerMultiLogger(rootPath.string());

			Log::setLevel(spdlog::level::from_str(SPVSDK_SPDLOG_LEVEL));
			Log::info("spvsdk version {}", SPVSDK_VERSION_MESSAGE);

#ifndef BUILD_SHARED_LIBS
			Log::info("Registering plugin ...");
			REGISTER_MERKLEBLOCKPLUGIN(ELA, getELAPluginComponent);
			REGISTER_MERKLEBLOCKPLUGIN(SideStandard, getIDPluginComponent);
#endif

			ErrorChecker::CheckPathExists(rootPath);

			directory_iterator it{rootPath};
			while (it != directory_iterator{}) {

				path temp = *it;
				if (!exists(temp) || !is_directory(temp)) {
					++it;
					continue;
				}

				std::string masterWalletId = temp.filename().string();
				if (exists((*it) / LOCAL_STORE_FILE) || exists((*it) / MASTER_WALLET_STORE_FILE)) {
					GetMasterWallet(masterWalletId);
				}
				++it;
			}

			if (_masterWalletMap.size() > 0)
				Log::info("{} master wallets were loaded from local store", _masterWalletMap.size());
		}

		std::vector<std::string> MasterWalletManager::GetAllMasterWalletIds() const {
			std::vector<std::string> result;
			std::for_each(_masterWalletMap.begin(), _masterWalletMap.end(),
						  [&result](const MasterWalletMap::value_type &item) {
							  result.push_back(item.first);
						  });
			return result;
		}

		IMasterWallet *MasterWalletManager::GetMasterWallet(const std::string &masterWalletId) const {
			if (_masterWalletMap.find(masterWalletId) != _masterWalletMap.cend() &&
				_masterWalletMap[masterWalletId] != nullptr)
				return _masterWalletMap[masterWalletId];

			MasterWallet *masterWallet = new MasterWallet(masterWalletId, _rootPath, _p2pEnable, ImportFromLocalStore);

			checkRedundant(masterWallet);
			_masterWalletMap[masterWalletId] = masterWallet;
			masterWallet->InitSubWallets();
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
				Log::info("Destroying sub wallets.");
				std::vector<ISubWallet *> subWallets = masterWallet->GetAllSubWallets();
				for (int i = 0; i < subWallets.size(); ++i) {
					masterWallet->DestroyWallet(subWallets[i]);
				}

				Log::info("({}) Clearing local.", masterWallet->GetId());
				masterWallet->ClearLocal();
				delete masterWallet;
			}

			ErrorChecker::CheckCondition(hasRedundant, Error::CreateMasterWalletError,
										 "Master wallet already exist.");
		}
	}
}