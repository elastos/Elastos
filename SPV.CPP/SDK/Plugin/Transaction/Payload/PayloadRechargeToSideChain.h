// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef __ELASTOS_SDK_PAYLOADISSUETOKEN_H
#define __ELASTOS_SDK_PAYLOADISSUETOKEN_H

#include "IPayload.h"

namespace Elastos {
	namespace ElaWallet {

		class PayloadRechargeToSideChain :
				public IPayload {
		public:
			PayloadRechargeToSideChain();

			PayloadRechargeToSideChain(const CMBlock &merkeProff, const CMBlock &mainChainTransaction);

			PayloadRechargeToSideChain(const PayloadRechargeToSideChain &payload);

			~PayloadRechargeToSideChain();

			virtual void Serialize(ByteStream &ostream, uint8_t version) const;

			virtual bool Deserialize(ByteStream &istream, uint8_t version);

			virtual nlohmann::json toJson() const;

			virtual void fromJson(const nlohmann::json &jsonData);

			virtual IPayload &operator=(const IPayload &payload);

			PayloadRechargeToSideChain &operator=(const PayloadRechargeToSideChain &payload);

		private:
			CMBlock _merkeProof;
			CMBlock _mainChainTransaction;
		};
	}
}

#endif //__ELASTOS_SDK_PAYLOADISSUETOKEN_H
