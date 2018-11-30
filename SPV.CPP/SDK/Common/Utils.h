// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef __ELASTOS_SDK_UTILS_H__
#define __ELASTOS_SDK_UTILS_H__

#include <string>
#include <assert.h>
#include <random>
#include<functional>

#include "BRInt.h"

#include "CMemBlock.h"

namespace Elastos {
	namespace ElaWallet {

		class Utils {
		public:

			static std::string UInt256ToString(const UInt256 &u256, bool reverse = false);

			static UInt256 UInt256FromString(const std::string &u256, bool reverse = false);

			static std::string UInt168ToString(const UInt168 &u168);

			static UInt168 UInt168FromString(const std::string &str);

			static std::string UInt128ToString(const UInt128 &u128);

			static UInt128 UInt128FromString(const std::string &str);

			static inline uint8_t getRandomByte() {
				std::random_device rd;
				std::mt19937_64 gen(rd());
				std::uniform_int_distribution<> dis(0, 255);
				auto dice = std::bind(dis, gen);
				return dice();
			}

			static bool Encrypt(std::string &ctBase64, const std::string &data, const std::string &passwd);

			static bool Decrypt(std::string &data, const std::string &ctBase64, const std::string &passwd);

			static bool Encrypt(std::string &ctBase64, const CMBlock &data, const std::string &passwd);

			static bool Decrypt(CMBlock &data, const std::string &ctBase64, const std::string &passwd);

			static std::string encodeHex(const CMBlock &in);

			static std::string encodeHex(const uint8_t *hex, size_t hexLen);

			static CMBlock decodeHex(const std::string &s);

			static std::string convertToString(const CMBlock &data) {
				char p[data.GetSize()];
				memcpy(p, data, data.GetSize());
				std::string ret(p, data.GetSize());
				return ret;
			}

			static CMBlock convertToMemBlock(const std::string &str) {
				CMBlock result(str.size());
				memcpy(result, str.c_str(), str.size());
				return result;
			}

			static std::string UInt168ToAddress(const UInt168 &u);

			static bool UInt168FromAddress(UInt168 &u, const std::string &address);

			static uint32_t getAddressTypeBySignType(const int signType);

			static UInt168 codeToProgramHash(const std::string &redeemScript);

			static UInt168 codeToProgramHash(const CMBlock &redeemScript);

		};
	}
}

#endif //__ELASTOS_SDK_UTILS_H__
