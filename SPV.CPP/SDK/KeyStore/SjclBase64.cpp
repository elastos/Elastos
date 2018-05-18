// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <vector>
#include <assert.h>

#include "BRInt.h"

#include "SjclBase64.h"

namespace Elastos {
	namespace SDK {

		std::string SjclBase64::_chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

		std::vector<unsigned char> SjclBase64::toBits(const std::string &base64Str) {
			std::string str = base64Str;
			str.erase(str.find_last_not_of("=") + 1);

			std::vector<unsigned char> out;
			uint32_t i, bits = 0;
			uint32_t ta = 0;
			size_t x;
			for (i = 0; i < str.size(); i++) {
				x = _chars.find(str.at(i), 0);
				if (x == std::string::npos) {
					out.clear();
					return out;
				}
				if (bits > 26) {
					bits -= 26;

					std::vector<unsigned char> temp = convertToCharArray(ta ^ x >> bits);
					out.insert(out.end(), temp.begin(), temp.end());
					ta = x << (32 - bits);
				} else {
					bits += 6;
					ta ^= x << (32 - bits);
				}
			}
			if (bits & 56) {
				out.push_back(partial(bits & 56, ta, 1));
			}
			return out;
		}

		std::string SjclBase64::fromBits(const unsigned char *bitArray, size_t length) {
			assert(length % 4 == 0);

			std::vector<uint32_t> arr = convertToUint32(bitArray, length);

			std::string out;
			uint32_t i = 0, bits = 0;
			uint32_t ta = 0;
			size_t bl = length * 8;
			for (i = 0; out.size() * 6 < bl;) {
				out += arr.size() > i
					   ? _chars.at((ta ^ arr[i] >> bits) >> 26)
					   : _chars.at(ta >> 26);
				if (bits < 6) {
					ta = arr[i] << (6 - bits);
					bits += 26;
					i++;
				} else {
					ta <<= 6;
					bits -= 6;
				}
			}
			while ((out.size() & 3)) { out += "="; }
			return out;
		}

		uint32_t SjclBase64::partial(uint32_t len, uint32_t x, int end) {
			if (len == 32) { return x; }
			return (end ? x | 0 : x << (32 - len)) + len * 0x10000000000;
		}

		std::vector<unsigned char> SjclBase64::convertToCharArray(uint32_t value) {
			uint8_t data[32 / 8];
			UInt32SetBE(data, value);

			std::vector<unsigned char> result;
			for (int i = 0; i < 32 / 8; ++i) {
				result.push_back(data[i]);
			}
			return result;
		}

		std::vector<uint32_t> SjclBase64::convertToUint32(const unsigned char *bitArray, size_t length) {
			std::vector<uint32_t> result;

			for (int i = 0; i < length; i += 4) {
				result.push_back(UInt32GetBE(&bitArray[i]));
			}
			return result;
		}
	}
}