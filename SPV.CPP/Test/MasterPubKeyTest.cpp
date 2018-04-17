// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#define CATCH_CONFIG_MAIN

#include <BRBIP32Sequence.h>

#include "BRBIP39WordsEn.h"
#include "catch.hpp"
#include "MasterPubKey.h"

using namespace Elastos::SDK;

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

		ByteData pubkeyBytes = masterPubKey.getPubKey();
		REQUIRE(pubkeyBytes.length > 0);
		REQUIRE(sizeof(pubkeyBytes.data) > 0);
	}

	SECTION("serialize method test") {
		std::string phrase = "a test seed ha";
		MasterPubKey masterPubKey(phrase);

		const ByteData bytes = masterPubKey.serialize();
		REQUIRE(bytes.length > 0);
		REQUIRE(sizeof(bytes.data) > 0);

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

		boost::shared_ptr<BRKey> brkey = boost::shared_ptr<BRKey>(new BRKey());
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

TEST_CASE("MasterPubKey static method test", "[MasterPubKey]") {
	SECTION("bip32BitIDKey method test") {
		UInt128 seed = *(UInt128 *) "\x00\x01\x02\x03\x04\x05\x06\x07\x08\x09\x0A\x0B\x0C\x0D\x0E\x0F";
		ByteData seedByte(seed.u8, sizeof(seed));

		ByteData byteData(nullptr, 0);
		byteData = MasterPubKey::bip32BitIDKey(seedByte, 0, "https://www.elastos.org");
		REQUIRE(byteData.data != nullptr);
		REQUIRE(byteData.length > 0);
	}

	SECTION("phrase validate method test") {
		std::string s = "bless bird birth blind blossom boil bonus entry equal error fence fetch";
		std::vector<std::string> words;
		for (std::string str : BRBIP39WordsEn) {
			words.push_back(str);
		}
		bool result = MasterPubKey::validateRecoveryPhrase(words, s);
		REQUIRE(result == false);

		UInt128 seed = *(UInt128 *) "\x00\x01\x02\x03\x04\x05\x06\x07\x08\x09\x0A\x0B\x0C\x0D\x0E\x0F";
		ByteData seedByte(seed.u8, sizeof(seed));

		ByteData paperKey(nullptr, 0);
		paperKey = MasterPubKey::generatePaperKey(seedByte, words);
		REQUIRE(paperKey.length > 0);
		const char *paperStr = (const char *) paperKey.data;
		REQUIRE(paperStr != nullptr);

		result = MasterPubKey::validateRecoveryPhrase(words, std::string(paperStr));
		REQUIRE(result == true);
	}
}
