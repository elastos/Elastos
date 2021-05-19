// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef __ELASTOS_SDK_IPLUGIN_H__
#define __ELASTOS_SDK_IPLUGIN_H__

#include "IMerkleBlock.h"

namespace Elastos {
	namespace ElaWallet {

		class IPlugin {
		public:
			virtual ~IPlugin() {}

			virtual MerkleBlockPtr CreateBlock() = 0;
		};

	}
}

#endif //__ELASTOS_SDK_IPLUGIN_H__
