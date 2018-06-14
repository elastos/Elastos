// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef __ELASTOS_SDK_ELAWEBWALLETJSON_H__
#define __ELASTOS_SDK_ELAWEBWALLETJSON_H__

#include "BitcoreWalletClientJson.h"
#include "Mstream.h"

namespace Elastos {
	namespace SDK {
		
		class ElaWebWalletJson : public BitcoreWalletClientJson {
		public:
			ElaWebWalletJson();

			virtual ~ElaWebWalletJson();

		private:
			JSON_SM_LS(ElaWebWalletJson);
			JSON_SM_RS(ElaWebWalletJson);
			TO_JSON(ElaWebWalletJson);
			FROM_JSON(ElaWebWalletJson);
		};
	}
}

#endif //__ELASTOS_SDK_ELAWEBWALLETJSON_H__
