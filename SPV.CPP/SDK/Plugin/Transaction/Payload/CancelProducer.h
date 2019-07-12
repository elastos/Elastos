// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef __ELASTOS_SDK_CANCELPRODUCER_H__
#define __ELASTOS_SDK_CANCELPRODUCER_H__

#include "IPayload.h"

namespace Elastos {
	namespace ElaWallet {

		class CancelProducer : public IPayload {
		public:
			CancelProducer();

			CancelProducer(const CancelProducer &payload);

			~CancelProducer();

			const bytes_t &GetPublicKey() const;

			void SetPublicKey(const bytes_t &key);

			void SetSignature(const bytes_t &signature);

			void SerializeUnsigned(ByteStream &ostream, uint8_t version) const;

			bool DeserializeUnsigned(const ByteStream &istream, uint8_t version);

			virtual size_t EstimateSize(uint8_t version) const;

			virtual void Serialize(ByteStream &ostream, uint8_t version) const;

			virtual bool Deserialize(const ByteStream &istream, uint8_t version);

			virtual nlohmann::json ToJson(uint8_t version) const;

			virtual void FromJson(const nlohmann::json &j, uint8_t version);

			virtual IPayload &operator=(const IPayload &payload);

			CancelProducer &operator=(const CancelProducer &payload);

		private:
			bytes_t _publicKey;
			bytes_t _signature;
		};

	}
}

#endif // __ELASTOS_SDK_PAYLOADCANCELPRODUCER_H__
