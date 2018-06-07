// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <boost/function.hpp>

#include "WalletFactory.h"
#include "MasterWallet.h"
#include "Log.h"
#include "ParamChecker.h"

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
														 const std::string &language,
														 const std::string &rootPath) {
			MasterWallet *masterWallet = new MasterWallet(phrasePassword, payPassword, language, rootPath);
			return masterWallet;
		}

		void WalletFactory::DestroyWallet(IMasterWallet *masterWallet) {
			MasterWallet *masterWalletInner = static_cast<MasterWallet *>(masterWallet);
			delete masterWalletInner;
		}

		IMasterWallet *
		WalletFactory::ImportWalletWithKeystore(const std::string &keystorePath, const std::string &backupPassword,
												const std::string &payPassword, const std::string &phrasePassword,
												const std::string &rootPath) {
			ParamChecker::checkPassword(backupPassword);
			ParamChecker::checkPassword(payPassword);

			return WalletFactoryInner::importWalletInternal("english",
															[&keystorePath, &backupPassword, &payPassword, &phrasePassword, &rootPath](
																	MasterWallet *masterWallet) {
																return masterWallet->importFromKeyStore(keystorePath,
																										backupPassword,
																										payPassword,
																										phrasePassword,
																										rootPath);
															});
		}

		IMasterWallet *
		WalletFactory::ImportWalletWithMnemonic(const std::string &mnemonic, const std::string &phrasePassword,
												const std::string &payPassword, const std::string &language,
												const std::string &rootPath) {
			ParamChecker::checkPassword(phrasePassword);
			ParamChecker::checkPassword(payPassword);

			return WalletFactoryInner::importWalletInternal(language,
															[&mnemonic, &phrasePassword, &payPassword, &rootPath](
																	MasterWallet *masterWallet) {
																return masterWallet->importFromMnemonic(mnemonic,
																										phrasePassword,
																										payPassword,
																										rootPath);
															});
		}

		void WalletFactory::ExportWalletWithKeystore(IMasterWallet *masterWallet, const std::string &backupPassword,
													 const std::string &keystorePath) {

			ParamChecker::checkPassword(backupPassword);

			MasterWallet *wallet = static_cast<MasterWallet *>(masterWallet);
			if (!wallet->Initialized()) {
				Log::warn("Exporting failed, check if the wallet has been initialized.");
				return;
			}

			wallet->exportKeyStore(backupPassword, keystorePath);
		}

		std::string
		WalletFactory::ExportWalletWithMnemonic(IMasterWallet *masterWallet, const std::string &payPassword) {

			ParamChecker::checkPassword(payPassword);

			MasterWallet *wallet = static_cast<MasterWallet *>(masterWallet);
			if (!wallet->Initialized()) {
				throw std::logic_error("Exporting failed, check if the wallet has been initialized.");
			}

			std::string mnemonic;
			if (wallet->exportMnemonic(payPassword, mnemonic)) {
				return mnemonic;
			}
			return "";
		}
	}
}