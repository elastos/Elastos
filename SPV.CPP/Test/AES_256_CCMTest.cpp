// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#define CATCH_CONFIG_MAIN

#include <fstream>
#include <catch.hpp>

#include "AES_256_CCM.h"

using namespace Elastos::SDK;

TEST_CASE("encrypt/decrypt", "[AES_256_CCM]") {
	SECTION("encrypt/decrypt") {
		unsigned char plaintext[5] = {0, 1, 2, 3, 4};

		CMemBlock<unsigned char> cmCipher = AES_256_CCM::encrypt(plaintext, sizeof(plaintext),
																 (unsigned char *) "password", strlen("password"));

		CMemBlock<unsigned char> cmPlain = AES_256_CCM::decrypt(cmCipher, cmCipher.GetSize(),
																(unsigned char *) "password", strlen("password"));

		REQUIRE(sizeof(plaintext) == cmPlain.GetSize());
		REQUIRE(0 == memcmp(plaintext, cmPlain, cmPlain.GetSize()));

		unsigned char aad[5] = {0, 1, 2, 3, 4};

		cmCipher = AES_256_CCM::encrypt(plaintext, sizeof(plaintext), (unsigned char *) "password", strlen("password"),
										aad, sizeof(aad));

		cmPlain = AES_256_CCM::decrypt(cmCipher, cmCipher.GetSize(), (unsigned char *) "password", strlen("password"),
									   aad, sizeof(aad));

		REQUIRE(sizeof(plaintext) == cmPlain.GetSize());
		REQUIRE(0 == memcmp(plaintext, cmPlain, cmPlain.GetSize()));

	}
}