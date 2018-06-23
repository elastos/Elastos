// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <boost/function.hpp>
#include <boost/filesystem.hpp>

#include "MasterWalletManager.h"
#include "Log.h"
#include "MasterWallet.h"
#include "ParamChecker.h"

using namespace boost::filesystem;

#define MASTER_WALLET_STORE_FILE "MasterWalletStore.json"

namespace Elastos {
	namespace ElaWallet {

		class WalletFactoryInner {
		public:
			static IMasterWallet *importWalletInternal(const std::string &masterWalletId, const std::string &language,
													   const std::string &rootPath,
													   const boost::function<bool(MasterWallet *)> &walletImportFun) {
				ParamChecker::checkNotEmpty(masterWalletId);

				MasterWallet *masterWallet = new MasterWallet(masterWalletId, language, rootPath);

				if (!walletImportFun(masterWallet) || !masterWallet->Initialized()) {
					delete masterWallet;
					return nullptr;
				}
				return masterWallet;
			}
		};

		MasterWalletManager::MasterWalletManager(const std::string &rootPath) :
			_rootPath(rootPath) {
			initMasterWallets();
		}

		MasterWalletManager::MasterWalletManager(const MasterWalletMap &walletMap, const std::string &rootPath) :
				_masterWalletMap(walletMap),
				_rootPath(rootPath){
		}

		MasterWalletManager::~MasterWalletManager() {
		}

		void MasterWalletManager::SaveConfigs() {
			std::for_each(_masterWalletMap.begin(), _masterWalletMap.end(),
						  [](const MasterWalletMap::value_type &item) {
							  MasterWallet *masterWallet = static_cast<MasterWallet *>(item.second);
							  masterWallet->Save();
						  });
		}

		IMasterWallet *
		MasterWalletManager::CreateMasterWallet(const std::string &masterWalletId, const std::string &language) {
			ParamChecker::checkNotEmpty(masterWalletId);
			if (_masterWalletMap.find(masterWalletId) != _masterWalletMap.end())
				return _masterWalletMap[masterWalletId];

			MasterWallet *masterWallet = new MasterWallet(masterWalletId, language, _rootPath);
			_masterWalletMap[masterWalletId] = masterWallet;
			return masterWallet;
		}

		bool MasterWalletManager::InitializeMasterWallet(const std::string &masterWalletId,
														 const std::string &mnemonic,
														 const std::string &phrasePassword,
														 const std::string &payPassword) {

			ParamChecker::checkNotEmpty(masterWalletId);
			if (_masterWalletMap.find(masterWalletId) == _masterWalletMap.end())
				throw std::invalid_argument("Unknown master wallet id.");

			MasterWallet *masterWallet = static_cast<MasterWallet *>(_masterWalletMap[masterWalletId]);
			if (masterWallet->Initialized())
				return false;
			return masterWallet->importFromMnemonic(mnemonic, phrasePassword, payPassword);
		}

		std::vector<IMasterWallet *> MasterWalletManager::GetAllMasterWallets() const {
			std::vector<IMasterWallet *> result;
			for (MasterWalletMap::const_iterator it = _masterWalletMap.cbegin(); it != _masterWalletMap.cend(); ++it) {
				result.push_back(it->second);
			}
			return result;
		};

		void MasterWalletManager::DestroyWallet(const std::string &masterWalletId) {
			ParamChecker::checkNotEmpty(masterWalletId);

			if (_masterWalletMap.find(masterWalletId) == _masterWalletMap.end())
				return;

			IMasterWallet *masterWallet = _masterWalletMap[masterWalletId];

			MasterWallet *masterWalletInner = static_cast<MasterWallet *>(masterWallet);
			if(masterWalletInner->Initialized()) {
				std::vector<ISubWallet *> subWallets = masterWallet->GetAllSubWallets();
				for (int i = 0; i < subWallets.size(); ++i) {
					masterWallet->DestroyWallet(subWallets[i]);
				}
			}

			 _masterWalletMap.erase(masterWalletId);
			delete masterWallet;
		}

