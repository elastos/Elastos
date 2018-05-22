// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "WalletFactory.h"
#include "MasterWallet.h"
#include "Log.h"

namespace Elastos {
	namespace SDK {

		WalletFactory::WalletFactory() {

		}

		WalletFactory::~WalletFactory() {

		}

		IMasterWallet *WalletFactory::CreateMasterWallet(const std::string &name, const std::string &phrasePassword,
														 const std::string &payPassword) {
			if (_masterWallets.find(name) != _masterWallets.end())
				return _masterWallets[name];

			MasterWallet *masterWallet = new MasterWallet(name, phrasePassword, payPassword);
			_masterWallets[name] = masterWallet;
			return masterWallet;
		}

		void WalletFactory::DestroyWallet(IMasterWallet *masterWallet) {
			_masterWallets.erase(std::find_if(_masterWallets.begin(), _masterWallets.end(),
											  [masterWallet](const MasterWalletMap::value_type &item) {
												  return item.second == masterWallet;
											  }));
		}

		IMasterWallet *
		WalletFactory::ImportWalletWithKeystore(const std::string &keystorePath, const std::string &backupPassword,
												const std::string &payPassword) {
			return importWalletInternal([&keystorePath, &backupPassword, &payPassword](MasterWallet *masterWallet) {
				return masterWallet->importFromKeyStore(keystorePath, backupPassword, payPassword);
			});
		}

		IMasterWallet *
		WalletFactory::ImportWalletWithMnemonic(const std::string &mnemonic, const std::string &phrasePassword,
												const std::string &payPassword) {
			return importWalletInternal([&mnemonic, &phrasePassword, &payPassword](MasterWallet *masterWallet) {
				return masterWallet->importFromMnemonic(mnemonic, phrasePassword, payPassword);
			});
		}

		void WalletFactory::ExportWalletWithKeystore(IMasterWallet *masterWallet, const std::string &backupPassword,
													 const std::string &keystorePath) {
			MasterWallet *wallet = static_cast<MasterWallet *>(masterWallet);
			if (wallet->Initialized() || _masterWallets.find(wallet->GetName()) == _masterWallets.end()) {
				Log::warn("Exporting failed, check if the wallet has been initialized.");
				return;
			}

			wallet->exportKeyStore(backupPassword, keystorePath);
		}

		std::string
		WalletFactory::ExportWalletWithMnemonic(IMasterWallet *masterWallet, const std::string &phrasePassword) {
			MasterWallet *wallet = static_cast<MasterWallet *>(masterWallet);
			if (wallet->Initialized() || _masterWallets.find(wallet->GetName()) == _masterWallets.end()) {
				Log::warn("Exporting failed, check if the wallet has been initialized.");
				return "";
			}

			std::string mnemonic;
			if (wallet->exportMnemonic(phrasePassword, mnemonic)) {
				return mnemonic;
			}
			return "";
		}

		IMasterWallet *
		WalletFactory::importWalletInternal(const boost::function<bool(MasterWallet *)> &walletImportFun) {
			MasterWallet *masterWallet = new MasterWallet();

			if (!walletImportFun(masterWallet) || !masterWallet->Initialized()) {
				delete masterWallet;
				return nullptr;
			}

			if (_masterWallets.find(masterWallet->GetName()) != _masterWallets.end()) {
				Log::info("Importing wallet already exist.");
				std::string walletName = masterWallet->GetName();
				delete masterWallet; //if we found master wallet, we just delete the new one.
				return _masterWallets[walletName];
			}
			_masterWallets[masterWallet->GetName()] = masterWallet;
		}
	}
}