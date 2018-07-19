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

		MasterWalletManager::MasterWalletManager(const std::string &rootPath) :
				_rootPath(rootPath),
				_p2pEnable(true) {
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

		std::string MasterWalletManager::GenerateMnemonic(const std::string &language) {
			return MasterWallet::GenerateMnemonic(language, _rootPath);
		}

		IMasterWallet *MasterWalletManager::CreateMasterWallet(
				const std::string &masterWalletId,
				const std::string &mnemonic,
				const std::string &phrasePassword,
				const std::string &payPassword,
				const std::string &language) {

			ParamChecker::checkNotEmpty(masterWalletId);
			if (_masterWalletMap.find(masterWalletId) != _masterWalletMap.end())
				return _masterWalletMap[masterWalletId];

			MasterWallet *masterWallet = new MasterWallet(masterWalletId, mnemonic, phrasePassword, payPassword,
														  language, _p2pEnable, _rootPath);
			_masterWalletMap[masterWalletId] = masterWallet;

			return masterWallet;
		}

		std::vector<IMasterWallet *> MasterWalletManager::GetAllMasterWallets() const {
			std::vector<IMasterWallet *> result;
			for (MasterWalletMap::const_iterator it = _masterWalletMap.cbegin(); it != _masterWalletMap.cend(); ++it) {
				result.push_back(it->second);
			}
			return result;
		};

		void MasterWalletManager::removeWallet(const std::string &masterWalletId, bool saveMaster) {
			ParamChecker::checkNotEmpty(masterWalletId);

			if (_masterWalletMap.find(masterWalletId) == _masterWalletMap.end())
				return;

			IMasterWallet *masterWallet = _masterWalletMap[masterWalletId];

			MasterWallet *masterWalletInner = static_cast<MasterWallet *>(masterWallet);
			if (saveMaster) {
				SPDLOG_DEBUG(Log::getLogger(),"[MasterWalletManager::removeWallet] Begin save ({}).", masterWalletId);
				masterWalletInner->Save();
				SPDLOG_DEBUG(Log::getLogger(),"[MasterWalletManager::removeWallet] End save ({}).", masterWalletId);
			} else {
				SPDLOG_DEBUG(Log::getLogger(),"[MasterWalletManager::removeWallet] Begin clear local ({}).", masterWalletId);
				masterWalletInner->ClearLocal();
				SPDLOG_DEBUG(Log::getLogger(),"[MasterWalletManager::removeWallet] End clear local ({}).", masterWalletId);
			}

			SPDLOG_DEBUG(Log::getLogger(),"[MasterWalletManager::removeWallet] Begin destroy sub wallets ({}).",
								   masterWalletId);
			std::vector<ISubWallet *> subWallets = masterWallet->GetAllSubWallets();
			for (int i = 0; i < subWallets.size(); ++i) {
				masterWallet->DestroyWallet(subWallets[i]);
			}
			SPDLOG_DEBUG(Log::getLogger(),"[MasterWalletManager::removeWallet] End destroy sub wallets of ({}).",
								   masterWalletId);

			SPDLOG_DEBUG(Log::getLogger(),"[MasterWalletManager::removeWallet] Removing master wallet from map ({}).",
								   masterWalletId);
			_masterWalletMap.erase(masterWalletId);

			SPDLOG_DEBUG(Log::getLogger(),"[MasterWalletManager::removeWallet] Deleting master wallet ({}).", masterWalletId);
			delete masterWallet;
		}

		void MasterWalletManager::DestroyWallet(const std::string &masterWalletId) {
			removeWallet(masterWalletId, false);
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


			MasterWallet *masterWallet = new MasterWallet(masterWalletId, keystoreContent, backupPassword,
														  payPassword, phrasePassword, _rootPath, _p2pEnable);
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

			MasterWallet *masterWallet = new MasterWallet(masterWalletId, mnemonic, phrasePassword, payPassword,
														  language, _p2pEnable, _rootPath);
			_masterWalletMap[masterWalletId] = masterWallet;
			return masterWallet;
		}

		nlohmann::json
		MasterWalletManager::ExportWalletWithKeystore(IMasterWallet *masterWallet, const std::string &backupPassword,
													  const std::string &payPassword) {

			ParamChecker::checkPassword(backupPassword, "Backup");
			ParamChecker::checkPassword(payPassword, "Pay");

			MasterWallet *wallet = static_cast<MasterWallet *>(masterWallet);
			return wallet->exportKeyStore(backupPassword, payPassword);
		}

		std::string
		MasterWalletManager::ExportWalletWithMnemonic(IMasterWallet *masterWallet, const std::string &payPassword) {

			ParamChecker::checkNullPointer(masterWallet);
			ParamChecker::checkPassword(payPassword, "Pay");

			MasterWallet *wallet = static_cast<MasterWallet *>(masterWallet);

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
					MasterWallet *masterWallet = new MasterWallet(temp, _rootPath, _p2pEnable);
					_masterWalletMap[masterWalletId] = masterWallet;
				}
				++it;
			}
		}

	}
}