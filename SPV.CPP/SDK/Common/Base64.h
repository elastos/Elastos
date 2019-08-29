// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef __ELASTOS_SDK_BASE64_H__
#define __ELASTOS_SDK_BASE64_H__

#include "typedefs.h"

#include <string>
#include <openssl/bio.h>
#include <openssl/evp.h>
#include <openssl/buffer.h>

namespace Elastos {
	namespace ElaWallet {
		class Base64 {
		public:
			static std::string Encode(const void *input, size_t inputLen);

			static std::string Encode(const bytes_t &input);

			static bytes_t Decode(const std::string &input);

		};
	}
}


#endif //__ELASTOS_SDK_BASE64_H__
