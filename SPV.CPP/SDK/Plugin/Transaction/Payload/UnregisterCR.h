// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef __ELASTOS_SDK_UNREGISTERCR_H__
#define __ELASTOS_SDK_UNREGISTERCR_H__

#include <Plugin/Transaction/Payload/IPayload.h>

namespace Elastos {
	namespace ElaWallet {
		class UnregisterCR : public IPayload {
		public:
			UnregisterCR();

			~UnregisterCR();

			void SetCID(const uint168 &cid);

			const uint168 &GetCID() const;

			void SetSignature(const bytes_t &signature);

			const bytes_t &GetSignature() const;

			virtual size_t EstimateSize(uint8_t version) const;

			virtual void Serialize(ByteStream &ostream, uint8_t version) const;

			virtual bool Deserialize(const ByteStream &istream, uint8_t version);

			void SerializeUnsigned(ByteStream &ostream, uint8_t version) const;

			bool DeserializeUnsigned(const ByteStream &istream, uint8_t version);

			virtual nlohmann::json ToJson(uint8_t version) const;

			virtual void FromJson(const nlohmann::json &j, uint8_t version);

			virtual IPayload &operator=(const IPayload &payload);

			UnregisterCR &operator=(const UnregisterCR &payload);
		private:
			uint168 _cid;
			bytes_t _signature;
		};
	}
}

#endif //__ELASTOS_SDK_UNREGISTERCR_H__
