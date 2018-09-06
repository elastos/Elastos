// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#define CATCH_CONFIG_MAIN

#include <boost/shared_ptr.hpp>

#include "Key.h"
#include "catch.hpp"
#include "Log.h"
#include "Utils.h"

using namespace Elastos::ElaWallet;

TEST_CASE("Key test", "[Key]") {

	SECTION("Default contructor test") {
		Key key;
		REQUIRE(key.getRaw() != nullptr);
	}

	SECTION("Contructor with BRKey") {
		BRKey *brkey = new BRKey;
		Key key(brkey);
		REQUIRE(key.getRaw() != nullptr);
	}

	SECTION("Contructor with string and CMBlock") {
		std::string priKey = "S6c56bnXQiBjk9mqSYE7ykVQ7NzrRy";
		const char *str = priKey.c_str();
		Key key(priKey);
		REQUIRE(key.getRaw() != nullptr);

		int len = 0;
		while (str[len] != '\0') {
			len++;
		}
		uint8_t data[len];
		memcpy(data, str, len);
		CMBlock byteData;
		byteData.SetMemFixed(data, len);
		Key key1(byteData);

		std::string privateKey = key.getPrivKey();
		REQUIRE(privateKey.empty() == false);
		REQUIRE(privateKey == key1.getPrivKey());

		CMBlock byteData1 = key.getPubkey();
		REQUIRE(byteData1.GetSize() > 0);
		REQUIRE(byteData1 != false);
		CMBlock byteData2 = key1.getPubkey();
		REQUIRE(byteData1.GetSize() == byteData2.GetSize());
		for (size_t i = 0; i < byteData1.GetSize(); i++) {
			REQUIRE(byteData1[i] == byteData2[i]);
		}

		std::string addr = key.address();
		REQUIRE(addr.empty() == false);
		REQUIRE(addr == key1.address());
	}

	SECTION("Contructor with secret") {
		UInt256 secret = uint256("0000000000000000000000000000000000000000000000000000000000000001");
		Key key(secret, true);
		REQUIRE(key.getRaw() != nullptr);

		UInt256 hash = key.getSecret();
		int result = UInt256Eq(&secret, &hash);
		REQUIRE(result == 1);
		REQUIRE(true == key.getCompressed());

		Key key1(secret, false);
		REQUIRE(false == key1.getCompressed());
	}

	SECTION("Key setPrivKey test") {
		Key key1;
		key1.setPrivKey("S6c56bnXQiBjk9mqSYE7ykVQ7NzrRy");
		REQUIRE(key1.getPrivKey().empty() == false);

		Key key2("S6c56bnXQiBjk9mqSYE7ykVQ7NzrRy");
		REQUIRE(key1.getPrivKey() == key2.getPrivKey());
	}

	SECTION("Key encodeSHA256 ,compactSign and verify test") {
		std::string str = "Everything should be made as simple as possible, but not simpler.";
		UInt256 hash = Key::encodeSHA256(str);
		UInt256 zero = UINT256_ZERO;
		int res = UInt256Eq(&hash, &zero);
		REQUIRE(res == 0);

		UInt256 secret = uint256("0000000000000000000000000000000000000000000000000000000000000001");
		Key key(secret, true);

		CMBlock md;
		md.SetMemFixed(hash.u8, sizeof(hash));
		CMBlock compactData = key.compactSign(md);
		std::string dataHex = Utils::encodeHex(compactData);
		REQUIRE(compactData.GetSize() == 65);

		bool ret = key.verify(hash, compactData);
		REQUIRE(ret == true);
	}

	SECTION("Key encryptNative test") {
		uint8_t bytes1[] = "\x00\x01\x02\x03\x04\x05\x06\x07\x08\x09\x0A\x0B\x0C\x0D\x0E\x0F";
		CMBlock data;
		data.SetMemFixed(bytes1, sizeof(bytes1));
		uint8_t bytes2[] = "\x50\x51\x52\x53\xc0\xc1\xc2\xc3\xc4\xc5\xc6\xc7";
		CMBlock nonce;
		nonce.SetMemFixed(bytes2, sizeof(bytes2));

		Key key("S6c56bnXQiBjk9mqSYE7ykVQ7NzrRy");
		CMBlock byteData = key.encryptNative(data, nonce);
		REQUIRE(byteData.GetSize() >= 16);
		REQUIRE(byteData != false);

		CMBlock decode = key.decryptNative(byteData, nonce);
		REQUIRE(decode.GetSize() == data.GetSize());
		int r = memcmp(bytes1, decode, decode.GetSize());
		REQUIRE(r == 0);
	}

}

#ifdef PRESSURE_TEST

TEST_CASE("Key sign pressure test", "[KeySign]") {

	bool result = true;
	std::string message = "mymessage";
	for (int i = 0; i < 10000; ++i) {
		Key key("S6c56bnXQiBjk9mqSYE7ykVQ7NzrRy");

		std::string signedData = key.compactSign(message);
		if(!Key::verifyByPublicKey(Utils::encodeHex(key.getPubkey()), message, signedData)) {
			result = false;
			printf("index %d occured in error\n", i);
			REQUIRE(false);
		}
	}

	std::cout << result;
}

#endif