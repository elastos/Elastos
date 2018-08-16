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
							"MagicNumber": 2018001,
							"KnowingPeers":
							[
								{
									"Address": "13.210.251.118",
									"Port": 21866,
									"Timestamp": 0,
									"Services": 1,
									"Flags": 0
								},
								{
									"Address": "18.194.136.248",
									"Port": 21866,
									"Timestamp": 0,
									"Services": 1,
									"Flags": 0
								},
								{
									"Address": "35.177.55.45",
									"Port": 21866,
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
