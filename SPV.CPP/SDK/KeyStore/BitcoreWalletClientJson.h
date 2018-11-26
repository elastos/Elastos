// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef __ELASTOS_SDK_BITCOREWALLETCLIENTJSON_H__
#define __ELASTOS_SDK_BITCOREWALLETCLIENTJSON_H__

#include <SDK/Common/Mstream.h>

#include <nlohmann/json.hpp>
#include <string>
#include <vector>

namespace Elastos {
	namespace ElaWallet {

		class BitcoreWalletClientJson {
		public:
			struct PubKeyItem {
				std::string xPubKey;
				std::string requestPubKey;
				//std::string copayerName;

			private:
				JSON_SM_LS(PubKeyItem);
				JSON_SM_RS(PubKeyItem);
				TO_JSON(PubKeyItem);
				FROM_JSON(PubKeyItem);
			};

		public:
			BitcoreWalletClientJson();

			virtual ~BitcoreWalletClientJson();

		public:
			const std::string &getEncryptedEntropySource() const;
			void setEncryptedEntropySource(const std::string &entropy);
			const std::string &getMnemonic() const;
			void setMnemonic(const std::string mnemonic);

		private:
			JSON_SM_LS(BitcoreWalletClientJson);
			JSON_SM_RS(BitcoreWalletClientJson);
			TO_JSON(BitcoreWalletClientJson);
			FROM_JSON(BitcoreWalletClientJson);

		private:
			std::string _coin;
			std::string _network;
			std::string _xPrivKey;
			std::string _xPubKey;
			std::string _requestPrivKey;
			std::string _requestPubKey;
			std::string _copayerId;
			std::vector<PubKeyItem> _publicKeyRing;
			std::string _walletId;
			std::string _walletName;
			int _m;
			int _n;
			std::string _walletPrivKey;
			std::string _personalEncryptingKey;
			std::string _sharedEncryptingKey;
			std::string _copayerName;
			std::string _mnemonic;
			std::string _entropySource;
			bool _mnemonicHasPassphrase;
			std::string _derivationStrategy;
			int _account;
			bool _compliantDerivation;
			std::string _addressType;
		};
	}
}

#endif //__ELASTOS_SDK_BITCOREWALLETCLIENTJSON_H__
