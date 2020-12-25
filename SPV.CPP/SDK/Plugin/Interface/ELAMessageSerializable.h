// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef __ELASTOS_SDK_ELAMESSAGESERIALIZABLE_H__
#define __ELASTOS_SDK_ELAMESSAGESERIALIZABLE_H__

#include <Common/ByteStream.h>
#include <nlohmann/json.hpp>

#include <istream>
#include <ostream>

namespace Elastos {
	namespace ElaWallet {

		class ELAMessageSerializable {
		public:
			virtual ~ELAMessageSerializable() {}

			virtual size_t EstimateSize() const = 0;

			virtual void Serialize(ByteStream &stream, bool extend = false) const = 0;

			virtual bool Deserialize(const ByteStream &stream, bool extend = false) = 0;

		};

	}
}

#endif //__ELASTOS_SDK_ELAMESSAGESERIALIZABLE_H__
