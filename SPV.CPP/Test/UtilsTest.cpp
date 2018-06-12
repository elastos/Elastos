// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#define CATCH_CONFIG_MAIN

#include "catch.hpp"

#include "CMemBlock.h"
#include "Utils.h"

using namespace Elastos::SDK;


TEST_CASE("Utils test 0", "[Utils]") {
	SECTION("UInt256 and string converting") {
		std::string rawStr = "000000000019d6689c085ae165831e934ff763ae46a2a6c172b3f1b60a8ce26f";
		UInt256 u1 = {
			.u8 = {
				0x00, 0x00, 0x00, 0x00, 0x00, 0x19, 0xd6, 0x68, 0x9c, 0x08, 0x5a, 0xe1, 0x65, 0x83, 0x1e, 0x93,
				0x4f, 0xf7, 0x63, 0xae, 0x46, 0xa2, 0xa6, 0xc1, 0x72, 0xb3, 0xf1, 0xb6, 0x0a, 0x8c, 0xe2, 0x6f
			}
		};
		UInt256 u2 = Utils::UInt256FromString(rawStr);
		REQUIRE(0 == memcmp(&u1, &u2, sizeof(UInt256)));


		u1 = {
			.u8 = {
				0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0x00, 0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF,
				0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0x00, 0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF
			}
		};
		REQUIRE("11223344556677889900aabbccddeeff11223344556677889900aabbccddeeff" == Utils::UInt256ToString(u1));
	}

	SECTION("Key encodeHex and decodeHex test") {
		uint8_t bytes1[] = "\x00\x01\x02\x03\x04\x05\x06\x07\x08\x09\x0A\x0B\x0C\x0D\x0E\x0F";
		CMBlock data;
		data.SetMemFixed(bytes1, sizeof(bytes1) - 1);

		std::string str = Utils::encodeHex(data);
		REQUIRE(str == "000102030405060708090a0b0c0d0e0f");

		CMBlock decodeByte = Utils::decodeHex(str);
		int res = memcmp(bytes1, decodeByte, decodeByte.GetSize());
		REQUIRE(res == 0);
	}
}

TEST_CASE("Utils test 1", "[Utils]") {
	std::string rawStr = "000000000019d6689c085ae165831e934ff763ae46a2a6c172b3f1b60a8ce26f";
	UInt256 u1 = {
		.u8 = {
			0x00, 0x00, 0x00, 0x00, 0x00, 0x19, 0xd6, 0x68, 0x9c, 0x08, 0x5a, 0xe1, 0x65, 0x83, 0x1e, 0x93,
			0x4f, 0xf7, 0x63, 0xae, 0x46, 0xa2, 0xa6, 0xc1, 0x72, 0xb3, 0xf1, 0xb6, 0x0a, 0x8c, 0xe2, 0x6f
		}
	};
	UInt256 u2 = Utils::UInt256FromString(rawStr);
	REQUIRE(0 == memcmp(&u1, &u2, sizeof(UInt256)));


	u1 = {
		.u8 = {
			0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0x00, 0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF,
			0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0x00, 0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF
		}
	};
	REQUIRE("11223344556677889900aabbccddeeff11223344556677889900aabbccddeeff" == Utils::UInt256ToString(u1));
}

TEST_CASE ("enctrypt/decrept content without nothing and password with meaning", "[aes crypto]") {
	SECTION("Normal") {
		std::string password = "password";
		const char content[] = "This is crypto testing.";
		CMBlock DataToBeEncrypted(sizeof(content));
		DataToBeEncrypted.Zero();
		memcpy(DataToBeEncrypted, content, sizeof(content));
		CMBlock DataBeEncrypted;
		CMBlock DataBeDecrypted;
		DataBeEncrypted = Utils::encrypt(DataToBeEncrypted, password);
		DataBeDecrypted = Utils::decrypt(DataBeEncrypted, password);

		REQUIRE(0 == strcmp(content, (const char *) (const void *) DataBeDecrypted));
	}
}

TEST_CASE ("enctrypt/decrept content without nothing and password with meaning, which depand on chinese",
		   "[aes crypto]") {
	SECTION("Normal") {
		std::string password = "密码";
		const char content[] = "现在进行加密测试";
		CMBlock DataToBeEncrypted(sizeof(content));
		DataToBeEncrypted.Zero();
		memcpy(DataToBeEncrypted, content, sizeof(content));
		CMBlock DataBeEncrypted;
		CMBlock DataBeDecrypted;
		DataBeEncrypted = Utils::encrypt(DataToBeEncrypted, password);
		DataBeDecrypted = Utils::decrypt(DataBeEncrypted, password);

		REQUIRE(0 == strcmp(content, (const char *) (const void *) DataBeDecrypted));
	}
}

TEST_CASE ("enctrypt/decrept content with "" and password with meaning", "[aes crypto]") {
	SECTION("None Normal") {
		std::string password = "password";
		const char content[] = "";
		CMBlock DataToBeEncrypted(sizeof(content));
		DataToBeEncrypted.Zero();
		memcpy(DataToBeEncrypted, content, sizeof(content));
		CMBlock DataBeEncrypted;
		CMBlock DataBeDecrypted;
		DataBeEncrypted = Utils::encrypt(DataToBeEncrypted, password);
		DataBeDecrypted = Utils::decrypt(DataBeEncrypted, password);

		REQUIRE(0 == strcmp(content, (const char *) (const void *) DataBeDecrypted));
	}
}

TEST_CASE ("enctrypt/decrept content with nothing"" and password with meaning", "[aes crypto]") {
	SECTION("None Normal") {
		std::string password = "password";
		CMBlock DataToBeEncrypted;
		CMBlock DataBeEncrypted;
		CMBlock DataBeDecrypted;
		DataBeEncrypted = Utils::encrypt(DataToBeEncrypted, password);
		DataBeDecrypted = Utils::decrypt(DataBeEncrypted, password);

		REQUIRE((const void *) DataBeDecrypted == (const void *) DataToBeEncrypted);
	}
}

TEST_CASE ("enctrypt/decrept content without nothing and password with "" ", "[aes crypto]") {
	SECTION("None Normal") {
		std::string password = "";
		const char content[] = "This is crypto testing.";
		CMBlock DataToBeEncrypted(sizeof(content));
		DataToBeEncrypted.Zero();
		memcpy(DataToBeEncrypted, content, sizeof(content));
		CMBlock DataBeEncrypted;
		CMBlock DataBeDecrypted;
		DataBeEncrypted = Utils::encrypt(DataToBeEncrypted, password);
		DataBeDecrypted = Utils::decrypt(DataBeEncrypted, password);

		REQUIRE(0 == strcmp(content, (const char *) (const void *) DataBeDecrypted));
	}
}