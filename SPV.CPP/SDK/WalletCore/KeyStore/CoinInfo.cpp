// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "CoinInfo.h"

#include <SDK/Common/Utils.h>

namespace Elastos {
	namespace ElaWallet {

		CoinInfo::CoinInfo() :
				_chainID(""),
				_earliestPeerTime(0),
				_feePerKB(0) {

		}

		nlohmann::json CoinInfo::ToJson() const {
			nlohmann::json j;
			j["ChainID"] = _chainID;
			j["EarliestPeerTime"] = _earliestPeerTime;
			j["FeePerKB"] = _feePerKB;
			j["VisibleAssets"] = VisibleAssetsToJson();

			return j;
		}

		void CoinInfo::FromJson(const nlohmann::json &j) {
			_chainID = j["ChainID"].get<std::string>();
			if (_chainID == "IdChain")
				_chainID = "IDChain";
			_earliestPeerTime = j["EarliestPeerTime"].get<uint32_t>();
			_feePerKB = j["FeePerKB"].get<uint64_t>();
			if (j.find("VisibleAssets") != j.end())
				VisibleAssetsFromJson(j["VisibleAssets"]);
		}

		nlohmann::json CoinInfo::VisibleAssetsToJson() const {
			nlohmann::json j;
			std::for_each(_visibleAssets.begin(), _visibleAssets.end(), [&j](const uint256 &asset) {
				j.push_back(asset.GetHex());
			});
			return j;
		}

		void CoinInfo::VisibleAssetsFromJson(const nlohmann::json &j) {
			_visibleAssets.clear();
			std::vector<std::string> assets = j.get<std::vector<std::string>>();
			std::for_each(assets.begin(), assets.end(), [this](const std::string &assetStr) {
				_visibleAssets.emplace_back(assetStr);
			});
		}

		void CoinInfo::SetVisibleAsset(const uint256 &assetID) {
			bool found = false;

			for (size_t i = 0; i < _visibleAssets.size(); ++i) {
				if (_visibleAssets[i] == assetID)
					found = true;
			}

			if (!found)
				_visibleAssets.push_back(assetID);
		}

	}
}
