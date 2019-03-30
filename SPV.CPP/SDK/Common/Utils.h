// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef __ELASTOS_SDK_UTILS_H__
#define __ELASTOS_SDK_UTILS_H__

#include "typedefs.h"

#include <string>
#include <cassert>
#include <random>
#include <functional>
#include <set>

namespace Elastos {
	namespace ElaWallet {

		class Utils {
		public:

			static inline uint8_t getRandomByte() {
				std::random_device rd;
				std::mt19937_64 gen(rd());
				std::uniform_int_distribution<> dis(0, 255);
				auto dice = std::bind(dis, gen);
				return dice();
			}

			static bytes_t GetRandom(size_t bytes) {
				bytes_t out(bytes);

				for (size_t i = 0; i < out.size(); i++) {
					out[i] = Utils::getRandomByte();
				}

				return out;
			}
		};
	}
}

#endif //__ELASTOS_SDK_UTILS_H__
