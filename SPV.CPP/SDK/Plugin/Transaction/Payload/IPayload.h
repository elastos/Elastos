// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef __ELASTOS_SDK_IPAYLOAD_H__
#define __ELASTOS_SDK_IPAYLOAD_H__

#include <SDK/Plugin/Interface/ELAMessageSerializable.h>
#include <SDK/Common/CMemBlock.h>

#include <boost/shared_ptr.hpp>

namespace Elastos {
	namespace ElaWallet {

		class IPayload {
		public:
			virtual ~IPayload();

			virtual CMBlock getData(uint8_t versin) const;

			virtual void Serialize(ByteStream &ostream, uint8_t version) const = 0;
			virtual bool Deserialize(ByteStream &istream, uint8_t version) = 0;

			virtual nlohmann::json toJson() const = 0;
			virtual void fromJson(const nlohmann::json &) = 0;

			virtual bool isValid() const;

			virtual IPayload &operator=(const IPayload &payload) = 0;
		};

		typedef boost::shared_ptr<IPayload> PayloadPtr;

	}
}

#endif //__ELASTOS_SDK_IPAYLOAD_H__
