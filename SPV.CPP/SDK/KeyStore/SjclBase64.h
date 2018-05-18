// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef __ELASTOS_SDK_SJCLBASE64_H__
#define __ELASTOS_SDK_SJCLBASE64_H__

#include <string>

namespace Elastos {
	namespace SDK {

		class SjclBase64 {
		public:
			static std::vector<unsigned char> toBits(const std::string &base64Str);

			static std::string fromBits(const unsigned char *bits, size_t length);

		private:
			static uint32_t partial(uint32_t len, uint32_t x, int end);

			static std::vector<unsigned char> convertToCharArray(uint32_t value);

			static std::vector<uint32_t> convertToUint32(const unsigned char *bitArray, size_t length);

		private:
			static std::string _chars;
		};

	}
}

#endif //__ELASTOS_SDK_SJCLBASE64_H__
