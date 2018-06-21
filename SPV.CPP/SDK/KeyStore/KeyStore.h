// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef __ELASTOS_SDK_KEYSTORE_H__
#define __ELASTOS_SDK_KEYSTORE_H__

#include <boost/filesystem.hpp>

#include "ElaNewWalletJson.h"

namespace Elastos {
	namespace ElaWallet {

		class KeyStore {
		public:
			KeyStore();

			~KeyStore();

			bool open(const boost::filesystem::path &path, const std::string &password);

			bool open(const nlohmann::json &json, const std::string &password);

			bool save(const boost::filesystem::path &path, const std::string &password);

			bool save(nlohmann::json &json, const std::string &password);

			const ElaNewWalletJson &json() const;

			ElaNewWalletJson &json();

		private:

			ElaNewWalletJson _walletJson;
		};
	}
}

#endif //SPVSDK_KEYSTORE_H
