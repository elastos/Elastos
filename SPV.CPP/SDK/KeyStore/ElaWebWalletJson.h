// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef __ELASTOS_SDK_ELAWEBWALLETJSON_H__
#define __ELASTOS_SDK_ELAWEBWALLETJSON_H__

#include "BitcoreWalletClientJson.h"

namespace Elastos {
	namespace SDK {

		class ElaWebWalletJson : public BitcoreWalletClientJson {
		public:
			ElaWebWalletJson();

			virtual ~ElaWebWalletJson();

		private:
			std::string _mnemonic;
		};

		//support for json converting
		//read "Arbitrary types conversions" section in readme of
		//	https://github.com/nlohmann/json for more details
		void to_json(nlohmann::json &j, const ElaWebWalletJson &p);

		void from_json(const nlohmann::json &j, ElaWebWalletJson &p);

	}
}

#endif //__ELASTOS_SDK_ELAWEBWALLETJSON_H__
