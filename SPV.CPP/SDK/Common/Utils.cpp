// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "Utils.h"

#include <stdlib.h>
#include <algorithm>
#include <iterator>

namespace Elastos {
	namespace ElaWallet {

		bytes_t Utils::GetRandom(size_t bytes) {
			bytes_t out(bytes);

			for (size_t i = 0; i < out.size(); i++) {
				out[i] = Utils::getRandomByte();
			}

			return out;
		}

	}
}
