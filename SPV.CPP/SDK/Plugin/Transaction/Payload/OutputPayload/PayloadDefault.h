// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef __ELASTOS_SDK_OUTPUT_PAYLOADDEFAULT_H
#define __ELASTOS_SDK_OUTPUT_PAYLOADDEFAULT_H

#include <Plugin/Transaction/Payload/OutputPayload/IOutputPayload.h>

namespace Elastos {
	namespace ElaWallet {
		class PayloadDefault : public IOutputPayload {
		public:
			PayloadDefault();

			PayloadDefault(const PayloadDefault &payload);

			~PayloadDefault();

			virtual size_t EstimateSize() const;

			virtual void Serialize(ByteStream &ostream) const;

			virtual bool Deserialize(const ByteStream &istream);

			virtual nlohmann::json ToJson() const;

			virtual void FromJson(const nlohmann::json &j);

			virtual IOutputPayload &operator=(const IOutputPayload &payload);

			virtual PayloadDefault &operator=(const PayloadDefault &payload);
		};
	}
}

#endif //__ELASTOS_SDK_OUTPUT_PayloadDefault_H