		IMasterWallet *
		MasterWalletManager::ImportWalletWithKeystore(const std::string &masterWalletId,
													  const nlohmann::json &keystoreContent,
													  const std::string &backupPassword,
													  const std::string &payPassword,
													  const std::string &phrasePassword) {
			ParamChecker::checkPassword(backupPassword, "Backup");
			ParamChecker::checkPassword(payPassword, "Pay");
			ParamChecker::checkNotEmpty(masterWalletId);
			if (_masterWalletMap.find(masterWalletId) != _masterWalletMap.end())
				return _masterWalletMap[masterWalletId];


			IMasterWallet *masterWallet = NULL;
			masterWallet = WalletFactoryInner::importWalletInternal(masterWalletId, "english", _rootPath,
																	[&keystoreContent, &backupPassword, &payPassword, &phrasePassword](
																			MasterWallet *masterWallet) {
																		return masterWallet->importFromKeyStore(
																				keystoreContent,
																				backupPassword,
																				payPassword,
																				phrasePassword);
																	});
			ParamChecker::checkNullPointer(masterWallet);
			_masterWalletMap[masterWalletId] = masterWallet;
			return masterWallet;
		}

		IMasterWallet *
		MasterWalletManager::ImportWalletWithMnemonic(const std::string &masterWalletId, const std::string &mnemonic,
													  const std::string &phrasePassword, const std::string &payPassword,
													  const std::string &language) {
			ParamChecker::checkPasswordWithNullLegal(phrasePassword, "Phrase");
			ParamChecker::checkPassword(payPassword, "Pay");
			ParamChecker::checkNotEmpty(masterWalletId);
			if (_masterWalletMap.find(masterWalletId) != _masterWalletMap.end())
				return _masterWalletMap[masterWalletId];

			IMasterWallet *masterWallet = NULL;

			masterWallet = WalletFactoryInner::importWalletInternal(masterWalletId, language, _rootPath,
																	[&mnemonic, &phrasePassword, &payPassword](
																			MasterWallet *masterWallet) {
																		return masterWallet->importFromMnemonic(
																				mnemonic,
																				phrasePassword,
																				payPassword);
																	});
			ParamChecker::checkNullPointer(masterWallet);
			_masterWalletMap[masterWalletId] = masterWallet;
			return masterWallet;
		}

		nlohmann::json
		MasterWalletManager::ExportWalletWithKeystore(IMasterWallet *masterWallet, const std::string &backupPassword,
													  const std::string &payPassword) {

			ParamChecker::checkPassword(backupPassword, "Backup");
			ParamChecker::checkPassword(payPassword, "Pay");

			MasterWallet *wallet = static_cast<MasterWallet *>(masterWallet);
			if (!wallet->Initialized()) {
				Log::warn("Exporting failed, check if the wallet has been initialized.");
				return nlohmann::json();
			}

			return wallet->exportKeyStore(backupPassword, payPassword);
		}

		std::string
		MasterWalletManager::ExportWalletWithMnemonic(IMasterWallet *masterWallet, const std::string &payPassword) {

			ParamChecker::checkNullPointer(masterWallet);
			ParamChecker::checkPassword(payPassword, "Pay");

			MasterWallet *wallet = static_cast<MasterWallet *>(masterWallet);
			if (!wallet->Initialized()) {
				throw std::logic_error("Exporting failed, check if the wallet has been initialized.");
			}

			std::string mnemonic;
			if (!wallet->exportMnemonic(payPassword, mnemonic)) {
				throw std::logic_error("Password is wrong.");
			}
			return mnemonic;
		}

		void MasterWalletManager::initMasterWallets() {
			path rootPath = _rootPath;

			directory_iterator it{rootPath};
			while (it != directory_iterator{}) {

				path temp = *it;
				if (!exists(temp) || !is_directory(temp)) {
					++it;
					continue;
				}

				std::string masterWalletId = temp.filename().string();
				temp /= MASTER_WALLET_STORE_FILE;
				if (exists(temp)) {
					MasterWallet *masterWallet = new MasterWallet(temp, _rootPath);
					_masterWalletMap[masterWalletId] = masterWallet;
				}
				++it;
			}
		}

	}
}