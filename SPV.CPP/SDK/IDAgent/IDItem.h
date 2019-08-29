// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef __ELASTOS_SDK_IDPATH_H__
#define __ELASTOS_SDK_IDPATH_H__

#include <SDK/Common/Mstream.h>
#include <SDK/Common/typedefs.h>

#include <nlohmann/json.hpp>

#include <cstdint>
#include <string>

namespace Elastos {
	namespace ElaWallet {

		struct IDItem {
			IDItem() :
					Purpose(0),
					Index(0) {
			}

			IDItem(uint32_t purpose, uint32_t index, const bytes_t &pubKey = bytes_t()) :
					Purpose(purpose),
					Index(index),
					PublicKey(pubKey) {
			}

			inline bool operator<(const IDItem &b) {
				if (Purpose != b.Purpose)
					return Purpose < b.Purpose;
				return Index < b.Index;
			}

			inline bool operator==(const IDItem &b) {
				return Purpose == b.Purpose && Index == b.Index;
			}

			JSON_SM_LS(IDItem);

			JSON_SM_RS(IDItem);

			TO_JSON(IDItem);

			FROM_JSON(IDItem);

			uint32_t Purpose;
			uint32_t Index;
			bytes_t PublicKey;
		};

	}
}

#endif //__ELASTOS_SDK_IDPATH_H__
