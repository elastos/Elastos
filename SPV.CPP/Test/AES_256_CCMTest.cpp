// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#define CATCH_CONFIG_MAIN

#include <fstream>
#include <catch.hpp>

#include "AES_256_CCM.h"

using namespace Elastos::SDK;

TEST_CASE("encrypt/decrypt", "[AES_256_CCM]") {
	static unsigned char iv[] = {0x9F, 0x62, 0x54, 0x4C, 0x9D, 0x3F, 0xCA, 0xB2, 0xDD, 0x08, 0x33, 0xDF, 0x21,
								 0xCA, 0x80,
								 0xCF};
	static unsigned char salt[] = {0x65, 0x15, 0x63, 0x6B, 0x82, 0xC5, 0xAC, 0x56};

	SECTION("encrypt/decrypt") {
		unsigned char plaintext[5] = {0, 1, 2, 3, 4};

		CMBlock cmCipher = AES_256_CCM::encrypt(plaintext, sizeof(plaintext),
												(unsigned char *) "password", strlen("password"), salt, sizeof(salt),
												iv, sizeof(iv));

		CMBlock cmPlain = AES_256_CCM::decrypt(cmCipher, cmCipher.GetSize(),
											   (unsigned char *) "password", strlen("password"), salt, sizeof(salt), iv,
											   sizeof(iv));

		REQUIRE(sizeof(plaintext) == cmPlain.GetSize());
		REQUIRE(0 == memcmp(plaintext, cmPlain, cmPlain.GetSize()));

		unsigned char aad[5] = {0, 1, 2, 3, 4};

		cmCipher = AES_256_CCM::encrypt(plaintext, sizeof(plaintext), (unsigned char *) "password", strlen("password"),
										salt, sizeof(salt), iv, sizeof(iv), false,
										aad, sizeof(aad));

		cmPlain = AES_256_CCM::decrypt(cmCipher, cmCipher.GetSize(), (unsigned char *) "password", strlen("password"),
									   salt, sizeof(salt), iv, sizeof(iv), false,
									   aad, sizeof(aad));

		REQUIRE(sizeof(plaintext) == cmPlain.GetSize());
		REQUIRE(0 == memcmp(plaintext, cmPlain, cmPlain.GetSize()));

	}
}
