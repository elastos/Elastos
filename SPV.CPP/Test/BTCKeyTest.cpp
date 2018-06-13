// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#define CATCH_CONFIG_MAIN

#include "catch.hpp"

#include "BTCKey.h"
#include "BigIntFormat.h"

using namespace Elastos::SDK;

TEST_CASE("generate key", "[BTCKey]") {
	CMemBlock<uint8_t> privKey, pubKey;
	if (true == BTCKey::generateKey(privKey, pubKey, NID_secp256k1)) {
		CMemBlock<char> c_privKey, c_pubKey;
		c_privKey = Hex2Str(privKey);
		c_pubKey = Hex2Str(pubKey);
		std::cout << "privKey=" << (const char *) c_privKey << ":" << "pubKey=" << (const char *) c_pubKey << std::endl;
	}
}

TEST_CASE("get public key from private key", "[BTCKey]") {
	CMemBlock<uint8_t> pubKey;
	uint8_t privateKey[] = {137, 130, 127, 138, 111, 69, 76, 178, 118, 250, 113, 184, 5, 173, 174, 142, 115, 153, 49,
						  170, 3, 12, 53, 42, 210, 47, 58, 180, 204, 87, 159, 54};
	CMemBlock<uint8_t> mb_privKey;
	mb_privKey.SetMemFixed(privateKey, sizeof(privateKey));

	pubKey = BTCKey::getPubKeyFromPrivKey(mb_privKey, NID_secp256k1);
	if (true == pubKey) {
		CMemBlock<char> _pubKey = Hex2Str(pubKey);
		if (_pubKey) {
			std::cout << "pubKey=" << (const char *)_pubKey << std::endl;
		}
	}
}

