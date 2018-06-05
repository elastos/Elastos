// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <boost/function.hpp>

#include "WalletFactory.h"
#include "MasterWallet.h"
#include "Log.h"

#define MIN_PASSWORD_LENGTH 8
#define MAX_PASSWORD_LENGTH 128

namespace Elastos {
	namespace SDK {

		class WalletFactoryInner {
		public:
			static IMasterWallet *importWalletInternal(const std::string &language,
													   const boost::function<bool(MasterWallet *)> &walletImportFun) {
				MasterWallet *masterWallet = new MasterWallet(language);

				if (!walletImportFun(masterWallet) || !masterWallet->Initialized()) {
					delete masterWallet;
					return nullptr;
				}
				return masterWallet;
			}
		};

		WalletFactory::WalletFactory() noexcept {

		}

		WalletFactory::~WalletFactory() noexcept {

		}

		IMasterWallet *WalletFactory::CreateMasterWallet(const std::string &phrasePassword,
														 const std::string &payPassword,
														 const std::string &language) {

			if (phrasePassword.size() < MIN_PASSWORD_LENGTH || phrasePassword.size() > MAX_PASSWORD_LENGTH) {
				throw std::invalid_argument("Phrase password should between 8 and 128.");
			}
			if (payPassword.size() < MIN_PASSWORD_LENGTH || payPassword.size() > MAX_PASSWORD_LENGTH) {
				throw std::invalid_argument("Pay password should between 8 and 128.");
			}

			MasterWallet *masterWallet = new MasterWallet(phrasePassword, payPassword, language);
			return masterWallet;
		}

		void WalletFactory::DestroyWallet(IMasterWallet *masterWallet) {
			MasterWallet *masterWalletInner = static_cast<MasterWallet *>(masterWallet);
			delete masterWalletInner;
		}

		IMasterWallet *
		WalletFactory::ImportWalletWithKeystore(const std::string &keystorePath, const std::string &backupPassword,
												const std::string &payPassword) {
			return WalletFactoryInner::importWalletInternal("english", [&keystorePath, &backupPassword, &payPassword](
					MasterWallet *masterWallet) {
				return masterWallet->importFromKeyStore(keystorePath, backupPassword, payPassword);
			});
		}

		IMasterWallet *
		WalletFactory::ImportWalletWithMnemonic(const std::string &mnemonic, const std::string &phrasePassword,
												const std::string &payPassword, const std::string &language) {
			return WalletFactoryInner::importWalletInternal(language, [&mnemonic, &phrasePassword, &payPassword](
					MasterWallet *masterWallet) {
				return masterWallet->importFromMnemonic(mnemonic, phrasePassword, payPassword);
			});
		}

		void WalletFactory::ExportWalletWithKeystore(IMasterWallet *masterWallet, const std::string &backupPassword,
													 const std::string &keystorePath) {
			MasterWallet *wallet = static_cast<MasterWallet *>(masterWallet);
			if (!wallet->Initialized()) {
				Log::warn("Exporting failed, check if the wallet has been initialized.");
				return;
			}

			wallet->exportKeyStore(backupPassword, keystorePath);
		}

		std::string
		WalletFactory::ExportWalletWithMnemonic(IMasterWallet *masterWallet, const std::string &payPassword) {
			MasterWallet *wallet = static_cast<MasterWallet *>(masterWallet);
			if (!wallet->Initialized()) {
				Log::warn("Exporting failed, check if the wallet has been initialized.");
				return "";
			}

			std::string mnemonic;
			if (wallet->exportMnemonic(payPassword, mnemonic)) {
				return mnemonic;
			}
			return "";
		}
	}
}