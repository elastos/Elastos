// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <sstream>
#include "Utils.h"

namespace Elastos {
	namespace SDK {

		std::string Utils::UInt256ToString(const UInt256 &u256) {
			std::stringstream ss;

			for (int i = 0; i < sizeof(u256.u8); ++i) {
				ss << (char)_hexc(u256.u8[i] >> 4) << (char)_hexc(u256.u8[i]);
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
	}
}