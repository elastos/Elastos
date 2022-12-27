// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef __ELASTOS_SDK_TRANSFERCROSSCHAINASSET_H
#define __ELASTOS_SDK_TRANSFERCROSSCHAINASSET_H

#include "IPayload.h"

#include <Common/BigInt.h>
#include <map>

namespace Elastos {
	namespace ElaWallet {

		class TransferCrossChainAsset;

#define TransferCrossChainVersion 0x00
#define TransferCrossChainVersionV1 0x01

		class TransferInfo {
		public:
			TransferInfo();

			TransferInfo(const std::string &address, uint16_t index, const BigInt &amount);

			const std::string &CrossChainAddress() const;

			uint16_t OutputIndex() const;

			const BigInt &CrossChainAmount() const;

			virtual nlohmann::json ToJson(uint8_t version) const;

			virtual void FromJson(const nlohmann::json &j, uint8_t version);

			virtual bool operator==(const TransferInfo &payload) const;

		private:
			friend TransferCrossChainAsset;

			std::string _crossChainAddress;
			uint16_t _outputIndex;
			BigInt _crossChainAmount;
		};

		class TransferCrossChainAsset :
				public IPayload {
		public:
			TransferCrossChainAsset();

			TransferCrossChainAsset(const TransferCrossChainAsset &payload);

			TransferCrossChainAsset(const std::vector<TransferInfo> &info);

			~TransferCrossChainAsset();

			const std::vector<TransferInfo> &Info() const;

			virtual size_t EstimateSize(uint8_t version) const;

			virtual void Serialize(ByteStream &ostream, uint8_t version) const;

			virtual bool Deserialize(const ByteStream &istream, uint8_t version);

			virtual nlohmann::json ToJson(uint8_t version) const;

			virtual void FromJson(const nlohmann::json &j, uint8_t version);

			virtual bool IsValid(uint8_t version) const;

			virtual IPayload &operator=(const IPayload &payload);

			TransferCrossChainAsset &operator=(const TransferCrossChainAsset &payload);

			virtual bool Equal(const IPayload &payload, uint8_t version) const;
		private:
			std::vector<TransferInfo> _info;
		};
	}
}

#endif //__ELASTOS_SDK_PAYLOADTRANSFERCROSSCHAINASSET_H
