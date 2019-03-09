// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#define CATCH_CONFIG_MAIN

#include "catch.hpp"

#include "TestHelper.h"
#include <Core/BRInt.h>
#include <SDK/Common/Utils.h>
#include <SDK/Common/Log.h>
#include <SDK/Implement/SidechainSubWallet.h>

using namespace Elastos::ElaWallet;


TEST_CASE("Utils test", "[Utils]") {
	std::string rawStr = "6418be20291bc857c9a01e5ba205445b85a0593d47cc0b576d55a55e464f31b3";
	std::string expectStr = "99ca9a4467b547c19a6554021fd1b5b455b29d1adddbd910dd437bc143785767";

	SECTION("UInt256 to and from std::string") {
		SECTION("UInt256 and string converting") {
			UInt256 u1 = uint256("6418be20291bc857c9a01e5ba205445b85a0593d47cc0b576d55a55e464f31b3");
			UInt256 u2 = Utils::UInt256FromString(rawStr);
			REQUIRE(0 == memcmp(&u1, &u2, sizeof(UInt256)));

			u1 = uint256("99ca9a4467b547c19a6554021fd1b5b455b29d1adddbd910dd437bc143785767");
			REQUIRE(expectStr == Utils::UInt256ToString(u1));
		}

		SECTION("UInt256 and string reverse converting") {
			UInt256 u1 = uint256("b3314f465ea5556d570bcc473d59a0855b4405a25b1ea0c957c81b2920be1864");
			UInt256 u2 = Utils::UInt256FromString(rawStr, true);
			REQUIRE(0 == memcmp(&u1, &u2, sizeof(UInt256)));

			u1 = uint256("67577843c17b43dd10d9dbdd1a9db255b4b5d11f0254659ac147b567449aca99");
			REQUIRE(expectStr == Utils::UInt256ToString(u1, true));
		}
	}

	SECTION("Key encodeHex and decodeHex test") {
		CMBlock data = getRandCMBlock(100);
		std::string str = Utils::EncodeHex(data);
		CMBlock decodeData = Utils::DecodeHex(str);
		REQUIRE(decodeData == data);

		std::string dataString = "67577843c17b43dd10d9dbdd1a9db255b4b5d11f0254659ac147b567449aca99";
		CMBlock dataDecode = Utils::DecodeHex(dataString);
		REQUIRE(dataString == Utils::EncodeHex(dataDecode));
	}

	SECTION("ProgramHash and AddressHash Test") {
		UInt168 expectedHash = Utils::UInt168FromString("213a3b4511636bf45a582a02b2ee0a0d3c9c52dfe1");
		std::string redeemScript = "21022c9652d3ad5cc065aa9147dc2ad022f80001e8ed233de20f352950d351d472b7ac";

		UInt168 hash = Key::CodeToProgramHash(PrefixStandard, Utils::DecodeHex(redeemScript));

		REQUIRE(UInt168Eq(&hash, &expectedHash) == 1);

		std::string addr = Utils::UInt168ToAddress(hash);
		REQUIRE("ENTogr92671PKrMmtWo3RLiYXfBTXUe13Z" == addr);
	}

	SECTION("Encrypt and decrypt") {

		SECTION("Encrypt and decrypt test with CMBlock data") {
			CMBlock plainData = getRandCMBlock(50);
			std::string cipherString;
			std::string passwd = "123456789";
			REQUIRE(Utils::Encrypt(cipherString, plainData, passwd));

			CMBlock plainDataDecrypt;
			REQUIRE(Utils::Decrypt(plainDataDecrypt, cipherString, passwd));
			REQUIRE(plainData == plainDataDecrypt);

			std::string wrongPasswd = "00000000";
			REQUIRE(!Utils::Decrypt(plainDataDecrypt, cipherString, wrongPasswd));
		}

		SECTION("Encrypt and decrypt test with std::string") {
			std::string plainString = getRandString(100);
			std::string cipherString;
			std::string passwd = "1234567890";
			REQUIRE(Utils::Encrypt(cipherString, plainString, passwd));

			std::string plainStringDecrypt;
			REQUIRE(Utils::Decrypt(plainStringDecrypt, cipherString, passwd));
			REQUIRE(plainString == plainStringDecrypt);

			std::string wrongPasswd = "9876543210";
			REQUIRE(!Utils::Decrypt(plainStringDecrypt, cipherString, wrongPasswd));
		}

	}

}
