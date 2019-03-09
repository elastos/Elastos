// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef __ELASTOS_SDK_ELACOINPATH_H__
#define __ELASTOS_SDK_ELACOINPATH_H__

#include <SDK/Implement/SubWalletType.h>
#include <SDK/Common/Mstream.h>

#include <Core/BRInt.h>
#include <nlohmann/json.hpp>

#include <string>
#include <cstdint>

namespace Elastos {
	namespace ElaWallet {

		class CoinInfo {
		public:
			CoinInfo();

			const std::string &GetChainId() const;

			void SetChainId(const std::string &id);

			uint32_t GetEarliestPeerTime() const;

			void SetEaliestPeerTime(uint32_t time);

			uint32_t GetReconnectSeconds() const;

			void SetReconnectSeconds(uint32_t reconnectSeconds);

			uint32_t GetIndex() const;

			void SetIndex(uint32_t index);

			int GetUsedMaxAddressIndex() const;

			void SetUsedMaxAddressIndex(int index);

			bool GetSingleAddress() const;

			void SetSingleAddress(bool singleAddress);

			uint64_t GetFeePerKb() const;

			void SetFeePerKb(uint64_t fee);

			int GetForkId() const;

			void SetForkId(int forkId);

			SubWalletType GetWalletType() const;

			void SetWalletType(SubWalletType type);

			uint64_t GetMinFee() const;

			void SetMinFee(uint64_t fee);

			const std::string &GetGenesisAddress() const;

			void SetGenesisAddress(const std::string &address);

			bool GetEnableP2P() const;

			void SetEnableP2P(bool enable);

			const std::string &GetChainCode() const;

			void SetChainCode(const std::string &code);

			const std::string &GetPublicKey() const;

			void SetPublicKey(const std::string &pubKey);

			const std::vector<UInt256> &GetVisibleAssets() const;

			void SetVisibleAssets(const std::vector<UInt256> &assets);

			nlohmann::json VisibleAssetsToJson() const;

			void VisibleAssetsFromJson(const nlohmann::json &j);

		private:
			JSON_SM_LS(CoinInfo);
			JSON_SM_RS(CoinInfo);
			TO_JSON(CoinInfo);
			FROM_JSON(CoinInfo);

		private:
			std::string _chainId;
			uint32_t _earliestPeerTime;
			uint32_t _reconnectSeconds;
			int _forkId;
			uint32_t _index;
			int _usedMaxAddressIndex;
			bool _singleAddress;
			bool _enableP2P;
			uint64_t _minFee;
			uint64_t _feePerKb;
			SubWalletType _walletType;
			std::string _publicKey;
			std::string _chainCode;
			std::string _genesisAddress;
			std::vector<UInt256> _visibleAssets;
		};

	}
}

#endif //__ELASTOS_SDK_ELACOINPATH_H__
