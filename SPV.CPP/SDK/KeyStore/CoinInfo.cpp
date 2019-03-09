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
				_usedMaxAddressIndex(0),
				_singleAddress(false),
				_enableP2P(true),
				_feePerKb(0),
				_minFee(0),
				_walletType(Normal) {

		}

		const std::string &CoinInfo::GetChainId() const {
			return _chainId;
		}

		void CoinInfo::SetChainId(const std::string &id) {
			_chainId = id;
		}

		uint32_t CoinInfo::GetEarliestPeerTime() const {
			return _earliestPeerTime;
		}

		void CoinInfo::SetEaliestPeerTime(uint32_t time) {
			_earliestPeerTime = time;
		}

		uint32_t CoinInfo::GetIndex() const {
			return 0;//_index;
		}

		void CoinInfo::SetIndex(uint32_t index) {
			_index = 0;//index;
		}

		int CoinInfo::GetUsedMaxAddressIndex() const {
			return _usedMaxAddressIndex;
		}

		void CoinInfo::SetUsedMaxAddressIndex(int index) {
			_usedMaxAddressIndex = index;
		}

		bool CoinInfo::GetSingleAddress() const {
			return _singleAddress;
		}

		void CoinInfo::SetSingleAddress(bool singleAddress) {
			_singleAddress = singleAddress;
		}

		uint64_t CoinInfo::GetFeePerKb() const {
			return _feePerKb;
		}

		void CoinInfo::SetFeePerKb(uint64_t fee) {
			_feePerKb = fee;
		}

		nlohmann::json &operator<<(nlohmann::json &j, const CoinInfo &p) {
			to_json(j, p);

			return j;
		}

		const nlohmann::json &operator>>(const nlohmann::json &j, CoinInfo &p) {
			from_json(j, p);

			return j;
		}

		void to_json(nlohmann::json &j, const CoinInfo &p) {
			j["ChainID"] = p._chainId;
			j["EarliestPeerTime"] = p._earliestPeerTime;
			j["Index"] = 0;//p._index;
			j["UsedMaxAddressIndex"] = p._usedMaxAddressIndex;
			j["SingleAddress"] = p._singleAddress;
			j["WalletType"] = int(p.GetWalletType());
			j["MinFee"] = p._minFee;
			j["FeePerKB"] = p._feePerKb;
			j["EnableP2P"] = p._enableP2P;
			j["ReconnectSeconds"] = p._reconnectSeconds;
			j["ChainCode"] = p._chainCode;
			j["PublicKey"] = p._publicKey;
			j["VisibleAssets"] = p.VisibleAssetsToJson();
		}

		void from_json(const nlohmann::json &j, CoinInfo &p) {
			p._chainId = j["ChainID"].get<std::string>();
			p._earliestPeerTime = j["EarliestPeerTime"].get<uint32_t>();
			p._index = 0;//j["Index"].get<int>();
			p._usedMaxAddressIndex = j["UsedMaxAddressIndex"].get<int>();
			p._singleAddress = j["SingleAddress"].get<bool>();
			p._walletType = (SubWalletType) j["WalletType"].get<int>();
			p._minFee = j["MinFee"].get<uint64_t>();
			p._feePerKb = j["FeePerKB"].get<uint64_t>();
			p._reconnectSeconds = j["ReconnectSeconds"].get<uint32_t>();
			p._chainCode = j["ChainCode"].get<std::string>();
			p._publicKey = j["PublicKey"].get<std::string>();
			if (j.find("EnableP2P") != j.end())
				p._enableP2P = j["EnableP2P"].get<bool>();
			if (j.find("VisibleAssets") != j.end()) {
				p.VisibleAssetsFromJson(j["VisibleAssets"]);
			} else
				p._visibleAssets = {Asset::GetELAAssetID()};
		}

		int CoinInfo::GetForkId() const {
			return _forkId;
		}

		void CoinInfo::SetForkId(int forkId) {
			_forkId = forkId;
		}

		SubWalletType CoinInfo::GetWalletType() const {
			return _walletType;
		}

		void CoinInfo::SetWalletType(SubWalletType type) {
			_walletType = type;
		}

		uint64_t CoinInfo::GetMinFee() const {
			return _minFee;
		}

		void CoinInfo::SetMinFee(uint64_t fee) {
			_minFee = fee;
		}

		const std::string &CoinInfo::GetGenesisAddress() const {
			return _genesisAddress;
		}

		void CoinInfo::SetGenesisAddress(const std::string &address) {
			_genesisAddress = address;
		}

		bool CoinInfo::GetEnableP2P() const {
			return _enableP2P;
		}

		void CoinInfo::SetEnableP2P(bool enable) {
			_enableP2P = enable;
		}

		uint32_t CoinInfo::GetReconnectSeconds() const {
			return _reconnectSeconds;
		}

		void CoinInfo::SetReconnectSeconds(uint32_t reconnectSeconds) {
			_reconnectSeconds = reconnectSeconds;
		}

		const std::string &CoinInfo::GetChainCode() const {
			return _chainCode;
		}

		void CoinInfo::SetChainCode(const std::string &code) {
			_chainCode = code;
		}

		const std::string &CoinInfo::GetPublicKey() const {
			return _publicKey;
		}

		void CoinInfo::SetPublicKey(const std::string &pubKey) {
			_publicKey = pubKey;
		}

		const std::vector<UInt256> &CoinInfo::GetVisibleAssets() const {
			return _visibleAssets;
		}

		void CoinInfo::SetVisibleAssets(const std::vector<UInt256> &assets) {
			_visibleAssets = assets;
		}

		nlohmann::json CoinInfo::VisibleAssetsToJson() const {
			std::vector<std::string> assets;
			std::for_each(_visibleAssets.begin(), _visibleAssets.end(), [&assets](const UInt256 &asset) {
				assets.push_back(Utils::UInt256ToString(asset, true));
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
				_visibleAssets.push_back(Utils::UInt256FromString(assetStr, true));
			});
		}

	}
}