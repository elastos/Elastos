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
	namespace SDK {

		class Utils {
		public:

			static std::string UInt256ToString(const UInt256 &u256);

			static UInt256 UInt256FromString(const std::string &u256);

			static std::string UInt168ToString(const UInt168 &u168);

			static UInt168 UInt168FromString(const std::string &str);

			static std::string UInt128ToString(const UInt128 &u128);

			static UInt128 UInt128FromString(const std::string &str);

			static UInt128 generateRandomSeed();

			static inline uint8_t getRandomByte() {
				std::random_device rd;
				std::mt19937_64 gen(rd());
				std::uniform_int_distribution<> dis(0, 255);
				auto dice = std::bind(dis, gen);
				return dice();
			}

			static CMBlock
			encrypt(const CMBlock &data, const std::string &password);

			static CMBlock
			decrypt(const CMBlock &encryptedData, const std::string &password);

			static CMBlock
			encrypt(const CMBlock &data, const std::string &password, CMBlock &salt, CMBlock &iv, bool bAes128 = false);

			static CMBlock
			decrypt(const CMBlock &encryptedData, const std::string &password, CMBlock &salt, CMBlock &iv,
					bool bAes128 = false);

			static void decodeHex(uint8_t *target, size_t targetLen, const char *source, size_t sourceLen);

			static size_t decodeHexLength(size_t stringLen);

			static uint8_t *decodeHexCreate(size_t *targetLen, char *source, size_t sourceLen);

			static void encodeHex(char *target, size_t targetLen, const uint8_t *source, size_t sourceLen);

			static size_t encodeHexLength(size_t byteArrayLen);

			static char *encodeHexCreate(size_t *targetLen, uint8_t *source, size_t sourceLen);

			template<class T>
			static std::string convertToString(const CMemBlock<T, uint64_t> &data) {
				assert(sizeof(T) == sizeof(char));
				char p[data.GetSize()];
				memcpy(p, data, data.GetSize());
				std::string ret(p, data.GetSize());
				return ret;
			}

			template<class T>
			static CMemBlock<T, uint64_t> convertToMemBlock(const std::string &str) {
				assert(sizeof(T) == sizeof(char));
				CMemBlock<T, uint64_t> result(str.size());
				memcpy(result, str.c_str(), str.size());
				return result;
			}

			static std::string UInt168ToAddress(const UInt168 &u168);

			static UInt168 AddressToUInt168(const std::string &address);
		};
	}
}

#endif //__ELASTOS_SDK_UTILS_H__
