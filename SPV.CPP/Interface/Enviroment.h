// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef __ELASTOS_SDK_ENVIROMENT_H__
#define __ELASTOS_SDK_ENVIROMENT_H__

#include <string>

#include "IMasterWalletManager.h"

namespace Elastos {
	namespace SDK {

		class Enviroment {
		public:
			/**
			 * Initialize root path for all master wallets.
			 * @param rootPath specify directory for all config files, including mnemonic config files and peer connection config files. Root should not be empty, otherwise will throw invalid argument exception.
			 */
			static void InitializeRootPath(const std::string &rootPath);

			/**
			 * Get root path for all master wallets.
			 * @return root path.
			 */
			static std::string GetRootPath();

			static IMasterWalletManager *GetMasterWalletManager();

			static void SaveConfigs();

		private:
			static void CheckRootPath();

			static std::string _rootPath;
			static IMasterWalletManager *_manager;
		};

	}
}

#endif //__ELASTOS_SDK_ENVIROMENT_H__
