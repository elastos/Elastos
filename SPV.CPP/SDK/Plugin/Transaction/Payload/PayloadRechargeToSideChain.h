// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef __ELASTOS_SDK_PAYLOADISSUETOKEN_H
#define __ELASTOS_SDK_PAYLOADISSUETOKEN_H

#include <SDK/Common/uint256.h>
#include "IPayload.h"

namespace Elastos {
	namespace ElaWallet {

		class PayloadRechargeToSideChain :
				public IPayload {
		public:
			enum Version {
				V0,
				V1,
			};

		public:
			PayloadRechargeToSideChain();

			PayloadRechargeToSideChain(const bytes_t &merkeProff, const bytes_t &mainChainTransaction);

			PayloadRechargeToSideChain(const PayloadRechargeToSideChain &payload);

			~PayloadRechargeToSideChain();

			virtual size_t EstimateSize(uint8_t version) const;

			virtual void Serialize(ByteStream &ostream, uint8_t version) const;

			virtual bool Deserialize(const ByteStream &istream, uint8_t version);

			virtual nlohmann::json ToJson(uint8_t version) const;

			virtual void FromJson(const nlohmann::json &j, uint8_t version);

			virtual IPayload &operator=(const IPayload &payload);

			PayloadRechargeToSideChain &operator=(const PayloadRechargeToSideChain &payload);

		private:
			bytes_t _merkeProof;
			bytes_t _mainChainTransaction;
			uint256 _mainChainTxHash;
		};
	}
}

#endif //__ELASTOS_SDK_PAYLOADISSUETOKEN_H
