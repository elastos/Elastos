// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef __ELASTOS_SDK_IOUTPUTPAYLOAD_H__
#define __ELASTOS_SDK_IOUTPUTPAYLOAD_H__

#include <Common/ByteStream.h>
#include <nlohmann/json.hpp>
#include <boost/shared_ptr.hpp>
#include <Common/JsonSerializer.h>
#include <Plugin/Interface/ELAMessageSerializable.h>

namespace Elastos {
	namespace ElaWallet {

		class IOutputPayload : public ELAMessageSerializable, public JsonSerializer {
		public:
			virtual ~IOutputPayload();

			virtual bytes_t getData() const;

			virtual IOutputPayload &operator=(const IOutputPayload &payload) = 0;
		};

		typedef boost::shared_ptr<IOutputPayload> OutputPayloadPtr;

	}
}

#endif //__ELASTOS_SDK_IOUTPUTPAYLOAD_H__
