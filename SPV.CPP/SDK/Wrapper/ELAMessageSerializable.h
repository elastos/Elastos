// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef __ELASTOS_SDK_ELAMESSAGESERIALIZABLE_H__
#define __ELASTOS_SDK_ELAMESSAGESERIALIZABLE_H__

#include <istream>
#include <ostream>

#include "ByteStream.h"
#include "nlohmann/json.hpp"

namespace Elastos {
	namespace SDK {

		class ELAMessageSerializable {
		public:
			virtual ~ELAMessageSerializable();

			virtual void Serialize(ByteStream &ostream) const = 0;
			virtual void Deserialize(ByteStream &istream) = 0;

			virtual nlohmann::json toJson();
			virtual void fromJson(const nlohmann::json);
		};

	}
}

#endif //__ELASTOS_SDK_ELAMESSAGESERIALIZABLE_H__
