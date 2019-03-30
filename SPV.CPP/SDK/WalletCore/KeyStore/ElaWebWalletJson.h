// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef __ELASTOS_SDK_ELAWEBWALLETJSON_H__
#define __ELASTOS_SDK_ELAWEBWALLETJSON_H__

#include "BitcoreWalletClientJson.h"

#include <SDK/Common/Mstream.h>

namespace Elastos {
	namespace ElaWallet {
		
		class ElaWebWalletJson : public BitcoreWalletClientJson {
		public:
			ElaWebWalletJson();

			virtual ~ElaWebWalletJson();

			const std::string &Mnemonic() const { return _mnemonic; }

			void SetMnemonic(const std::string &m) { _mnemonic = m; }

			friend void to_json(nlohmann::json &j, const ElaWebWalletJson &p, bool withPrivKey);

			friend void from_json(const nlohmann::json &j, ElaWebWalletJson &p);

		protected:
			std::string _mnemonic;
		};
	}
}

#endif //__ELASTOS_SDK_ELAWEBWALLETJSON_H__
