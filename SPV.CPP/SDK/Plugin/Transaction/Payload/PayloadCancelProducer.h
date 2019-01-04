// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef __ELASTOS_SDK_PAYLOADCANCELPRODUCER_H__
#define __ELASTOS_SDK_PAYLOADCANCELPRODUCER_H__

#include "IPayload.h"

namespace Elastos {
	namespace ElaWallet {

		class PayloadCancelProducer : public IPayload {
		public:
			PayloadCancelProducer();

			PayloadCancelProducer(const PayloadCancelProducer &payload);

			~PayloadCancelProducer();

			const CMBlock &GetPublicKey() const;

			void SetPublicKey(const CMBlock &key);

			virtual void Serialize(ByteStream &ostream, uint8_t version) const;

			virtual bool Deserialize(ByteStream &istream, uint8_t version);

			virtual nlohmann::json toJson() const;

			virtual void fromJson(const nlohmann::json &);

			virtual IPayload &operator=(const IPayload &payload);

			PayloadCancelProducer &operator=(const PayloadCancelProducer &payload);

		private:
			CMBlock _publicKey;
		};

	}
}

#endif // __ELASTOS_SDK_PAYLOADCANCELPRODUCER_H__
