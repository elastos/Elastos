// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef __ELASTOS_SDK_IPAYLOAD_H__
#define __ELASTOS_SDK_IPAYLOAD_H__

#include <boost/shared_ptr.hpp>

#include "ELAMessageSerializable.h"
#include "c_util.h"

namespace Elastos {
	namespace SDK {

		class IPayload :
				public ELAMessageSerializable {
		public:
			virtual ~IPayload();

			virtual CMBlock getData() const = 0;
		};

		typedef boost::shared_ptr<IPayload> PayloadPtr;

	}
}

#endif //__ELASTOS_SDK_IPAYLOAD_H__
