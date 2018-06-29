// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef __ELASTOS_SDK_PAYLOADTRANSFERCROSSCHAINASSET_H
#define __ELASTOS_SDK_PAYLOADTRANSFERCROSSCHAINASSET_H

#include <map>

#include "IPayload.h"

namespace Elastos {
	namespace ElaWallet {

		class PayloadTransferCrossChainAsset :
				public IPayload {
		public:
			PayloadTransferCrossChainAsset();

			PayloadTransferCrossChainAsset(const std::vector<std::string> crossChainAddress,
			                               const std::vector<uint64_t> outputIndex,
			                               const std::vector<uint64_t> crossChainAmount);

			~PayloadTransferCrossChainAsset();

			void setCrossChainData(const std::vector<std::string> crossChainAddress,
			                   const std::vector<uint64_t> outputIndex,
			                   const std::vector<uint64_t> crossChainAmount);

			virtual CMBlock getData() const;

			const std::vector<std::string> &getCrossChainAddress() const;

			const std::vector<uint64_t> &getOutputIndex() const;

			const std::vector<uint64_t> &getCrossChainAmout() const;


			virtual void Serialize(ByteStream &ostream) const;

			virtual bool Deserialize(ByteStream &istream);

			virtual nlohmann::json toJson() const;

			virtual void fromJson(const nlohmann::json &jsonData);

			virtual bool isValid() const;

		private:
			std::vector<std::string> _crossChainAddress;
			std::vector<uint64_t> _outputIndex;
			std::vector<uint64_t> _crossChainAmount;
		};
	}
}

#endif //__ELASTOS_SDK_PAYLOADTRANSFERCROSSCHAINASSET_H
