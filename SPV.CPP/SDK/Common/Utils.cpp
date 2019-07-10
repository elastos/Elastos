// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "Utils.h"

#include <random>

namespace Elastos {
	namespace ElaWallet {

		uint8_t Utils::getRandomByte() {
			std::random_device rd;
			std::mt19937_64 gen(rd());
			std::uniform_int_distribution<> dis(0, 255);
			auto dice = std::bind(dis, gen);
			return dice();
		}

		bytes_t Utils::GetRandom(size_t bytes) {
			bytes_t out(bytes);

			for (size_t i = 0; i < out.size(); i++) {
				out[i] = Utils::getRandomByte();
			}

			return out;
		}

	}
}

