// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef __ELASTOS_SDK_IWALLETFACTORY_H__
#define __ELASTOS_SDK_IWALLETFACTORY_H__

#include "IMasterWallet.h"

namespace Elastos {
	namespace SDK {

		class IWalletFactory {
		public:
			virtual IMasterWallet *CreateMasterWallet(
					const std::string &name,
					const std::string &phrasePassword,
					const std::string &payPassword) = 0;

			virtual void DestroyWallet(IMasterWallet *masterWallet) = 0;

			virtual IMasterWallet *ImportWalletWithKeystore(
					const std::string &keystorePath,
					const std::string &backupPassword,
					const std::string &payPassword) = 0;

			virtual IMasterWallet *ImportWalletWithMnemonic(
					const std::string &mnemonic,
					const std::string &phrasePassword,
					const std::string &payPassword) = 0;

			virtual void ExportWalletWithKeystore(
					IMasterWallet *masterWallet,
					const std::string &backupPassword,
					const std::string &keystorePath) =0;

			virtual std::string ExportWalletWithMnemonic(
					IMasterWallet *masterWallet,
					const std::string &exportMnemonic) = 0;
		};

	}
}

#endif //__ELASTOS_SDK_IWALLETFACTORY_H__
