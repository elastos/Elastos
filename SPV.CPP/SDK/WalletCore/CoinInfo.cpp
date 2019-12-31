// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "CoinInfo.h"

#include <Common/Utils.h>

namespace Elastos {
	namespace ElaWallet {

		CoinInfo::CoinInfo() :
				_chainID(""),
				_earliestPeerTime(0) {

		}

		const std::string &CoinInfo::GetChainID() const {
			return _chainID;
		}

		void CoinInfo::SetChainID(const std::string &id) {
			_chainID = id;
		}

		time_t CoinInfo::GetEarliestPeerTime() const {
			return _earliestPeerTime;
		}

		void CoinInfo::SetEaliestPeerTime(time_t time) {
			_earliestPeerTime = time;
		}

		const std::vector<uint256> &CoinInfo::GetVisibleAssets() const {
			return _visibleAssets;
		}

		void CoinInfo::SetVisibleAssets(const std::vector<uint256> &assets) {
			_visibleAssets = assets;
		}

		nlohmann::json CoinInfo::ToJson() const {
			nlohmann::json j;
			j["ChainID"] = _chainID;
			j["EarliestPeerTime"] = _earliestPeerTime;
			j["VisibleAssets"] = VisibleAssetsToJson();

			return j;
		}

		void CoinInfo::FromJson(const nlohmann::json &j) {
			_chainID = j["ChainID"].get<std::string>();
			if (_chainID == "IdChain")
				_chainID = "IDChain";
			_earliestPeerTime = j["EarliestPeerTime"].get<uint32_t>();

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
			uint256 asset;
			for (nlohmann::json::const_iterator it = j.begin(); it != j.end(); ++it) {
				asset.SetHex((*it).get<std::string>());
				_visibleAssets.push_back(asset);
			}
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
