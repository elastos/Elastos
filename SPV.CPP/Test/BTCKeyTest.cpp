// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#define CATCH_CONFIG_MAIN

#include "catch.hpp"
#include "BRBIP32Sequence.h"

#include "BTCKey.h"
#include "BigIntFormat.h"

using namespace Elastos::SDK;

TEST_CASE("generate key", "[BTCKey]") {
	CMemBlock<uint8_t> privKey, pubKey;
	if (true == BTCKey::generateKey(privKey, pubKey, NID_secp256k1)) {
		CMemBlock<char> cPrivkey, cPubkey;
		cPrivkey = Hex2Str(privKey);
		cPubkey = Hex2Str(pubKey);
		std::cout << "privKey=" << (const char *) cPrivkey << ":" << "pubkey=" << (const char *) cPubkey << std::endl;
	}
}

TEST_CASE("get public key from private key", "[BTCKey]") {
	CMemBlock<uint8_t> pubKey;
	uint8_t privateKey[] = {137, 130, 127, 138, 111, 69, 76, 178, 118, 250, 113, 184, 5, 173, 174, 142, 115, 153, 49,
						  170, 3, 12, 53, 42, 210, 47, 58, 180, 204, 87, 159, 54};
	CMemBlock<uint8_t> mbPrivKey;
	mbPrivKey.SetMemFixed(privateKey, sizeof(privateKey));

	pubKey = BTCKey::getPubKeyFromPrivKey(mbPrivKey, NID_secp256k1);
	if (true == pubKey) {
		CMemBlock<char> mbPubKey = Hex2Str(pubKey);
		if (mbPubKey) {
			std::cout << "pubKey=" << (const char *)mbPubKey << std::endl;
		}
	}
}

TEST_CASE("verify public key", "[BTCKey]") {
	CMemBlock<uint8_t> privKey, pubKey;
	if (true == BTCKey::generateKey(privKey, pubKey, NID_secp256k1)) {
		CMemBlock<char> cPrivkey, cPubkey;
		cPrivkey = Hex2Str(privKey);
		cPubkey = Hex2Str(pubKey);
		std::cout << "privKey=" << (const char *) cPrivkey << ":" << "pubKey=" << (const char *) cPubkey << std::endl;
	}

	bool verified_pubkey = BTCKey::PublickeyIsValid(pubKey, NID_secp256k1);
	REQUIRE(true == verified_pubkey);
}

TEST_CASE("BRWallet fetch addrs", "[BTCKey]") {
	CMemBlock<uint8_t> privKey, pubKey;
	if (true == BTCKey::generateKey(privKey, pubKey, NID_secp256k1)) {
		CMemBlock<char> cPrivkey, cPubkey;
		cPrivkey = Hex2Str(privKey);
		cPubkey = Hex2Str(pubKey);
		std::cout << "privKey=" << (const char *) cPrivkey << ":" << "pubKey=" << (const char *) cPubkey << std::endl;
	}
}
