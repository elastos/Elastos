// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <boost/function.hpp>
#include <boost/filesystem.hpp>

#include "MasterWalletManager.h"
#include "Log.h"
#include "MasterWallet.h"
#include "ParamChecker.h"
#include "Enviroment.h"

using namespace boost::filesystem;

#define MASTER_WALLET_STORE_FILE "MasterWalletStore.json"

namespace Elastos {
	namespace ElaWallet {

		class WalletFactoryInner {
		public:
			static IMasterWallet *importWalletInternal(const std::string &masterWalletId, const std::string &language,
													   const boost::function<bool(MasterWallet *)> &walletImportFun) {
				ParamChecker::checkNotEmpty(masterWalletId);
				//ParamChecker::checkNullPointer(masterWalletManager);

				MasterWallet *masterWallet = new MasterWallet(masterWalletId, language);

				if (!walletImportFun(masterWallet) || !masterWallet->Initialized()) {
					delete masterWallet;
					return nullptr;
				}
				return masterWallet;
			}
		};

		MasterWalletManager::MasterWalletManager() {
			initMasterWallets();
		}

		MasterWalletManager::MasterWalletManager(const MasterWalletMap &walletMap) :
				_masterWalletMap(walletMap) {
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

			MasterWallet *masterWallet = new MasterWallet(masterWalletId, language);
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
			_masterWalletMap.erase(masterWalletId);
			delete masterWallet;
		}

		IMasterWallet *
		MasterWalletManager::ImportWalletWithKeystore(const std::string &masterWalletId,
													  const std::string &keystorePath,
													  const std::string &backupPassword, const std::string &payPassword,
													  const std::string &phrasePassword) {
			ParamChecker::checkPassword(backupPassword);
			ParamChecker::checkPassword(payPassword);
			ParamChecker::checkNotEmpty(masterWalletId);
			if (_masterWalletMap.find(masterWalletId) != _masterWalletMap.end())
				return _masterWalletMap[masterWalletId];


			IMasterWallet* masterWallet = NULL;
			masterWallet = WalletFactoryInner::importWalletInternal( masterWalletId, "english",
																	[&keystorePath, &backupPassword, &payPassword, &phrasePassword](
																		MasterWallet *masterWallet) {
																		return masterWallet->importFromKeyStore(keystorePath,
																												backupPassword,
																												payPassword,
																												phrasePassword);
																	});
			ParamChecker::checkNullPointer(masterWallet);
			_masterWalletMap[masterWalletId] = masterWallet;
			return masterWallet ;
		}

		IMasterWallet *
		MasterWalletManager::ImportWalletWithMnemonic(const std::string &masterWalletId, const std::string &mnemonic,
													  const std::string &phrasePassword, const std::string &payPassword,
													  const std::string &language) {
			ParamChecker::checkPasswordWithNullLegal(phrasePassword);
			ParamChecker::checkPassword(payPassword);
			ParamChecker::checkNotEmpty(masterWalletId);
			if (_masterWalletMap.find(masterWalletId) != _masterWalletMap.end())
				return _masterWalletMap[masterWalletId];

			IMasterWallet* masterWallet = NULL;

			masterWallet = WalletFactoryInner::importWalletInternal( masterWalletId, language,
													 [&mnemonic, &phrasePassword, &payPassword](
														 MasterWallet *masterWallet) {
														 return masterWallet->importFromMnemonic(mnemonic,
																								 phrasePassword,
																								 payPassword);
													 });
			ParamChecker::checkNullPointer(masterWallet);
			_masterWalletMap[masterWalletId] = masterWallet;
			return masterWallet ;
		}

		void
		MasterWalletManager::ExportWalletWithKeystore(IMasterWallet *masterWallet, const std::string &backupPassword,
													  const std::string &payPassword, const std::string &keystorePath) {

			ParamChecker::checkPassword(backupPassword);
			ParamChecker::checkPassword(payPassword);

			MasterWallet *wallet = static_cast<MasterWallet *>(masterWallet);
			if (!wallet->Initialized()) {
				Log::warn("Exporting failed, check if the wallet has been initialized.");
				return;
			}

			if (!wallet->exportKeyStore(backupPassword, payPassword, keystorePath)) {
				throw std::logic_error("exportKeyStore fail");
			}
		}

		std::string
		MasterWalletManager::ExportWalletWithMnemonic(IMasterWallet *masterWallet, const std::string &payPassword) {

			ParamChecker::checkNullPointer(masterWallet);
			ParamChecker::checkPassword(payPassword);

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
			path rootPath = Enviroment::GetRootPath();

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
					MasterWallet *masterWallet = new MasterWallet(temp);
					_masterWalletMap[masterWalletId] = masterWallet;
				}
				++it;
			}
		}

	}
}