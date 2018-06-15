// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef __ELASTOS_SDK_IDPATH_H__
#define __ELASTOS_SDK_IDPATH_H__

#include <cstdint>
#include <string>

#include <nlohmann/json.hpp>

#include "KeyStore/Mstream.h"

namespace Elastos {
	namespace SDK {

		struct IdItem {
			IdItem() :
					Purpose(0),
					Index(0),
					PublicKey("") {
			}

			IdItem(uint32_t purpose, uint32_t index, const std::string &pubKey = "") :
					Purpose(purpose),
					Index(index),
					PublicKey(pubKey) {
			}

			inline bool operator<(const IdItem &b) {
				if (Purpose != b.Purpose)
					return Purpose < b.Purpose;
				return Index < b.Index;
			}

			inline bool operator==(const IdItem &b) {
				return Purpose == b.Purpose && Index == b.Index;
			}

			JSON_SM_LS(IdItem);

			JSON_SM_RS(IdItem);

			TO_JSON(IdItem);

			FROM_JSON(IdItem);

			uint32_t Purpose;
			uint32_t Index;
			std::string PublicKey;
		};

	}
}

#endif //__ELASTOS_SDK_IDPATH_H__
