// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#define CATCH_CONFIG_MAIN

#include "catch.hpp"

#include "Account/StandardAccount.h"

using namespace Elastos::ElaWallet;

TEST_CASE("Derive id public and private key", "[Id agent]") {
	std::string phrasePassword = "phrasePassword";
	std::string payPassword = "payPassword";
	std::string chainId = "chainid";
	std::string mnemonic = "you train view salon cancel impulse phrase oxygen sport crack peasant observe";
	std::string language = "english";

	SECTION("Address derived from public and private key should be same") {
		StandardAccount account("Data", mnemonic, language, phrasePassword, payPassword);

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

