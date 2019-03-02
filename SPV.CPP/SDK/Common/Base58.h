// Created by Aaron Voisine on 9/15/15.
// Copyright (c) 2015 breadwallet LLC
// Copyright (c) 2017-2019 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef __ELASTOS_SDK_BASE58_H__
#define __ELASTOS_SDK_BASE58_H__

#include <SDK/Common/CMemBlock.h>
#include <string>

namespace Elastos {
	namespace ElaWallet {
		class Base58 {
		public:
			static std::string Encode(const void *data, size_t dataLen);

			static CMBlock Decode(const std::string &str);

			static std::string CheckEncode(const void *data, size_t dataLen);

			static CMBlock CheckDecode(const std::string &str);

		};
	}
}


#endif //SPVSDK_BASE58_H
