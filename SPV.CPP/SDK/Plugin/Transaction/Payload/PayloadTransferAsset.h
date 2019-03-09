// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef __ELASTOS_SDK_PAYLOADTRANSFERASSET_H
#define __ELASTOS_SDK_PAYLOADTRANSFERASSET_H

#include "IPayload.h"

namespace Elastos {
	namespace ElaWallet {

		class PayloadTransferAsset :
				public IPayload {
		public:
			PayloadTransferAsset();

			PayloadTransferAsset(const PayloadTransferAsset &payload);

			~PayloadTransferAsset();

			virtual void Serialize(ByteStream &ostream, uint8_t version) const;

			virtual bool Deserialize(ByteStream &istream, uint8_t version);

			virtual nlohmann::json ToJson(uint8_t version) const;

			virtual void FromJson(const nlohmann::json &j, uint8_t version);

			virtual IPayload &operator=(const IPayload &payload);

			PayloadTransferAsset &operator=(const PayloadTransferAsset &payload);

		};
	}
}

#endif //__ELASTOS_SDK_PAYLOADTRANSFERASSET_H
