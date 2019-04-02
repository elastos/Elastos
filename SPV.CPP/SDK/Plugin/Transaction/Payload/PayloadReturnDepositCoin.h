// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef __ELASTOS_SDK_PAYLOADRETURNDEPOSITCOIN_H__
#define __ELASTOS_SDK_PAYLOADRETURNDEPOSITCOIN_H__

#include "IPayload.h"

namespace Elastos {
	namespace ElaWallet {

		class PayloadReturnDepositCoin : public IPayload {
		public:
			PayloadReturnDepositCoin();

			PayloadReturnDepositCoin(const PayloadReturnDepositCoin &payload);

			~PayloadReturnDepositCoin();

			virtual void Serialize(ByteStream &ostream, uint8_t version) const;

			virtual bool Deserialize(const ByteStream &istream, uint8_t version);

			virtual nlohmann::json ToJson(uint8_t version) const;

			virtual void FromJson(const nlohmann::json &j, uint8_t version);

			virtual IPayload &operator=(const IPayload &payload);

			PayloadReturnDepositCoin &operator=(const PayloadReturnDepositCoin &payload);
		};

	}
}

#endif // __ELASTOS_SDK_PAYLOADRETURNDEPOSITCOIN_H__
