// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef __ELASTOS_SDK_ELANEWWALLETJSON_H__
#define __ELASTOS_SDK_ELANEWWALLETJSON_H__

#include "ElaWebWalletJson.h"
#include "CoinInfo.h"
#include "Mstream.h"
#include "CoinInfo.h"

namespace Elastos {
	namespace ElaWallet {

		class ElaNewWalletJson :
				public ElaWebWalletJson {
		public:
			ElaNewWalletJson(const std::string &rootPath);

			~ElaNewWalletJson();

			void addCoinInfo(const CoinInfo &info);

			void clearCoinInfo();

			const std::vector<CoinInfo> &getCoinInfoList() const;

			const std::string &getType() const;

			void setType(const std::string &type);

			const std::string &getLanguage() const;

			void setLanguage(const std::string &language);

			const std::vector<std::string> &getCoSigners() const;

			void setCoSigners(const std::vector<std::string> &coSigners);

			uint32_t getRequiredSignCount() const;

			void setRequiredSignCount(uint32_t count);

			const std::string &getPrivateKey() const;

			void setPrivateKey(const std::string &key);

			const std::string &getPhrasePassword() const;

			void setPhrasePassword(const std::string &phrasePassword);

			bool getIsSingleAddress() const;

			void setIsSingleAddress(bool value);

		private:
			JSON_SM_LS(ElaNewWalletJson);
			JSON_SM_RS(ElaNewWalletJson);
			TO_JSON(ElaNewWalletJson);
			FROM_JSON(ElaNewWalletJson);

		private:
			std::string _rootPath;
			std::vector<CoinInfo> _coinInfoList;
			std::string _type;
			std::string _language;
			std::vector<std::string> _coSigners;
			uint32_t _requiredSignCount;
			std::string _privateKey;
			std::string _phrasePassword;
			bool _isSingleAddress;
		};
	}
}

#endif //__ELASTOS_SDK_ELANEWWALLETJSON_H__
