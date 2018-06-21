// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef __ELASTOS_SDK_ELAPEERCONFIG_H__
#define __ELASTOS_SDK_ELAPEERCONFIG_H__

#include <nlohmann/json.hpp>

namespace Elastos {
	namespace ElaWallet {

		static nlohmann::json ElaPeerConfig =
					R"(
						  {
							"MagicNumber": 7630401,
							"KnowingPeers":
							[
								{
									"Address": "54.165.10.201",
									"Port": 20866,
									"Timestamp": 0,
									"Services": 1,
									"Flags": 0
								},
								{
									"Address": "34.198.67.91",
									"Port": 20866,
									"Timestamp": 0,
									"Services": 1,
									"Flags": 0
								},
								{
									"Address": "52.55.40.251",
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
