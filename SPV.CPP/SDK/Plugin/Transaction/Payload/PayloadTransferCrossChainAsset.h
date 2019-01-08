// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef __ELASTOS_SDK_PAYLOADTRANSFERCROSSCHAINASSET_H
#define __ELASTOS_SDK_PAYLOADTRANSFERCROSSCHAINASSET_H

#include "IPayload.h"

#include <map>

namespace Elastos {
	namespace ElaWallet {

		class PayloadTransferCrossChainAsset :
				public IPayload {
		public:
			PayloadTransferCrossChainAsset();

			PayloadTransferCrossChainAsset(const PayloadTransferCrossChainAsset &payload);

			PayloadTransferCrossChainAsset(
				const std::vector<std::string> &crossChainAddress,
				const std::vector<uint64_t> &outputIndex,
				const std::vector<uint64_t> &crossChainAmount);

			~PayloadTransferCrossChainAsset();

			void setCrossChainData(const std::vector<std::string> &crossChainAddress,
			                   const std::vector<uint64_t> &outputIndex,
			                   const std::vector<uint64_t> &crossChainAmount);

			const std::vector<std::string> &getCrossChainAddress() const;

			const std::vector<uint64_t> &getOutputIndex() const;

			const std::vector<uint64_t> &getCrossChainAmout() const;


			virtual void Serialize(ByteStream &ostream, uint8_t version) const;

			virtual bool Deserialize(ByteStream &istream, uint8_t version);

			virtual nlohmann::json toJson(uint8_t version) const;

			virtual void fromJson(const nlohmann::json &jsonData, uint8_t version);

			virtual bool isValid() const;

			virtual IPayload &operator=(const IPayload &payload);

			PayloadTransferCrossChainAsset &operator=(const PayloadTransferCrossChainAsset &payload);

		private:
			std::vector<std::string> _crossChainAddress;
			std::vector<uint64_t> _outputIndex;
			std::vector<uint64_t> _crossChainAmount;
		};
	}
}

#endif //__ELASTOS_SDK_PAYLOADTRANSFERCROSSCHAINASSET_H
