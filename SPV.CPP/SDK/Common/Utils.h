// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef __ELASTOS_SDK_UTILS_H__
#define __ELASTOS_SDK_UTILS_H__

#include "typedefs.h"

namespace Elastos {
	namespace ElaWallet {

		class Utils {
		public:

			static uint8_t getRandomByte();

			static bytes_t GetRandom(size_t bytes);
		};
	}
}

#endif //__ELASTOS_SDK_UTILS_H__
