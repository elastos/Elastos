// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef __ELASTOS_SDK_IDCHAINSUBWALLET_H__
#define __ELASTOS_SDK_IDCHAINSUBWALLET_H__

#include "ISubWallet.h"

namespace Elastos {
	namespace SDK {

		class IdChainSubWallet : public ISubWallet {
		public:
			bool GenerateId(std::string &id, std::string &privateKey);
		};

	}
}

#endif //__ELASTOS_SDK_IDCHAINSUBWALLET_H__
