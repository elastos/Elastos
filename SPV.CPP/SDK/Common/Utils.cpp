// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <sstream>
#include <stdlib.h>
#include "assert.h"
#include "Utils.h"

namespace Elastos {
	namespace SDK {

		std::string Utils::UInt256ToString(const UInt256 &u256) {
			std::stringstream ss;

			for (int i = 0; i < sizeof(u256.u8); ++i) {
				ss << (char) _hexc(u256.u8[i] >> 4) << (char) _hexc(u256.u8[i]);
			}

			return ss.str();
		}

		UInt256 Utils::UInt256FromString(const std::string &s) {
			UInt256 result = {0};

			for (int i = 0; i < sizeof(result.u8); ++i) {
				result.u8[i] = (_hexu((s)[2 * i]) << 4) | _hexu((s)[2 * i + 1]);
			}

			return result;
		}

		std::string Utils::UInt168ToString(const UInt168 &u168) {
			std::stringstream ss;

			for (int i = 0; i < sizeof(u168.u8); ++i) {
				ss << (char) _hexc(u168.u8[i] >> 4) << (char) _hexc(u168.u8[i]);
			}

			return ss.str();
		}

		UInt168 Utils::UInt168FromString(const std::string &str) {
			UInt168 result = {0};

			for (int i = 0; i < sizeof(result.u8); ++i) {
				result.u8[i] = (_hexu((str)[2 * i]) << 4) | _hexu((str)[2 * i + 1]);
			}

			return result;
		}

		void Utils::decodeHex(uint8_t *target, size_t targetLen, char *source, size_t sourceLen) {
			assert (0 == sourceLen % 2);
			assert (2 * targetLen == sourceLen);

			for (int i = 0; i < targetLen; i++) {
				target[i] = (uint8_t) ((_hexu(source[2 * i]) << 4) | _hexu(source[(2 * i) + 1]));
			}
		}

		size_t Utils::decodeHexLength(size_t stringLen) {
			assert (0 == stringLen % 2);
			return stringLen / 2;
		}

		uint8_t *Utils::decodeHexCreate(size_t *targetLen, char *source, size_t sourceLen) {
			size_t length = decodeHexLength(sourceLen);
			if (nullptr != targetLen) *targetLen = length;
			uint8_t *target = (uint8_t *) malloc(length);
			decodeHex(target, length, source, sourceLen);
			return target;
		}

		void Utils::encodeHex(char *target, size_t targetLen, uint8_t *source, size_t sourceLen) {
			assert (targetLen == 2 * sourceLen + 1);

			for (int i = 0; i < sourceLen; i++) {
				target[2 * i] = (uint8_t) _hexc (source[i] >> 4);
				target[2 * i + 1] = (uint8_t) _hexc (source[i]);
			}
			target[2 * sourceLen] = '\0';
		}

		size_t Utils::encodeHexLength(size_t byteArrayLen) {
			return 2 * byteArrayLen + 1;
		}

		char *Utils::encodeHexCreate(size_t *targetLen, uint8_t *source, size_t sourceLen) {
			size_t length = encodeHexLength(sourceLen);
			if (nullptr != targetLen) *targetLen = length;
			char *target = (char *) malloc(length);
			encodeHex(target, length, source, sourceLen);
			return target;
		}

		UInt128 Utils::generateRandomSeed() {
			UInt128 result;
			//todo [zxb] complete me
			return result;
		}

		CMBlock Utils::encrypt(const CMBlock &data, const std::string &password) {
			//todo complete me
			return CMBlock();
		}

		CMBlock Utils::decrypt(const CMBlock &encryptedData, const std::string &password) {
			//todo complete me
			return CMBlock();
		}
	}
}
