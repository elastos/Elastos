// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef __ELASTOS_SDK_IATTRIBUTE_H__
#define __ELASTOS_SDK_IATTRIBUTE_H__

#include <boost/shared_ptr.hpp>

#include "ELAMessageSerializable.h"

namespace Elastos {
	namespace SDK {

		class IAttribute :
				public ELAMessageSerializable {
		public:
			virtual ~IAttribute();
		};

		typedef boost::shared_ptr<IAttribute> AttributePtr;

	}
}

#endif //__ELASTOS_SDK_IATTRIBUTE_H__
