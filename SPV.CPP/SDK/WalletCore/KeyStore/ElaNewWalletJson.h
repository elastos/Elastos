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
			ElaNewWalletJson();

			~ElaNewWalletJson();

			void AddCoinInfo(const CoinInfo &info) { _coinInfoList.push_back(info); }

			void ClearCoinInfo() { _coinInfoList.clear(); }

			const std::vector<CoinInfo> &GetCoinInfoList() const { return _coinInfoList; }

			void SetCoinInfoList(const std::vector<CoinInfo> &list) { _coinInfoList = list; }

			const std::string &PassPhrase() const { return _passphrase; }

			void SetPassPhrase(const std::string &passphrase) { _passphrase = passphrase; }

			bool SingleAddress() const { return _singleAddress; }

			void SetSingleAddress(bool value) { _singleAddress = value; }

			bool Old() const { return _old; }

			ElaNewWalletJson &operator=(const nlohmann::json &j) {
				from_json(j, *this);
				return *this;
			}

			nlohmann::json ToJson(bool withPrivKey) const {
				nlohmann::json j;
				to_json(j, *this, withPrivKey);
				return j;
			}

			void FromJson(const nlohmann::json &j) {
				from_json(j, *this);
			}

			friend void to_json(nlohmann::json &j, const ElaNewWalletJson &p, bool withPrivKey);

			friend void from_json(const nlohmann::json &j, ElaNewWalletJson &p);

		private:
			std::vector<CoinInfo> _coinInfoList;
			std::string _passphrase __attribute((deprecated));
			bool _singleAddress;

			bool _old;
		};
	}
}

#endif //__ELASTOS_SDK_ELANEWWALLETJSON_H__
