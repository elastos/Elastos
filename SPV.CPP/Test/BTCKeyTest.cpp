// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#define CATCH_CONFIG_MAIN

#include "catch.hpp"
#include "BRBIP32Sequence.h"

#include "BTCKey.h"
#include "BigIntFormat.h"
#include "Key.h"
#include "Utils.h"

using namespace Elastos::ElaWallet;

TEST_CASE("generate key", "[BTCKey]") {
	CMemBlock<uint8_t> privKey, pubKey;

	bool ret = BTCKey::generateKey(privKey, pubKey, NID_secp256k1);
	REQUIRE(ret == true);
	REQUIRE(privKey.GetSize() > 0);
	REQUIRE(pubKey.GetSize() > 0);

	privKey.Resize(0);
	pubKey.Resize(0);
	ret = BTCKey::generateKey(privKey, pubKey, NID_X9_62_prime256v1);
	REQUIRE(ret == true);
	REQUIRE(privKey.GetSize() > 0);
	REQUIRE(pubKey.GetSize() > 0);
}

TEST_CASE("get public key from private key", "[BTCKey]") {
	CMemBlock<uint8_t> pubKey;
	uint8_t privateKey[] = {137, 130, 127, 138, 111, 69, 76, 178, 118, 250, 113, 184, 5, 173, 174, 142, 115, 153, 49,
						  170, 3, 12, 53, 42, 210, 47, 58, 180, 204, 87, 159, 54};

	uint8_t secp256K1_pub[] = {3, 97, 154, 192, 99, 67, 211, 248, 155, 151, 167, 34, 209, 107, 244, 160, 27, 234, 111,
	                           157, 102, 12, 191, 226, 145, 196, 70, 230, 99, 45, 128, 180, 234};

	uint8_t golang_pub[] = {3, 238, 47, 108, 60, 240, 239, 93, 249, 29, 16, 178, 198, 27, 188, 188, 14, 172, 45, 14, 65,
	                        93, 34, 68, 127, 27, 210, 116, 98, 137, 246, 77, 210};

 	CMemBlock<uint8_t> mb_privKey;
	mb_privKey.SetMemFixed(privateKey, sizeof(privateKey));

	pubKey = BTCKey::getPubKeyFromPrivKey(mb_privKey, NID_secp256k1);
	int ret = memcmp(pubKey, secp256K1_pub, sizeof(secp256K1_pub));
	REQUIRE(ret == 0);

	pubKey = BTCKey::getPubKeyFromPrivKey(mb_privKey, NID_X9_62_prime256v1);
	ret = memcmp(pubKey, golang_pub, sizeof(golang_pub));
	REQUIRE(ret == 0);

}

TEST_CASE("verify public key with NID_secp256k1", "[BTCKey]") {
	CMemBlock<uint8_t> privKey, pubKey;
	bool ret = BTCKey::generateKey(privKey, pubKey, NID_secp256k1);
	REQUIRE(true == ret);

	bool verified_pubkey = BTCKey::PublickeyIsValid(pubKey, NID_secp256k1);
	REQUIRE(true == verified_pubkey);
}

TEST_CASE("verify public key with NID_X9_62_prime256v1", "[BTCKey]") {
	CMemBlock<uint8_t> privKey, pubKey;
	bool ret = BTCKey::generateKey(privKey, pubKey, NID_X9_62_prime256v1);
	REQUIRE(true == ret);

	bool verified_pubkey = BTCKey::PublickeyIsValid(pubKey, NID_X9_62_prime256v1);
	REQUIRE(true == verified_pubkey);
}

TEST_CASE("BTCKey signatrue and verify Test", "[BTCKey]") {
	uint8_t privateKey[] = {247,142,52,74,196,253,223,144,25,37,217,85,90,68,219,124,39,167,120,92,213,45,147,112,101,195,206,220,127,13,126,191};
	std::string message = "message to signature";
	CMemBlock<uint8_t> privKey;
	privKey.SetMemFixed(privateKey, sizeof(privateKey));

	CMemBlock<uint8_t> publicKey = BTCKey::getPubKeyFromPrivKey(privKey, NID_X9_62_prime256v1);
	CMBlock pubData(publicKey.GetSize());
	memcpy(pubData, publicKey, publicKey.GetSize());
	std::string pubKey = Utils::encodeHex(pubData);

	CMemBlock<uint8_t> shaData(sizeof(UInt256));
	BRSHA256(shaData, message.c_str(), message.size());

	CMBlock signature = BTCKey::SignCompact(privKey, shaData);
	REQUIRE(signature.GetSize() == 65);

	UInt256 shaMsg;
	memcpy(shaMsg.u8, shaData, shaData.GetSize());
	bool ret = BTCKey::VerifyCompact(pubKey, shaMsg, signature);
	REQUIRE(ret == true);
}
