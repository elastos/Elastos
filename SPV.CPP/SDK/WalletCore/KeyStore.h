// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef __ELASTOS_SDK_KEYSTORE_H__
#define __ELASTOS_SDK_KEYSTORE_H__

#include "ElaNewWalletJson.h"

#include <boost/filesystem.hpp>

namespace Elastos {
	namespace ElaWallet {

		class KeyStore {
		public:
			KeyStore();

			KeyStore(const ElaNewWalletJson &walletjson);

			~KeyStore();

			bool Open(const boost::filesystem::path &path, const std::string &password);

			bool ImportReadonly(const nlohmann::json &json);

			bool Import(const nlohmann::json &json, const std::string &password);

			bool Save(const boost::filesystem::path &path, const std::string &password, bool withPrivKey);

			nlohmann::json ExportReadonly() const;

			nlohmann::json Export(const std::string &password, bool withPrivKey) const;

			const ElaNewWalletJson &WalletJson() const { return _walletJson; }

			ElaNewWalletJson& WalletJson() { return _walletJson; }

		private:

			ElaNewWalletJson _walletJson;
		};
	}
}

#endif //SPVSDK_KEYSTORE_H
