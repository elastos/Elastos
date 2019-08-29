// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef __ELASTOS_SDK_RETURNDEPOSITCOIN_H__
#define __ELASTOS_SDK_RETURNDEPOSITCOIN_H__

#include "IPayload.h"

namespace Elastos {
	namespace ElaWallet {

		class ReturnDepositCoin : public IPayload {
		public:
			ReturnDepositCoin();

			ReturnDepositCoin(const ReturnDepositCoin &payload);

			~ReturnDepositCoin();

			virtual size_t EstimateSize(uint8_t version) const;

			virtual void Serialize(ByteStream &ostream, uint8_t version) const;

			virtual bool Deserialize(const ByteStream &istream, uint8_t version);

			virtual nlohmann::json ToJson(uint8_t version) const;

			virtual void FromJson(const nlohmann::json &j, uint8_t version);

			virtual IPayload &operator=(const IPayload &payload);

			ReturnDepositCoin &operator=(const ReturnDepositCoin &payload);
		};

	}
}

#endif // __ELASTOS_SDK_PAYLOADRETURNDEPOSITCOIN_H__
