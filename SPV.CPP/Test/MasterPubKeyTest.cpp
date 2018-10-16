// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#define CATCH_CONFIG_MAIN

#include <BRBIP32Sequence.h>

#include "BRBIP39WordsEn.h"
#include "catch.hpp"
#include "MasterPubKey.h"

using namespace Elastos::ElaWallet;

TEST_CASE("MasterPubKey test", "[MasterPubKey]") {
	SECTION("Default constructor") {
		MasterPubKey masterPubKey;
		REQUIRE(masterPubKey.getRaw() != nullptr);
	}

	SECTION("Create width phrase") {
		std::string phrase = "a test seed ha";
		MasterPubKey masterPubKey(phrase);
		REQUIRE(masterPubKey.getRaw() != nullptr);
	}
}

TEST_CASE("MasterPubKey method test", "[MasterPubKey]") {
	SECTION("getPubKey method test") {
		std::string phrase = "a test seed ha";
		MasterPubKey masterPubKey(phrase);

		CMBlock pubkeyBytes = masterPubKey.getPubKey();
		REQUIRE(pubkeyBytes.GetSize() > 0);
	}

	SECTION("serialize method test") {
		std::string phrase = "a test seed ha";
		MasterPubKey masterPubKey(phrase);

		const CMBlock bytes = masterPubKey.serialize();
		REQUIRE(bytes.GetSize() > 0);

		MasterPubKey masterPubKey1;
		masterPubKey1.deserialize(bytes);
		BRMasterPubKey *key1 = masterPubKey.getRaw();
		BRMasterPubKey *key2 = masterPubKey1.getRaw();
		REQUIRE(key1->fingerPrint == key2->fingerPrint);
		int r = UInt256Eq(&key1->chainCode, &key1->chainCode);
		REQUIRE(r == 1);
		for (int i = 0; i < sizeof(key1->pubKey); i++) {
			REQUIRE(key1->pubKey[i] == key2->pubKey[i]);
		}
	}
	SECTION("getPubKeyAsKey method test") {
		std::string phrase = "a test seed ha";
		MasterPubKey masterPubKey(phrase);

		BRKey *brkey = new BRKey();
		brkey->compressed = 0;
		brkey->secret = UINT256_ZERO;
		boost::shared_ptr<Key> key = boost::shared_ptr<Key>(new Key(brkey));

		memset(key->getRaw()->pubKey, 0, sizeof(key->getRaw()->pubKey));
		char *pubkey1 = (char *) key->getRaw()->pubKey;
		key = masterPubKey.getPubKeyAsKey();
		REQUIRE(key.get() != nullptr);
		REQUIRE(key->getRaw()->compressed != 0);
		for (int i = 0; i < sizeof(key->getRaw()->secret); i++) {
			REQUIRE(key->getRaw()->secret.u8[i] == 0);
		}
		char *pubkey2 = (char *) key->getRaw()->pubKey;
		REQUIRE(std::string(pubkey1) != std::string(pubkey2));
	}
}
