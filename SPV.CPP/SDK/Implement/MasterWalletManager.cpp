// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <boost/function.hpp>

#include "MasterWalletManager.h"
#include "Log.h"
#include "MasterWallet.h"
#include "ParamChecker.h"

namespace Elastos {
	namespace SDK {

		class WalletFactoryInner {
		public:
			static IMasterWallet *importWalletInternal(MasterWalletManager *masterWalletManager,
													   const std::string &masterWalletId, const std::string &language,
													   const boost::function<bool(MasterWallet *)> &walletImportFun) {
				ParamChecker::checkNotEmpty(masterWalletId);

				MasterWallet *masterWallet = new MasterWallet(masterWalletId, language);

				if (!walletImportFun(masterWallet) || !masterWallet->Initialized()) {
					delete masterWallet;
					return nullptr;
				}
				return masterWallet;
			}
		};

		MasterWalletManager::MasterWalletManager() noexcept {

		}

		MasterWalletManager::~MasterWalletManager() noexcept {

		}

		IMasterWallet *MasterWalletManager::CreateMasterWallet(const std::string &masterWalletId,
															   const std::string &phrasePassword,
															   const std::string &payPassword,
															   const std::string &language) {

			ParamChecker::checkNotEmpty(masterWalletId);
			if(_masterWalletMap.find(masterWalletId) != _masterWalletMap.end())
				return _masterWalletMap[masterWalletId];

			MasterWallet *masterWallet = new MasterWallet(masterWalletId, phrasePassword, payPassword, language);
			_masterWalletMap[masterWalletId] = masterWallet;
			return masterWallet;
		}

		std::vector<IMasterWallet *> MasterWalletManager::GetAllMasterWallets() const {
			std::vector<IMasterWallet *> result;
			for(MasterWalletMap::const_iterator it = _masterWalletMap.cbegin(); it != _masterWalletMap.cend(); ++it){
				result.push_back(it->second);
			}
			return result;
		};

		void MasterWalletManager::DestroyWallet(IMasterWallet *masterWallet) {
			ParamChecker::checkNullPointer(masterWallet);

			if(_masterWalletMap.find(masterWallet->GetId()) == _masterWalletMap.end())
				return;

			_masterWalletMap.erase(masterWallet->GetId());
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
			if(_masterWalletMap.find(masterWalletId) != _masterWalletMap.end())
				return _masterWalletMap[masterWalletId];

			return WalletFactoryInner::importWalletInternal(this, masterWalletId, "english",
															[&keystorePath, &backupPassword, &payPassword, &phrasePassword](
																	MasterWallet *masterWallet) {
																return masterWallet->importFromKeyStore(keystorePath,
																										backupPassword,
																										payPassword,
																										phrasePassword);
															});
		}

		IMasterWallet *
		MasterWalletManager::ImportWalletWithMnemonic(const std::string &masterWalletId, const std::string &mnemonic,
													  const std::string &phrasePassword, const std::string &payPassword,
													  const std::string &language) {
			ParamChecker::checkPasswordWithNullLegal(phrasePassword);
			ParamChecker::checkPassword(payPassword);
			ParamChecker::checkNotEmpty(masterWalletId);
			if(_masterWalletMap.find(masterWalletId) != _masterWalletMap.end())
				return _masterWalletMap[masterWalletId];

			return WalletFactoryInner::importWalletInternal(this, masterWalletId, language,
															[&mnemonic, &phrasePassword, &payPassword](
																	MasterWallet *masterWallet) {
																return masterWallet->importFromMnemonic(mnemonic,
																										phrasePassword,
																										payPassword);
															});
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
				throw std::logic_error("Password is wrong.");
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
	}
}