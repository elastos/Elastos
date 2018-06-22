// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef __ELASTOS_SDK_TESTHELPER_H__
#define __ELASTOS_SDK_TESTHELPER_H__

#define ARRAY_SIZE(a) (sizeof(a) / sizeof((a)[0]))
#define TEST_ASCII_BEGIN 48

namespace Elastos {
	namespace ElaWallet {

		static UInt256 getRandUInt256(void) {
			UInt256 u;
			for (size_t i = 0; i < ARRAY_SIZE(u.u32); ++i) {
				u.u32[i] = rand();
			}
			return u;
		}

		static CMBlock getRandCMBlock(size_t size) {
			CMBlock block(size);

			for (size_t i = 0; i < size; ++i) {
				block[i] = (uint8_t) rand();
			}

			return block;
		}

		static std::string getRandString(size_t length) {
			char buf[length];
			for (size_t i = 0; i < length; ++i) {
				buf[i] = static_cast<uint8_t>(TEST_ASCII_BEGIN + rand() % 75);
			}

			return std::string(buf);
		}

		static UInt168 getRandUInt168(void) {
			UInt168 u;
			for (size_t i = 0; i < ARRAY_SIZE(u.u8); ++i) {
				u.u8[i] = rand();
			}
			return u;
		}

	}
}

#endif //__ELASTOS_SDK_TESTHELPER_H__
