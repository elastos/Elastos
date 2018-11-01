// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#define CATCH_CONFIG_MAIN

#include "catch.hpp"

#include "Utils.h"
#include "Account/StandardAccount.h"

using namespace Elastos::ElaWallet;

TEST_CASE("Derive id public and private key", "[Id agent]") {
	std::string phrasePassword = "phrasePassword";
	std::string payPassword = "payPassword";
	std::string chainId = "chainid";
	std::string mnemonic = "you train view salon cancel impulse phrase oxygen sport crack peasant observe";
	std::string language = "english";

	SECTION("Address derived from public and private key should be same") {
		StandardAccount account("Data", mnemonic, phrasePassword, payPassword);

		const MasterPubKey &publicKey = account.GetIDMasterPubKey();
		uint8_t tempKey[BRBIP32PubKey(NULL, 0, *publicKey.getRaw(), 1, 0)];
		size_t len = BRBIP32PubKey(tempKey, sizeof(tempKey), *publicKey.getRaw(), 1, 0);

		BRKey rawPubKey;
		BRKeySetPubKey(&rawPubKey, tempKey, len);
		Key pubKey(rawPubKey);
		std::string pubId = pubKey.keyToAddress(ELA_IDCHAIN);

		UInt512 seed = account.DeriveSeed(payPassword);
		BRKey rawPrivKey;
		UInt256 chainCode;
		BRBIP32PrivKeyPath(&rawPrivKey, &chainCode, &seed.u8[0], sizeof(seed), 3, 0 | BIP32_HARD, 1, 0);
		Key privKey(rawPrivKey);
		privKey.setPublicKey();
		std::string privId = privKey.keyToAddress(ELA_IDCHAIN);

		REQUIRE(pubId == privId);
	}
}

TEST_CASE("Derive public and private key", "[HD wallet]") {
	SECTION("Address") {
		int coinIndex = 0;
		std::string payPassword = "payPassword";
		std::string mnemonic = "flat universe quantum uniform emerge blame lemon detail april sting aerobic disease";
		for (size_t i = 0; i < 1; ++i) {
			StandardAccount account("Data", mnemonic, "", "payPassword");
			UInt512 seed = account.DeriveSeed(payPassword);
			Key key;
			UInt256 chainCode;
			BRBIP32PrivKeyPath(key.getRaw(), &chainCode, &seed, sizeof(seed), 5, 44 | BIP32_HARD,
							   coinIndex | BIP32_HARD, 0 | BIP32_HARD, 0, 0);

			key.setPublicKey();
			CMBlock pubkey = key.getPubkey();
			std::cout << "pubkey = " << Utils::encodeHex(pubkey) << std::endl;
			std::cout << "pubkey = " << account.GetPublicKey() << std::endl;
			var_clean(&chainCode);
			var_clean(&seed);
			REQUIRE("EJuHg2CdT9a9bqdKUAtbrAn6DGwXtKA6uh" == key.address());
		}
	}
}

