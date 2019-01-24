// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#define CATCH_CONFIG_MAIN

#include <catch.hpp>

#include <SDK/Crypto/Key.h>
#include <SDK/Common/Log.h>
#include <SDK/Common/Utils.h>
#include <SDK/KeyStore/Mnemonic.h>
#include <SDK/BIPs/BIP32Sequence.h>

#include <Core/BRBIP39Mnemonic.h>
#include <boost/shared_ptr.hpp>
#include <boost/filesystem/path.hpp>

using namespace Elastos::ElaWallet;

TEST_CASE("Key test", "[Key]") {

	SECTION("Contructor with string and CMBlock") {

		std::string phrase = "闲 齿 兰 丹 请 毛 训 胁 浇 摄 县 诉";
		std::string phrasePasswd = "";
		//init master public key and private key
		UInt512 seed;
		BRBIP39DeriveKey(&seed, phrase.c_str(), phrasePasswd.c_str());

		//init id chain derived master public key
		UInt256 chainCode;

		Key key = BIP32Sequence::PrivKeyPath(&seed, sizeof(seed), chainCode, 5,
											 44 | BIP32_HARD, 0 | BIP32_HARD, 0 | BIP32_HARD, 0, 0);

		REQUIRE("Ed8ZSxSB98roeyuRZwwekrnRqcgnfiUDeQ" == key.GetAddress(PrefixStandard));
	}

}


TEST_CASE("Key sign pressure test", "[KeySign]") {

#ifdef PRESSURE_TEST
#define LOOP_COUNT 10000
#else
#define LOOP_COUNT 1
#endif

	std::string message = "mymessage";
	for (int i = 0; i < LOOP_COUNT; ++i) {
		Key key;
		key.SetPrivKey("S6c56bnXQiBjk9mqSYE7ykVQ7NzrRy");

		CMBlock signedData = key.Sign(message);
		REQUIRE(key.Verify(message, signedData));
	}
}

