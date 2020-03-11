// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef __ELASTOS_SDK_IPAYLOAD_H__
#define __ELASTOS_SDK_IPAYLOAD_H__

#include <Plugin/Interface/ELAMessageSerializable.h>

#include <boost/shared_ptr.hpp>

namespace Elastos {
	namespace ElaWallet {

		class IPayload {
		public:
			virtual ~IPayload();

			virtual bytes_t GetData(uint8_t versin) const;

			virtual size_t EstimateSize(uint8_t version) const = 0;

			virtual void Serialize(ByteStream &ostream, uint8_t version) const = 0;

			virtual bool Deserialize(const ByteStream &istream, uint8_t version) = 0;

			virtual nlohmann::json ToJson(uint8_t version) const = 0;

			virtual void FromJson(const nlohmann::json &j, uint8_t version) = 0;

			virtual bool IsValid(uint8_t version) const;

			virtual IPayload &operator=(const IPayload &payload) = 0;
		};

		typedef boost::shared_ptr<IPayload> PayloadPtr;

	}
}

#endif //__ELASTOS_SDK_IPAYLOAD_H__
