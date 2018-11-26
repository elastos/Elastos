// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef __ELASTOS_SDK_PAYLOADCANCELPRODUCER_H__
#define __ELASTOS_SDK_PAYLOADCANCELPRODUCER_H__

#include <SDK/Plugin/Transaction/Payload/IPayload.h>

namespace Elastos {
	namespace ElaWallet {

		class PayloadCancelProducer : public IPayload {
		public:
			PayloadCancelProducer();

			~PayloadCancelProducer();

			const std::string &GetPublicKey() const;

			void SetPublicKey(const std::string &key);

			virtual void Serialize(ByteStream &ostream) const;

			virtual bool Deserialize(ByteStream &istream);

			virtual nlohmann::json toJson() const;

			virtual void fromJson(const nlohmann::json &);

		private:
			std::string _publicKey;
		};

	}
}

#endif // __ELASTOS_SDK_PAYLOADCANCELPRODUCER_H__
