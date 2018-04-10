// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef __ELASTOS_SDK_UTILS_H__
#define __ELASTOS_SDK_UTILS_H__

#include <string>
#include <BRInt.h>

namespace Elastos {
	namespace SDK {

		class Utils {
		public:
			static std::string UInt256ToString(const UInt256 &u256);
			static UInt256 UInt256FromString(const std::string &u256);
		};
	}
}

#endif //__ELASTOS_SDK_UTILS_H__
