// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef __ELASTOS_SDK_ELAPEERCONFIG_H__
#define __ELASTOS_SDK_ELAPEERCONFIG_H__

#include <nlohmann/json.hpp>

namespace Elastos {
	namespace SDK {

		static nlohmann::json ElaPeerConfig =
					R"(
						  {
							"MagicNumber": 7630401,
							"KnowingPeers":
							[
								{
									"Address": "127.0.0.1",
									"Port": 20866,
									"Timestamp": 0,
									"Services": 1,
									"Flags": 0
								}
							]
						}
					)"_json;

	}
}

#endif //__ELASTOS_SDK_ELAPEERCONFIG_H__
