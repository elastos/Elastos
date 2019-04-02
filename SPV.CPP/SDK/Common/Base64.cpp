/**
 *  base64.cpp and base64.h
 *
 *  base64 encoding and decoding with C++.
 *
 *  Version: 1.01.00
 *
 *  Copyright (C) 2004-2017 René Nyffenegger
 *
 *  This source code is provided 'as-is', without any express or implied
 *  warranty. In no event will the author be held liable for any damages
 *  arising from the use of this software.
 *
 *  Permission is granted to anyone to use this software for any purpose,
 *  including commercial applications, and to alter it and redistribute it
 *  freely, subject to the following restrictions:
 *
 *  1. The origin of this source code must not be misrepresented; you must not
 *     claim that you wrote the original source code. If you use this source code
 *     in a product, an acknowledgment in the product documentation would be
 *     appreciated but is not required.
 *
 *  2. Altered source versions must be plainly marked as such, and must not be
 *     misrepresented as being the original source code.
 *
 *  3. This notice may not be removed or altered from any source distribution.
 *
 *  René Nyffenegger rene.nyffenegger@adp-gmbh.ch
 */

#include "Base64.h"

#include <SDK/Common/ByteStream.h>
#include <openssl/bio.h>
#include <openssl/evp.h>
#include <openssl/buffer.h>

namespace Elastos {
	namespace ElaWallet {

		static const std::string base64_chars =
			"ABCDEFGHIJKLMNOPQRSTUVWXYZ"
			"abcdefghijklmnopqrstuvwxyz"
			"0123456789+/";

		std::string Base64::Encode(const void *input, size_t len) {
			return Encode(bytes_t(input, len));
		}

		std::string Base64::Encode(const bytes_t &input) {
			BIO *bio, *b64;
			BUF_MEM *bufferPtr;
			b64 = BIO_new(BIO_f_base64());
			bio = BIO_new(BIO_s_mem());
			bio = BIO_push(b64, bio);

			BIO_set_flags(bio, BIO_FLAGS_BASE64_NO_NL);
			BIO_write(bio, &input[0], input.size());
			BIO_flush(bio);
			BIO_get_mem_ptr(bio, &bufferPtr);
			std::string result(bufferPtr->data, bufferPtr->length);
			BIO_set_close(bio, BIO_CLOSE);
			BIO_free_all(bio);
			return result;
		}

		bytes_t Base64::Decode(const std::string &input) {
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

	}
}
