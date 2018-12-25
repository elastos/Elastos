// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#define CATCH_CONFIG_MAIN

#include <catch.hpp>

#include <SDK/Crypto/Key.h>
#include <SDK/Common/Log.h>
#include <SDK/Common/Utils.h>
#include <SDK/KeyStore/Mnemonic.h>

#include <Core/BRBIP39Mnemonic.h>
#include <Core/BRBIP32Sequence.h>
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

		BRKey masterKey;
		BRBIP32APIAuthKey(&masterKey, &seed, sizeof(seed));

		//init id chain derived master public key
		BRKey idMasterKey;
		UInt256 idChainCode;
		BRBIP32PrivKeyPath(&idMasterKey, &idChainCode, &seed, sizeof(seed),
						   5, 44 | BIP32_HARD, 0 | BIP32_HARD, 0 | BIP32_HARD, 0, 0);
		Key key(idMasterKey.secret, idMasterKey.compressed);

		REQUIRE("Ed8ZSxSB98roeyuRZwwekrnRqcgnfiUDeQ" == key.address());
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