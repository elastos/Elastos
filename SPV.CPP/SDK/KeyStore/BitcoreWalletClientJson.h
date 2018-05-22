// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef __ELASTOS_SDK_BITCOREWALLETCLIENTJSON_H__
#define __ELASTOS_SDK_BITCOREWALLETCLIENTJSON_H__

#include <string>
#include <vector>
#include <nlohmann/json.hpp>

#include "Mstream.h"

namespace Elastos {
	namespace SDK {

		class BitcoreWalletClientJson {
		public:
			struct PubKeyItem {
				std::string xPubKey;
				std::string requestPubKey;

			private:
				JSON_SM_LS(PubKeyItem);
				JSON_SM_RS(PubKeyItem);
				TO_JSON(PubKeyItem);
				FROM_JSON(PubKeyItem);
			};

		public:
			BitcoreWalletClientJson();

			virtual ~BitcoreWalletClientJson();

		private:
			JSON_SM_LS(BitcoreWalletClientJson);
			JSON_SM_RS(BitcoreWalletClientJson);
			TO_JSON(BitcoreWalletClientJson);
			FROM_JSON(BitcoreWalletClientJson);

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
	}
}

#endif //__ELASTOS_SDK_BITCOREWALLETCLIENTJSON_H__
