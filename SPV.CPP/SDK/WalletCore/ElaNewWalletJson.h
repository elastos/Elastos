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

			void AddCoinInfo(const CoinInfoPtr &info);

			void ClearCoinInfo();

			const std::vector<CoinInfoPtr> &GetCoinInfoList() const;

			void SetCoinInfoList(const std::vector<CoinInfoPtr> &list);

			bool SingleAddress() const;

			void SetSingleAddress(bool value);

			const std::string &OwnerPubKey() const;

			void SetOwnerPubKey(const std::string &pubkey);

			const std::string &xPubKeyHDPM() const;

			void SetxPubKeyHDPM(const std::string &xpub);

			const std::string &GetSeed() const;

			void SetSeed(const std::string &seed);

			const std::string &GetETHSCPrimaryPubKey() const;

			void SetETHSCPrimaryPubKey(const std::string &pubkey);

			virtual nlohmann::json ToJson(bool withPrivKey) const;

			virtual void FromJson(const nlohmann::json &j);

		private:
			void ToJsonCommon(nlohmann::json &j) const;

			void FromJsonCommon(const nlohmann::json &j);

		private:
			std::vector<CoinInfoPtr> _coinInfoList;
			std::string _ownerPubKey;
			std::string _xPubKeyHDPM;
			std::string _seed;
			std::string _ethscPrimaryPubKey;
			bool _singleAddress;
		};
	}
}

#endif //__ELASTOS_SDK_ELANEWWALLETJSON_H__
