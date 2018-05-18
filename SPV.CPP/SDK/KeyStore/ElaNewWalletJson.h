// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef __ELASTOS_SDK_ELANEWWALLETJSON_H__
#define __ELASTOS_SDK_ELANEWWALLETJSON_H__

#include "ElaWebWalletJson.h"

namespace Elastos {
	namespace SDK {

		class ElaNewWalletJson : ElaWebWalletJson{
		public:
			ElaNewWalletJson();

			~ElaNewWalletJson();

			//todo add id related data
		};

		//support for json converting
		//read "Arbitrary types conversions" section in readme of
		//	https://github.com/nlohmann/json for more details
		void to_json(nlohmann::json &j, const ElaNewWalletJson &p);

		void from_json(const nlohmann::json &j, ElaNewWalletJson &p);

	}
}

#endif //__ELASTOS_SDK_ELANEWWALLETJSON_H__
