// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef __ELASTOS_SDK_ELACOINPATH_H__
#define __ELASTOS_SDK_ELACOINPATH_H__

#include <SDK/Common/uint256.h>

#include <nlohmann/json.hpp>

#include <string>
#include <cstdint>

namespace Elastos {
	namespace ElaWallet {

		class CoinInfo {
		public:
			CoinInfo();

			const std::string &GetChainID() const { return _chainID; }

			void SetChainID(const std::string &id) { _chainID = id; }

			time_t GetEarliestPeerTime() const { return _earliestPeerTime; }

			void SetEaliestPeerTime(time_t time) { _earliestPeerTime = time; }

			uint64_t GetFeePerKB() const { return _feePerKB; }

			void SetFeePerKB(uint64_t fee) { _feePerKB = fee; }

			const std::vector<uint256> &GetVisibleAssets() const { return _visibleAssets; }

			void SetVisibleAssets(const std::vector<uint256> &assets) { _visibleAssets = assets; }

			nlohmann::json VisibleAssetsToJson() const;

			void VisibleAssetsFromJson(const nlohmann::json &j);

			void SetVisibleAsset(const uint256 &assetID);

			nlohmann::json ToJson() const;

			void FromJson(const nlohmann::json &j);

		private:
			std::string _chainID;
			time_t _earliestPeerTime;
			uint64_t _feePerKB;
			std::vector<uint256> _visibleAssets;
		};

		typedef boost::shared_ptr<CoinInfo> CoinInfoPtr;

	}
}

#endif //__ELASTOS_SDK_ELACOINPATH_H__
