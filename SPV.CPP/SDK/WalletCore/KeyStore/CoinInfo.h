// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef __ELASTOS_SDK_ELACOINPATH_H__
#define __ELASTOS_SDK_ELACOINPATH_H__

#include <SDK/Implement/SubWalletType.h>
#include <SDK/Common/Mstream.h>
#include <SDK/Common/uint256.h>

#include <nlohmann/json.hpp>

#include <string>
#include <cstdint>

namespace Elastos {
	namespace ElaWallet {

		class CoinInfo {
		public:
			CoinInfo();

			const std::string &GetChainId() const { return _chainId; }

			void SetChainId(const std::string &id) { _chainId = id; }

			uint32_t GetEarliestPeerTime() const { return _earliestPeerTime; }

			void SetEaliestPeerTime(uint32_t time) { _earliestPeerTime = time; }

			uint32_t GetReconnectSeconds() const { return _reconnectSeconds; }

			void SetReconnectSeconds(uint32_t reconnectSeconds) { _reconnectSeconds = reconnectSeconds; }

			uint32_t GetIndex() const { return _index; }

			void SetIndex(uint32_t index) { _index = index; }

			uint64_t GetFeePerKb() const { return _feePerKb; }

			void SetFeePerKb(uint64_t fee) { _feePerKb = fee; }

			int GetForkId() const { return _forkId; }

			void SetForkId(int forkId) { _forkId = forkId; }

			SubWalletType GetWalletType() const { return _walletType; }

			void SetWalletType(SubWalletType type) { _walletType = type; }

			uint64_t GetMinFee() const { return _minFee; }

			void SetMinFee(uint64_t fee) { _minFee = fee; }

			const std::string &GetGenesisAddress() const { return _genesisAddress; }

			void SetGenesisAddress(const std::string &address) { _genesisAddress = address; }

			bool GetEnableP2P() const { return _enableP2P; }

			void SetEnableP2P(bool enable) { _enableP2P = enable; }

			const std::vector<uint256> &GetVisibleAssets() const { return _visibleAssets; }

			void SetVisibleAssets(const std::vector<uint256> &assets) { _visibleAssets = assets; }

			nlohmann::json VisibleAssetsToJson() const;

			void VisibleAssetsFromJson(const nlohmann::json &j);

			void SetVisibleAsset(const uint256 &assetID);

		private:
			TO_JSON(CoinInfo);

			FROM_JSON(CoinInfo);

		private:
			std::string _chainId;
			uint32_t _earliestPeerTime;
			uint32_t _reconnectSeconds;
			int _forkId;
			uint32_t _index;
			bool _enableP2P;
			uint64_t _minFee;
			uint64_t _feePerKb;
			SubWalletType _walletType;
			std::string _genesisAddress;
			std::vector<uint256> _visibleAssets;
		};

	}
}

#endif //__ELASTOS_SDK_ELACOINPATH_H__
