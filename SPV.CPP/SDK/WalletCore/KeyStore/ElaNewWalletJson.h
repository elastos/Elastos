// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef __ELASTOS_SDK_ELANEWWALLETJSON_H__
#define __ELASTOS_SDK_ELANEWWALLETJSON_H__

#include "ElaWebWalletJson.h"

#include <boost/shared_ptr.hpp>

namespace Elastos {
	namespace ElaWallet {

		class CoinInfo;

		typedef boost::shared_ptr<CoinInfo> CoinInfoPtr;

		class ElaNewWalletJson :
				public ElaWebWalletJson {
		public:
			ElaNewWalletJson();

			~ElaNewWalletJson();

			void AddCoinInfo(const CoinInfoPtr &info) { _coinInfoList.push_back(info); }

			void ClearCoinInfo() { _coinInfoList.clear(); }

			const std::vector<CoinInfoPtr> &GetCoinInfoList() const { return _coinInfoList; }

			void SetCoinInfoList(const std::vector<CoinInfoPtr> &list) { _coinInfoList = list; }

			const std::string &PassPhrase() const { return _passphrase; }

			void SetPassPhrase(const std::string &passphrase) { _passphrase = passphrase; }

			bool SingleAddress() const { return _singleAddress; }

			void SetSingleAddress(bool value) { _singleAddress = value; }

			const std::string &OwnerPubKey() const { return _ownerPubKey; }

			void SetOwnerPubKey(const std::string &pubkey) { _ownerPubKey = pubkey; }

			virtual nlohmann::json ToJson(bool withPrivKey) const;

			virtual void FromJson(const nlohmann::json &j);

			friend void to_json(nlohmann::json &j, const ElaNewWalletJson &p);

			friend void from_json(const nlohmann::json &j, ElaNewWalletJson &p);

		private:
			void ToJsonCommon(nlohmann::json &j) const;

			void FromJsonCommon(const nlohmann::json &j);

		private:
			std::vector<CoinInfoPtr> _coinInfoList;
//			std::string _passphrase __attribute((deprecated));
			std::string _passphrase;
			std::string _ownerPubKey;
			bool _singleAddress;
		};
	}
}

#endif //__ELASTOS_SDK_ELANEWWALLETJSON_H__
