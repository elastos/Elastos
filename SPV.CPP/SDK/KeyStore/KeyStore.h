// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef __ELASTOS_SDK_KEYSTORE_H__
#define __ELASTOS_SDK_KEYSTORE_H__

#include "ElaNewWalletJson.h"
#include <SDK/Account/IAccount.h>

#include <boost/filesystem.hpp>

namespace Elastos {
	namespace ElaWallet {

		class KeyStore {
		public:
			KeyStore(const std::string &rootPath);

			~KeyStore();

			bool Open(const boost::filesystem::path &path, const std::string &password);

			// to support old web keystore temporary
			bool Import(const nlohmann::json &json, const std::string &passwd, const std::string &phrasePasswd);

			bool Import(const nlohmann::json &json, const std::string &password);

			bool Save(const boost::filesystem::path &path, const std::string &password);

			bool Export(nlohmann::json &json, const std::string &password);

			bool IsOld();

			bool HasPhrasePassword() const;

			const ElaNewWalletJson &json() const;

			ElaNewWalletJson &json();

			IAccount *CreateAccountFromJson(const std::string &payPassword) const;

			void InitJsonFromAccount(IAccount *account, const std::string &payPassword);

		private:
			void InitAccountByType(IAccount *account, const std::string &payPassword);

			void InitStandardAccount(IAccount *account, const std::string &payPassword);

			void InitSimpleAccount(IAccount *account, const std::string &payPassword);

			void InitMultiSignAccount(IAccount *account, const std::string &payPassword);

		private:

			std::string _rootPath;
			ElaNewWalletJson _walletJson;
		};
	}
}

#endif //SPVSDK_KEYSTORE_H
