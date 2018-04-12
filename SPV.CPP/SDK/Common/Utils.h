// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef __ELASTOS_SDK_UTILS_H__
#define __ELASTOS_SDK_UTILS_H__

#include <string>

#include "BRInt.h"

namespace Elastos {
	namespace SDK {

		class Utils {
		public:
			static std::string UInt256ToString(const UInt256 &u256);
			static UInt256 UInt256FromString(const std::string &u256);

			static void	decodeHex (uint8_t *target, size_t targetLen, char *source, size_t sourceLen);
			static size_t decodeHexLength (size_t stringLen);
			static uint8_t *decodeHexCreate (size_t *targetLen, char *source, size_t sourceLen);
			static void	encodeHex (char *target, size_t targetLen, uint8_t *source, size_t sourceLen);
			static size_t encodeHexLength(size_t byteArrayLen);
			static char *encodeHexCreate (size_t *targetLen, uint8_t *source, size_t sourceLen);
		};
	}
}

#endif //__ELASTOS_SDK_UTILS_H__
