// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef __ELASTOS_SDK_BTCBASE58_H__
#define __ELASTOS_SDK_BTCBASE58_H__

#include <string>

#include <CMemBlock.h>

namespace Elastos {
	namespace SDK {

		class BTCBase58 {
		public:
			static std::string EncodeBase58(unsigned char *pData, size_t pDataLen);
			static CMemBlock<unsigned char> DecodeBase58(const std::string &str);
		};

	}
}


#endif //__ELASTOS_SDK_BTCBASE58_H__
