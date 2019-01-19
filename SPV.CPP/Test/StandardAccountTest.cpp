// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#define CATCH_CONFIG_MAIN

#include "catch.hpp"

#include <SDK/Common/Utils.h>
#include <Core/BRAddress.h>
#include <SDK/Account/StandardAccount.h>

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
		CMBlock pubKey(65);
		size_t len = BRBIP32PubKey(pubKey, pubKey.GetSize(), *publicKey.getRaw(), 1, 0);
		pubKey.Resize(len);

		Key key;
		key.SetPublicKey(pubKey);
		std::string pubId = key.keyToAddress(ELA_IDCHAIN);

		UInt512 seed = account.DeriveSeed(payPassword);
		BRKey rawPrivKey;
		UInt256 chainCode;
		BRBIP32PrivKeyPath(&rawPrivKey, &chainCode, &seed.u8[0], sizeof(seed), 3, 0 | BIP32_HARD, 1, 0);
		Key privKey(rawPrivKey);
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
			UInt256 chainCode;
			BRKey brKey;
			BRBIP32PrivKeyPath(&brKey, &chainCode, &seed, sizeof(seed), 5, 44 | BIP32_HARD,
							   coinIndex | BIP32_HARD, 0 | BIP32_HARD, 0, 0);

			Key key(brKey);
			CMBlock pubkey = key.GetPublicKey();
			var_clean(&chainCode);
			var_clean(&seed);
			REQUIRE("EJuHg2CdT9a9bqdKUAtbrAn6DGwXtKA6uh" == key.address());
		}
	}
}
