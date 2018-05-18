// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef __ELASTOS_SDK_BITCOREWALLETCLIENTJSON_H__
#define __ELASTOS_SDK_BITCOREWALLETCLIENTJSON_H__

#include <string>
#include <vector>
#include <nlohmann/json.hpp>

namespace Elastos {
	namespace SDK {

		class BitcoreWalletClientJson {
		public:
			struct PubKeyItem {
				std::string xPubKey;
				std::string requestPubKey;
			};

		public:
			BitcoreWalletClientJson();

			virtual ~BitcoreWalletClientJson();

		private:
			std::string _coin;
			std::string _network;
			std::string _xPrivKey;
			PubKeyItem _pubKeyItem;
			std::vector<PubKeyItem> _publicKeyRing;
			std::string _requestPrivKey;
			std::string _copayerId;
			std::string _walletId;
			std::string _walletName;
			int _m;
			int _n;
			std::string _walletPrivKey;
			std::string _personalEncryptingKey;
			std::string _sharedEncryptingKey;
			std::string _copayerName;
			std::string _entropySource;
			std::string _derivationStrategy;
			int _account;
			bool _compliantDerivation;
			std::string _addressType;
		};

		//support for json converting
		//read "Arbitrary types conversions" section in readme of
		//	https://github.com/nlohmann/json for more details
		void to_json(nlohmann::json &j, const BitcoreWalletClientJson &p);

		void from_json(const nlohmann::json &j, BitcoreWalletClientJson &p);

	}
}

#endif //__ELASTOS_SDK_BITCOREWALLETCLIENTJSON_H__
