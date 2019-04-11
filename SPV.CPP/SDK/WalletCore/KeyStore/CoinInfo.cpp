// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "CoinInfo.h"

#include <SDK/Common/Utils.h>
#include <SDK/Plugin/Transaction/Asset.h>

namespace Elastos {
	namespace ElaWallet {

		CoinInfo::CoinInfo() :
				_chainId(""),
				_earliestPeerTime(0),
				_forkId(0),
				_index(0),
				_enableP2P(true),
				_feePerKb(0),
				_minFee(0),
				_walletType(Normal) {

		}

		void to_json(nlohmann::json &j, const CoinInfo &p) {
			j["ChainID"] = p._chainId;
			j["EarliestPeerTime"] = p._earliestPeerTime;
			j["Index"] = p._index;
			j["WalletType"] = int(p.GetWalletType());
			j["MinFee"] = p._minFee;
			j["FeePerKB"] = p._feePerKb;
			j["EnableP2P"] = p._enableP2P;
			j["ReconnectSeconds"] = p._reconnectSeconds;
			j["VisibleAssets"] = p.VisibleAssetsToJson();
		}

		void from_json(const nlohmann::json &j, CoinInfo &p) {
			p._chainId = j["ChainID"].get<std::string>();
			p._earliestPeerTime = j["EarliestPeerTime"].get<uint32_t>();
			p._index = j["Index"].get<uint32_t>();
			p._walletType = (SubWalletType) j["WalletType"].get<int>();
			p._minFee = j["MinFee"].get<uint64_t>();
			p._feePerKb = j["FeePerKB"].get<uint64_t>();
			p._reconnectSeconds = j["ReconnectSeconds"].get<uint32_t>();
			if (j.find("EnableP2P") != j.end())
				p._enableP2P = j["EnableP2P"].get<bool>();
			if (j.find("VisibleAssets") != j.end()) {
				p.VisibleAssetsFromJson(j["VisibleAssets"]);
			} else
				p._visibleAssets = {Asset::GetELAAssetID()};
		}

		nlohmann::json CoinInfo::VisibleAssetsToJson() const {
			std::vector<std::string> assets;
			std::for_each(_visibleAssets.begin(), _visibleAssets.end(), [&assets](const uint256 &asset) {
				assets.push_back(asset.GetHex());
			});
			nlohmann::json j;
			std::for_each(assets.begin(), assets.end(), [&j](const std::string &asset) {
				j.push_back(asset);
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

	}
}
