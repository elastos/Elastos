// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef __ELASTOS_SDK_ELAWEBWALLETJSON_H__
#define __ELASTOS_SDK_ELAWEBWALLETJSON_H__

#include "BitcoreWalletClientJson.h"

namespace Elastos {
	namespace ElaWallet {
		
		class ElaWebWalletJson : public BitcoreWalletClientJson {
		public:
			ElaWebWalletJson();

			virtual ~ElaWebWalletJson();

			const std::string &Mnemonic() const { return _mnemonic; }

			void SetMnemonic(const std::string &m) { _mnemonic = m; }

			virtual nlohmann::json ToJson(bool withPrivKey) const;

			virtual void FromJson(const nlohmann::json &j);

		protected:
			std::string _mnemonic;
		};
	}
}

#endif //__ELASTOS_SDK_ELAWEBWALLETJSON_H__
