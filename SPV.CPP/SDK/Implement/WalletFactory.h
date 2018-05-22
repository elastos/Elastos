// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef __ELASTOS_SDK_WALLETFACTORY_H__
#define __ELASTOS_SDK_WALLETFACTORY_H__

#include <map>
#include <boost/function.hpp>

#include "Interface/IWalletFactory.h"

namespace Elastos {
	namespace SDK {

		class MasterWallet;

		class WalletFactory : public IWalletFactory {
		public:
			WalletFactory();

			virtual ~WalletFactory();

			virtual IMasterWallet *CreateMasterWallet(
					const std::string &name,
					const std::string &phrasePassword,
					const std::string &payPassword);

			virtual void DestroyWallet(IMasterWallet *masterWallet);

			virtual IMasterWallet *ImportWalletWithKeystore(
					const std::string &keystorePath,
					const std::string &backupPassword,
					const std::string &payPassword);

			virtual IMasterWallet *ImportWalletWithMnemonic(
					const std::string &mnemonic,
					const std::string &phrasePassword,
					const std::string &payPassword);

			virtual void ExportWalletWithKeystore(
					IMasterWallet *masterWallet,
					const std::string &backupPassword,
					const std::string &keystorePath);

			virtual std::string ExportWalletWithMnemonic(
					IMasterWallet *masterWallet,
					const std::string &phrasePassword);

		protected:
			IMasterWallet *importWalletInternal(const boost::function<bool(MasterWallet *)> &walletImportFun);

		protected:
			typedef std::map<std::string, IMasterWallet *> MasterWalletMap;
			MasterWalletMap _masterWallets;
		};

	}
}

#endif //__ELASTOS_SDK_WALLETFACTORY_H__
