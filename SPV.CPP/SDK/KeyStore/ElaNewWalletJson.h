// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef __ELASTOS_SDK_ELANEWWALLETJSON_H__
#define __ELASTOS_SDK_ELANEWWALLETJSON_H__

#include "ElaWebWalletJson.h"
#include "CoinInfo.h"

#include <SDK/Common/Mstream.h>

namespace Elastos {
	namespace ElaWallet {

		class ElaNewWalletJson :
				public ElaWebWalletJson {
		public:
			ElaNewWalletJson(const std::string &rootPath);

			~ElaNewWalletJson();

			void AddCoinInfo(const CoinInfo &info);

			void ClearCoinInfo();

			const std::vector<CoinInfo> &GetCoinInfoList() const;

			const std::string &GetType() const;

			void SetType(const std::string &type);

			const std::string &GetLanguage() const;

			void SetLanguage(const std::string &language);

			const std::vector<std::string> &GetCoSigners() const;

			void SetCoSigners(const std::vector<std::string> &coSigners);

			uint32_t GetRequiredSignCount() const;

			void SetRequiredSignCount(uint32_t count);

			const std::string &GetPrivateKey() const;

			void SetPrivateKey(const std::string &key);

			const std::string &GetPhrasePassword() const;

			void SetPhrasePassword(const std::string &phrasePassword);

			bool GetIsSingleAddress() const;

			void SetIsSingleAddress(bool value);

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
