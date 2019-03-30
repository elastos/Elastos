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
			static std::string Encode(const void *input, size_t inputLen) {
				BIO *bio, *b64;
				BUF_MEM *bufferPtr;
				b64 = BIO_new(BIO_f_base64());
				bio = BIO_new(BIO_s_mem());
				bio = BIO_push(b64, bio);

				BIO_set_flags(bio, BIO_FLAGS_BASE64_NO_NL);
				BIO_write(bio, input, inputLen);
				BIO_flush(bio);
				BIO_get_mem_ptr(bio, &bufferPtr);
				std::string result(bufferPtr->data, bufferPtr->length);
				BIO_set_close(bio, BIO_CLOSE);
				BIO_free_all(bio);
				return result;
			}

			static std::string Encode(const bytes_t &input) {
				return Encode(input.data(), input.size());
			}

			static bytes_t Decode(const std::string &input) {
				size_t len = input.size(), padding = 0;
				if (input[len - 1] == '=' && input[len - 2] == '=') //last two chars are =
					padding = 2;
				else if (input[len - 1] == '=') //last char is =
					padding = 1;

				int decodedLen = (len*3)/4 - padding;

				BIO *bio, *b64;

				bytes_t buffer(decodedLen);

				bio = BIO_new_mem_buf(input.c_str(), -1);
				b64 = BIO_new(BIO_f_base64());
				bio = BIO_push(b64, bio);

				BIO_set_flags(bio, BIO_FLAGS_BASE64_NO_NL);
				decodedLen = BIO_read(bio, &buffer[0], input.size());
				BIO_free_all(bio);
				buffer.resize(decodedLen);
				return buffer;
			}

		};
	}
}


#endif //__ELASTOS_SDK_BASE64_H__
