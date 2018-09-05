// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#define CATCH_CONFIG_MAIN

#include "catch.hpp"
#include "BRBIP32Sequence.h"
#include <openssl/ec.h>
#include <Core/BRKey.h>
#include <Core/BRBIP32Sequence.h>
#include <Core/BRCrypto.h>

#include "BigIntFormat.h"
#include "WalletTool.h"
#include "SDK/KeyStore/Mnemonic.h"

using namespace Elastos::ElaWallet;

//#define BTCKEY_DEBUG_MSG

TEST_CASE("generate key", "[BTCKey]") {
	CMBlock privKey, pubKey;
	if (true == BTCKey::generateKey(privKey, pubKey, NID_X9_62_prime256v1)) {
		CMemBlock<char> cPrivkey, cPubkey;
		cPrivkey = Hex2Str(privKey);
		cPubkey = Hex2Str(pubKey);
		REQUIRE(true == BTCKey::KeyIsValid(privKey, pubKey, NID_X9_62_prime256v1));
#ifdef BTCKEY_DEBUG_MSG
		std::cout << "privKey=" << (const char *) cPrivkey << ":" << "pubkey=" << (const char *) cPubkey << std::endl;
#endif
	}
}

TEST_CASE("get public key from private key", "[BTCKey]") {
	CMBlock pubKey;
	uint8_t privateKey[] = {137, 130, 127, 138, 111, 69, 76, 178, 118, 250, 113, 184, 5, 173, 174, 142, 115, 153, 49,
							170, 3, 12, 53, 42, 210, 47, 58, 180, 204, 87, 159, 54};
	CMBlock mbPrivKey;
	mbPrivKey.SetMemFixed(privateKey, sizeof(privateKey));

	pubKey = BTCKey::getPubKeyFromPrivKey(mbPrivKey, NID_secp256k1);
	if (true == pubKey) {
		REQUIRE(true == BTCKey::KeyIsValid(mbPrivKey, pubKey));
		CMemBlock<char> mbPubKey = Hex2Str(pubKey);
		if (mbPubKey) {
#ifdef BTCKEY_DEBUG_MSG
			std::cout << "pubKey=" << (const char *) mbPubKey << std::endl;
#endif
		}
	}
}

TEST_CASE("verify public key ", "[BTCKey]") {
	CMBlock privKey, pubKey;
	if (true == BTCKey::generateKey(privKey, pubKey, NID_secp256k1)) {
		REQUIRE(true == BTCKey::KeyIsValid(privKey, pubKey, NID_secp256k1));
		CMemBlock<char> cPrivkey, cPubkey;
		cPrivkey = Hex2Str(privKey);
		cPubkey = Hex2Str(pubKey);
#ifdef BTCKEY_DEBUG_MSG
		std::cout << "privKey=" << (const char *) cPrivkey << ":" << "pubKey=" << (const char *) cPubkey << std::endl;
#endif
		bool verified_pubkey = BTCKey::PublickeyIsValid(pubKey, NID_secp256k1);
		REQUIRE(true == verified_pubkey);
	}
}

TEST_CASE("secp256k1 sign/verify", "[BTCKey]") {
	CMBlock privKey, pubKey;
	if (true == BTCKey::generateKey(privKey, pubKey)) {
		if (BTCKey::KeyIsValid(privKey, pubKey)) {
			uint8_t data[] = {0, 1, 2, 3, 4, 5};
			uint8_t *pPrivKey = privKey;
			size_t szPrivKey = privKey.GetSize();
			assert(szPrivKey = sizeof(UInt256));
			BRKey key;
			if (1 == BRKeySetSecret(&key, (UInt256 *) pPrivKey, 1)) {
				size_t szPubKey = BRKeyPubKey(&key, nullptr, 0);
				CMBlock mbPubKey(szPubKey);
				szPubKey = BRKeyPubKey(&key, mbPubKey, szPubKey);
				uint8_t utPubKey[szPubKey];
				memcpy(utPubKey, mbPubKey, szPubKey);
				UInt256 md = UINT256_ZERO;
				BRSHA256(&md, data, sizeof(data));
				uint8_t signedData[256] = {0};
				size_t signedLen = BRKeySign(&key, signedData, sizeof(signedData), md);
				CMBlock mbSignedData(signedLen);
				signedLen = BRKeySign(&key, mbSignedData, signedLen, md);
				bool bVerify = BRKeyVerify(&key, md, mbSignedData, signedLen);
				REQUIRE(true == bVerify);
			}
		}
	}
}

TEST_CASE("Secp256k1Sign/Verify ECDSASign/ECDSAVerify", "[BTCKey]") {
	std::string mnemonic = "abandon abandon abandon abandon abandon abandon abandon abandon abandon abandon abandon about";
	CMBlock seed = BTCKey::getPrivKeySeed(mnemonic, "", "Data");
	CMemBlock<char> mbcSeed = Hex2Str(seed);

#ifdef BTCKEY_DEBUG_MSG
	printf("\r\nSeed:\r\n");
	printf("%s\n", (const char *) mbcSeed);
#endif

	uint32_t chain = SEQUENCE_EXTERNAL_CHAIN;
	UInt256 chainCode = UINT256_ZERO;
	CMBlock privKey = BTCKey::getMasterPrivkey(seed);
	CMBlock pubKey = BTCKey::getPubKeyFromPrivKey(privKey);

#ifdef BTCKEY_DEBUG_MSG
	printf("\n\nprivKey, len=%zu\n\n", privKey.GetSize());
	for (size_t i = 0; i < privKey.GetSize(); i++) {
		printf("%02X ", privKey[i]);
	}
	printf("\n\n");
	printf("\n\npubKey, len=%zu\n\n", pubKey.GetSize());
	for (size_t i = 0; i < pubKey.GetSize(); i++) {
		printf("%02X ", pubKey[i]);
	}
	printf("\n\n");
#endif

	uint8_t data[] = {0, 1, 2, 3, 4, 5};
	CMBlock mbData;
	mbData.SetMemFixed(data, sizeof(data));

	uint8_t secp256k1SignedData[256] = {0};
	size_t szSecp256k1SignedData = 0;
	BRKey key;
	if (1 == BRKeySetSecret(&key, (UInt256 *) (void *) privKey, 1)) {
		BRKeyPubKey(&key, nullptr, 0);
		UInt256 md = UINT256_ZERO;
		BRSHA256(&md, data, sizeof(data));
		uint8_t signedData[256] = {0};
		size_t signedLen = BRKeySign(&key, signedData, sizeof(signedData), md);
		szSecp256k1SignedData = signedLen;
		memcpy(secp256k1SignedData, signedData, signedLen);
		bool bVerify = BRKeyVerify(&key, md, signedData, signedLen);
		REQUIRE(true == bVerify);
	}

	uint8_t ECDSASignedData[256] = {0};
	size_t szECDSASignedData = 0;
	CMBlock mbSignedData;
	if (BTCKey::ECDSASign(privKey, mbData, mbSignedData)) {
		szECDSASignedData = mbSignedData.GetSize();
		memcpy(ECDSASignedData, mbSignedData, mbSignedData.GetSize());
		bool bVerify = BTCKey::ECDSAVerify(pubKey, mbData, mbSignedData);
		REQUIRE(true == bVerify);
	}

#ifdef BTCKEY_DEBUG_MSG
	printf("\n\nSecp256k1 Sign, len=%zu\n\n", szSecp256k1SignedData);
	for (size_t i = 0; i < szSecp256k1SignedData; i++) {
		printf("%02X ", secp256k1SignedData[i]);
	}
	printf("\n\n");

	printf("\n\nECDSA Sign, len=%zu\n\n", szECDSASignedData);
	for (size_t i = 0; i < szECDSASignedData; i++) {
		printf("%02X ", ECDSASignedData[i]);
	}
	printf("\n\n");
#endif
}

TEST_CASE("ECDSACompactSign/ECDSACompactVerify", "[BTCKey]") {
	std::string mnemonic = "abandon abandon abandon abandon abandon abandon abandon abandon abandon abandon abandon about";
	CMBlock seed = BTCKey::getPrivKeySeed(mnemonic, "", "Data");
	CMemBlock<char> mbcSeed = Hex2Str(seed);

#ifdef BTCKEY_DEBUG_MSG
	printf("\r\nSeed:\r\n");
	printf("%s\n", (const char *) mbcSeed);
#endif

	uint32_t chain = SEQUENCE_EXTERNAL_CHAIN;
	UInt256 chainCode = UINT256_ZERO;
	CMBlock privKey = BTCKey::getMasterPrivkey(seed);
	CMBlock pubKey = BTCKey::getPubKeyFromPrivKey(privKey);

#ifdef BTCKEY_DEBUG_MSG
	printf("\n\nprivKey, len=%zu\n\n", privKey.GetSize());
	for (size_t i = 0; i < privKey.GetSize(); i++) {
		printf("%02X ", privKey[i]);
	}
	printf("\n\n");
	printf("\n\npubKey, len=%zu\n\n", pubKey.GetSize());
	for (size_t i = 0; i < pubKey.GetSize(); i++) {
		printf("%02X ", pubKey[i]);
	}
	printf("\n\n");
#endif

	uint8_t data[] = {0, 1, 2, 3, 4, 5};
	CMBlock mbData;
	mbData.SetMemFixed(data, sizeof(data));

	uint8_t ECDSASignedData[256] = {0};
	size_t szECDSASignedData = 0;
	CMBlock mbSignedData;
	if (BTCKey::ECDSACompactSign(privKey, mbData, mbSignedData)) {
		szECDSASignedData = mbSignedData.GetSize();
		memcpy(ECDSASignedData, mbSignedData, mbSignedData.GetSize());
		bool bVerify = BTCKey::ECDSACompactVerify(pubKey, mbData, mbSignedData);
		REQUIRE(true == bVerify);
	}

#ifdef BTCKEY_DEBUG_MSG
	printf("\n\nECDSACompact Sign, len=%zu\n\n", szECDSASignedData);
	for (size_t i = 0; i < szECDSASignedData; i++) {
		printf("%02X ", ECDSASignedData[i]);
	}
	printf("\n\n");
#endif
}

TEST_CASE("2 BRBIP32PrivKey BRBIP32PubKey NID_secp256k1", "[BTCKey]") {
	std::string mnemonic = "abandon abandon abandon abandon abandon abandon abandon abandon abandon abandon abandon about";
	CMBlock seed = BTCKey::getPrivKeySeed(mnemonic, "", "Data");
	CMemBlock<char> mbcSeed = Hex2Str(seed);

#ifdef BTCKEY_DEBUG_MSG
	printf("\r\nSeed:\r\n");
	printf("%s\n", (const char *) mbcSeed);
#endif

	uint32_t chain = SEQUENCE_EXTERNAL_CHAIN;
	UInt256 chainCode = UINT256_ZERO;
	CMBlock mbPrivkey = BTCKey::getMasterPrivkey(seed);
	CMBlock mbPubkey = BTCKey::getPubKeyFromPrivKey(mbPrivkey);


	std::vector<CMBlock> vecHDPrivkey, vecHDPubkey;

#ifdef BTCKEY_DEBUG_MSG
	printf("\n\n***************************\n\n");
	printf("NID_secp256k1 generate privKey\n");
#endif
	if (true == mbPrivkey) {
#ifdef BTCKEY_DEBUG_MSG
		printf("\nprivKey:\n");
		for (size_t i = 0; i < mbPrivkey.GetSize(); i++) {
			printf("%02X ", mbPrivkey[i]);
		}
		printf("\n\n");
#endif

		for (uint32_t count = 0; count < 10; count++) {
			CMBlock mbuPrivkey = BTCKey::getDerivePrivKey(seed, chain, count, chainCode);
			vecHDPrivkey.push_back(mbuPrivkey);

#ifdef BTCKEY_DEBUG_MSG
			printf("generate hd privkey%d, len=%zu:\n", count, mbuPrivkey.GetSize());
			for (size_t i = 0; i < mbuPrivkey.GetSize(); i++) {
				printf("%02X ", mbuPrivkey[i]);
			}
			printf("\n");
#endif
		}
	}

#ifdef BTCKEY_DEBUG_MSG
	printf("\n\n***************************\n\n");
	printf("NID_secp256k1 generate pubKey\n");
#endif
	if (true == mbPubkey && true == BTCKey::PublickeyIsValid(mbPubkey)) {
#ifdef BTCKEY_DEBUG_MSG
		printf("\npubKey:\n");
		for (size_t i = 0; i < mbPubkey.GetSize(); i++) {
			printf("%02X ", mbPubkey[i]);
		}
		printf("\n\n");
#endif

		for (uint32_t count = 0; count < 10; count++) {
			CMBlock mbuPubKey = BTCKey::getDerivePubKey(mbPubkey, chain, count, chainCode);
			vecHDPubkey.push_back(mbuPubKey);

#ifdef BTCKEY_DEBUG_MSG
			printf("generate hd pubkey%d, len=%zu:\n", count, mbuPubKey.GetSize());
			for (size_t i = 0; i < mbuPubKey.GetSize(); i++) {
				printf("%02X ", mbuPubKey[i]);
			}
			printf("\n");
#endif
		}
	}

	REQUIRE(true == BTCKey::KeyIsValid(mbPrivkey, mbPubkey));

	for (size_t i = 0; i < 10; i++) {
		bool b = BTCKey::KeyIsValid(vecHDPrivkey[i], vecHDPubkey[i]);
		REQUIRE(true == b);
	}
}

TEST_CASE("3 BRBIP32PrivKey BRBIP32PubKey NID_secp256k1", "[BTCKey]") {
	CMBlock mbPrivkey, mbPubkey;
	if (false == BTCKey::generateKey(mbPrivkey, mbPubkey)) {
		return;
	}
	if (false == BTCKey::PublickeyIsValid(mbPubkey)) {
		return;
	}

	uint32_t chain = SEQUENCE_EXTERNAL_CHAIN;
	UInt256 chainCode = UINT256_ZERO;

	std::vector<CMBlock> vecHDPrivkey, vecHDPubkey;

#ifdef BTCKEY_DEBUG_MSG
	printf("\n\n***************************\n\n");
	printf("NID_secp256k1 generate privKey\n");
#endif
	if (true == mbPrivkey) {
#ifdef BTCKEY_DEBUG_MSG
		printf("\nprivKey:\n");
		for (size_t i = 0; i < mbPrivkey.GetSize(); i++) {
			printf("%02X ", mbPrivkey[i]);
		}
		printf("\n\n");
#endif

		std::vector<CMBlock> privKeys(10);
		const uint32_t indexes[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
		BTCKey::getDerivePrivKey_Secret(privKeys, mbPrivkey, chain, indexes, chainCode);
		for (uint32_t count = 0; count < 10; count++) {
			vecHDPrivkey.push_back(privKeys[count]);

#ifdef BTCKEY_DEBUG_MSG
			printf("generate hd privkey%d, len=%zu:\n", count, privKeys[count].GetSize());
			for (size_t i = 0; i < privKeys[count].GetSize(); i++) {
				printf("%02X ", privKeys[count][i]);
			}
			printf("\n");
#endif
		}
	}

#ifdef BTCKEY_DEBUG_MSG
	printf("\n\n***************************\n\n");
	printf("NID_secp256k1 generate pubKey\n");
#endif
	if (true == mbPubkey && true == BTCKey::PublickeyIsValid(mbPubkey)) {
#ifdef BTCKEY_DEBUG_MSG
		printf("\npubKey:\n");
		for (size_t i = 0; i < mbPubkey.GetSize(); i++) {
			printf("%02X ", mbPubkey[i]);
		}
		printf("\n\n");
#endif

		for (uint32_t count = 0; count < 10; count++) {
			CMBlock mbHDPubKey = BTCKey::getDerivePubKey(mbPubkey, chain, count, chainCode);
			vecHDPubkey.push_back(mbHDPubKey);

#ifdef BTCKEY_DEBUG_MSG
			printf("generate hd pubkey%d, len=%zu:\n", count, mbHDPubKey.GetSize());
			for (size_t i = 0; i < mbHDPubKey.GetSize(); i++) {
				printf("%02X ", mbHDPubKey[i]);
			}
			printf("\n");
#endif
		}
	}

	REQUIRE(true == BTCKey::KeyIsValid(mbPrivkey, mbPubkey));

	for (size_t i = 0; i < 10; i++) {
		REQUIRE(true == BTCKey::KeyIsValid(vecHDPrivkey[i], vecHDPubkey[i]));
	}
}

TEST_CASE("4 BRBIP32PrivKey BRBIP32PubKey NID_X9_62_prime256v1", "[BTCKey]") {
	std::string mnemonic = "abandon abandon abandon abandon abandon abandon abandon abandon abandon abandon abandon about";
	CMBlock seed = BTCKey::getPrivKeySeed(mnemonic, "", "Data");
	CMemBlock<char> mbcSeed = Hex2Str(seed);

#ifdef BTCKEY_DEBUG_MSG
	printf("\nSeed:\n");
	printf("%s\n", (const char *) mbcSeed);
#endif

	CMBlock mbPrivkey = BTCKey::getMasterPrivkey(seed, NID_X9_62_prime256v1);
	CMBlock mbPubkey = BTCKey::getPubKeyFromPrivKey(mbPrivkey, NID_X9_62_prime256v1);

	uint32_t chain = SEQUENCE_EXTERNAL_CHAIN;
	UInt256 chainCode = UINT256_ZERO;

	std::vector<CMBlock> vecHDPrivkey, vecHDPubkey;

#ifdef BTCKEY_DEBUG_MSG
	printf("\n\n***************************\n\n");
	printf("NID_X9_62_prime256v1 generate privKey\n");
#endif
	if (true == mbPrivkey) {
#ifdef BTCKEY_DEBUG_MSG
		printf("\nprivKey:\n");
		for (size_t i = 0; i < mbPrivkey.GetSize(); i++) {
			printf("%02X ", mbPrivkey[i]);
		}
		printf("\n\n");
#endif

		for (uint32_t count = 0; count < 10; count++) {
			CMBlock mbHDPrivkey = BTCKey::getDerivePrivKey_Secret(mbPrivkey, chain, count, chainCode,
																  NID_X9_62_prime256v1);
			vecHDPrivkey.push_back(mbHDPrivkey);

#ifdef BTCKEY_DEBUG_MSG
			printf("generate hd privkey%d, len=%zu:\n", count, mbHDPrivkey.GetSize());
			for (size_t i = 0; i < mbHDPrivkey.GetSize(); i++) {
				printf("%02X ", mbHDPrivkey[i]);
			}
			printf("\n");
#endif
		}
	}

#ifdef BTCKEY_DEBUG_MSG
	printf("\n\n***************************\n\n");
	printf("NID_X9_62_prime256v1 generate pubKey\n");
#endif
	if (true == mbPubkey && true == BTCKey::PublickeyIsValid(mbPubkey, NID_X9_62_prime256v1)) {
#ifdef BTCKEY_DEBUG_MSG
		printf("\npubKey:\n");
		for (size_t i = 0; i < mbPubkey.GetSize(); i++) {
			printf("%02X ", mbPubkey[i]);
		}
		printf("\n\n");
#endif

		for (uint32_t count = 0; count < 10; count++) {
			CMBlock mbHDPubKey = BTCKey::getDerivePubKey(mbPubkey, chain, count, chainCode,
														 NID_X9_62_prime256v1);
			vecHDPubkey.push_back(mbHDPubKey);

#ifdef BTCKEY_DEBUG_MSG
			printf("generate hd pubkey%d, len=%zu:\n", count, mbHDPubKey.GetSize());
			for (size_t i = 0; i < mbHDPubKey.GetSize(); i++) {
				printf("%02X ", mbHDPubKey[i]);
			}
			printf("\n");
#endif
		}
	}

	REQUIRE(true == BTCKey::KeyIsValid(mbPrivkey, mbPubkey, NID_X9_62_prime256v1));

	for (size_t i = 0; i < 10; i++) {
		REQUIRE(true == BTCKey::KeyIsValid(vecHDPrivkey[i], vecHDPubkey[i], NID_X9_62_prime256v1));
	}
}

TEST_CASE("5 BRBIP32PrivKey BRBIP32PubKey NID_secp256k1", "[BTCKey]") {
	std::string mnemonic = "abandon abandon abandon abandon abandon abandon abandon abandon abandon abandon abandon about";
	CMBlock seed = BTCKey::getPrivKeySeed(mnemonic, "", "Data");
	CMemBlock<char> mbcSeed = Hex2Str(seed);

#ifdef BTCKEY_DEBUG_MSG
	printf("\r\nSeed:\r\n");
	printf("%s\n", (const char *) mbcSeed);
#endif

	uint32_t chain = SEQUENCE_EXTERNAL_CHAIN;
	UInt256 chainCode = UINT256_ZERO;
	CMBlock mbPrivkey = BTCKey::getMasterPrivkey(seed);
	CMBlock mbPubkey = BTCKey::getPubKeyFromPrivKey(mbPrivkey);


	std::vector<CMBlock> vecHDPrivkey, vecHDPubkey;

#ifdef BTCKEY_DEBUG_MSG
	printf("\n\n***************************\n\n");
	printf("NID_secp256k1 generate privKey\n");
#endif
	if (true == mbPrivkey) {
#ifdef BTCKEY_DEBUG_MSG
		printf("\nprivKey:\n");
		for (size_t i = 0; i < mbPrivkey.GetSize(); i++) {
			printf("%02X ", mbPrivkey[i]);
		}
		printf("\n\n");
#endif

		for (uint32_t count = 0; count < 10; count++) {
			CMBlock mbuPrivkey = BTCKey::getDerivePrivKey_depth(seed, chainCode, false, NID_secp256k1, 4,
																1 | BIP32_HARD, chain, chain, count);
			vecHDPrivkey.push_back(mbuPrivkey);

#ifdef BTCKEY_DEBUG_MSG
			printf("generate hd privkey%d, len=%zu:\n", count, mbuPrivkey.GetSize());
			for (size_t i = 0; i < mbuPrivkey.GetSize(); i++) {
				printf("%02X ", mbuPrivkey[i]);
			}
			printf("\n");
#endif
		}
	}

#ifdef BTCKEY_DEBUG_MSG
	printf("\n\n***************************\n\n");
	printf("NID_secp256k1 generate pubKey\n");
#endif
	if (true == mbPubkey && true == BTCKey::PublickeyIsValid(mbPubkey)) {
#ifdef BTCKEY_DEBUG_MSG
		printf("\npubKey:\n");
		for (size_t i = 0; i < mbPubkey.GetSize(); i++) {
			printf("%02X ", mbPubkey[i]);
		}
		printf("\n\n");
#endif

		for (uint32_t count = 0; count < 10; count++) {
			CMBlock mbuPubKey = BTCKey::getDerivePubKey(mbPubkey, chain, count, chainCode);
			vecHDPubkey.push_back(mbuPubKey);

#ifdef BTCKEY_DEBUG_MSG
			printf("generate hd pubkey%d, len=%zu:\n", count, mbuPubKey.GetSize());
			for (size_t i = 0; i < mbuPubKey.GetSize(); i++) {
				printf("%02X ", mbuPubKey[i]);
			}
			printf("\n");
#endif
		}
	}

	REQUIRE(true == BTCKey::KeyIsValid(mbPrivkey, mbPubkey));

	for (size_t i = 0; i < 10; i++) {
		bool b = BTCKey::KeyIsValid(vecHDPrivkey[i], vecHDPubkey[i]);
		REQUIRE(true == b);
	}
}

TEST_CASE("6 BRBIP32PrivKey BRBIP32PubKey NID_X9_62_prime256v1", "[BTCKey]") {
	CMBlock mbPrivkey, mbPubkey;
	if (false == BTCKey::generateKey(mbPrivkey, mbPubkey, NID_X9_62_prime256v1)) {
		return;
	}
	if (false == BTCKey::PublickeyIsValid(mbPubkey, NID_X9_62_prime256v1)) {
		return;
	}

	uint32_t chain = SEQUENCE_EXTERNAL_CHAIN;
	UInt256 chainCode = UINT256_ZERO;

	std::vector<CMBlock> vecHDPrivkey, vecHDPubkey;

#ifdef BTCKEY_DEBUG_MSG
	printf("\n\n***************************\n\n");
	printf("NID_secp256k1 generate privKey\n");
#endif
	if (true == mbPrivkey) {
#ifdef BTCKEY_DEBUG_MSG
		printf("\nprivKey:\n");
		for (size_t i = 0; i < mbPrivkey.GetSize(); i++) {
			printf("%02X ", mbPrivkey[i]);
		}
		printf("\n\n");
#endif

		for (uint32_t count = 0; count < 10; count++) {
			CMBlock mbChildPrivkey = BTCKey::getDerivePrivKey_Secret_depth(mbPrivkey, chainCode, false,
																		   NID_X9_62_prime256v1, 4, 1 | BIP32_HARD,
																		   chain, chain, count);
			vecHDPrivkey.push_back(mbChildPrivkey);

#ifdef BTCKEY_DEBUG_MSG
			//			printf("generate hd privkey%d, len=%zu:\n", count, privKeys[count].GetSize());
			//			for (size_t i = 0; i < privKeys[count].GetSize(); i++) {
			//				printf("%02X ", privKeys[count][i]);
			//			}
			//			printf("\n");
#endif
		}
	}

#ifdef BTCKEY_DEBUG_MSG
	printf("\n\n***************************\n\n");
	printf("NID_secp256k1 generate pubKey\n");
#endif
	if (true == mbPubkey && true == BTCKey::PublickeyIsValid(mbPubkey, NID_X9_62_prime256v1)) {
#ifdef BTCKEY_DEBUG_MSG
		printf("\npubKey:\n");
		for (size_t i = 0; i < mbPubkey.GetSize(); i++) {
			printf("%02X ", mbPubkey[i]);
		}
		printf("\n\n");
#endif

		for (uint32_t count = 0; count < 10; count++) {
			CMBlock mbHDPubKey = BTCKey::getDerivePubKey(mbPubkey, chain, count, chainCode, NID_X9_62_prime256v1);
			vecHDPubkey.push_back(mbHDPubKey);

#ifdef BTCKEY_DEBUG_MSG
			printf("generate hd pubkey%d, len=%zu:\n", count, mbHDPubKey.GetSize());
			for (size_t i = 0; i < mbHDPubKey.GetSize(); i++) {
				printf("%02X ", mbHDPubKey[i]);
			}
			printf("\n");
#endif
		}
	}

	REQUIRE(true == BTCKey::KeyIsValid(mbPrivkey, mbPubkey, NID_X9_62_prime256v1));

	for (size_t i = 0; i < 10; i++) {
		REQUIRE(true == BTCKey::KeyIsValid(vecHDPrivkey[i], vecHDPubkey[i], NID_X9_62_prime256v1));
	}
}

/**********************************************************************************************************************/
// Special for NID_X9_62_prime256v1 below
/**********************************************************************************************************************/

TEST_CASE("ECDSACompactSign_sha256/ECDSACompactVerify_sha256 for ECDSA NID_X9_62_prime256v1", "[BTCKey]") {
	// Debug with golang
	static int PRINT_SIGN_RESULT = 0;
	static int PRINT_VISUAL_RESULT = 0;
	// Debug with golang

	static int ECDSA_NID = NID_X9_62_prime256v1;

	std::string mnemonic = "abandon abandon abandon abandon abandon abandon abandon abandon abandon abandon abandon about";
	CMBlock seed = BTCKey::getPrivKeySeed(mnemonic, "", "Data", "english", ECDSA_NID);

	CMBlock privKey = BTCKey::getMasterPrivkey(seed, ECDSA_NID);
	CMBlock pubKey = BTCKey::getPubKeyFromPrivKey(privKey, ECDSA_NID);

	uint8_t data[] = {0, 1, 2, 3, 4, 5};
	UInt256 md = UINT256_ZERO;
	BRSHA256(&md, data, sizeof(data));

	CMBlock mbSignedData;
	if (BTCKey::ECDSACompactSign_sha256(privKey, md, mbSignedData, ECDSA_NID)) {
		bool bVerify = BTCKey::ECDSACompactVerify_sha256(pubKey, md, mbSignedData, ECDSA_NID);
		REQUIRE(true == bVerify);

		// Debug with golang
		if (1 == PRINT_SIGN_RESULT) {
			printf("\nseed, len=%zu\n", seed.GetSize());
			for (size_t i = 0; i < seed.GetSize(); i++) {
				if (1 == PRINT_VISUAL_RESULT) {
					printf("%02X ", seed[i]);
				} else {
					printf("%02X", seed[i]);
				}
			}
			printf("\n\n");
			printf("\nprivKey, len=%zu\n", privKey.GetSize());
			for (size_t i = 0; i < privKey.GetSize(); i++) {
				if (1 == PRINT_VISUAL_RESULT) {
					printf("%02X ", privKey[i]);
				} else {
					printf("%02X", privKey[i]);
				}
			}
			printf("\n\n");
			printf("\npubKey, len=%zu\n", pubKey.GetSize());
			for (size_t i = 0; i < pubKey.GetSize(); i++) {
				if (1 == PRINT_VISUAL_RESULT) {
					printf("%02X ", pubKey[i]);
				} else {
					printf("%02X", pubKey[i]);
				}
			}
			printf("\n\n");
			printf("\nSignature result, len=%zu\n", mbSignedData.GetSize());
			for (size_t i = 0; i < mbSignedData.GetSize(); i++) {
				if (1 == PRINT_VISUAL_RESULT) {
					printf("%02X ", mbSignedData[i]);
				} else {
					printf("%02X", mbSignedData[i]);
				}
			}
			printf("\n\n");
		}
		// Debug with golang
	}
}

// It is very importent test demo for demostrating ECDSA NID_X9_62_prime256v1's use from mnemonic, seed, masterprivatekey
// masterpublickey to derive child privatekey, derive child publickkey and signature/verify
TEST_CASE("Derive PrivateKey and PublicKey for ECDSA NID_X9_62_prime256v1", "[BTCKey]") {
	static int PRINT_SIGN_RESULT = 1;
	static int ECDSA_NID = NID_X9_62_prime256v1;

	std::string mnemonic = "abandon abandon abandon abandon abandon abandon abandon abandon abandon abandon abandon about";
	CMBlock seed = BTCKey::getPrivKeySeed(mnemonic, "", "Data", "english", ECDSA_NID);

	CMBlock mbMasterPrivKey = BTCKey::getMasterPrivkey(seed, ECDSA_NID);
	CMBlock mbMastePubKey = BTCKey::getPubKeyFromPrivKey(mbMasterPrivKey, ECDSA_NID);

	uint32_t chain = SEQUENCE_EXTERNAL_CHAIN;
	UInt256 chainCode = UINT256_ZERO;

	for (uint32_t index = 0; index < 10; index++) {
		CMBlock mbChildPrivkey = BTCKey::getDerivePrivKey_Secret(mbMasterPrivKey, chain, index, chainCode,
																 ECDSA_NID);
		CMBlock mbChildPubKey = BTCKey::BTCKey::getDerivePubKey(mbMastePubKey, chain, index, chainCode,
																ECDSA_NID);
		REQUIRE(true == BTCKey::KeyIsValid(mbChildPrivkey, mbChildPubKey, ECDSA_NID));

		uint8_t data[] = {0, 1, 2, 3, 4, 5};
		UInt256 md = UINT256_ZERO;
		BRSHA256(&md, data, sizeof(data));

		CMBlock mbSignedData;
		if (BTCKey::ECDSACompactSign_sha256(mbChildPrivkey, md, mbSignedData, ECDSA_NID)) {
			bool bVerify = BTCKey::ECDSACompactVerify_sha256(mbChildPubKey, md, mbSignedData, ECDSA_NID);
			if (!bVerify && 1 == PRINT_SIGN_RESULT) {
				printf("child index %d occured in error\n", index);
			}
			REQUIRE(true == bVerify);
		}
	}
}

TEST_CASE("ECDSA65Sign_sha256/ECDSA65Verify_sha256 for ECDSA NID_X9_62_prime256v1", "[BTCKey]") {
	static int ECDSA_NID = NID_X9_62_prime256v1;

	std::string mnemonic = "abandon abandon abandon abandon abandon abandon abandon abandon abandon abandon abandon about";
	CMBlock seed = BTCKey::getPrivKeySeed(mnemonic, "", "Data", "english", ECDSA_NID);

	CMBlock privKey = BTCKey::getMasterPrivkey(seed, ECDSA_NID);
	CMBlock pubKey = BTCKey::getPubKeyFromPrivKey(privKey, ECDSA_NID);

	uint8_t data[] = {0, 1, 2, 3, 4, 5};
	UInt256 md = UINT256_ZERO;
	BRSHA256(&md, data, sizeof(data));

	CMBlock mbSignedData;
	if (BTCKey::ECDSA65Sign_sha256(privKey, md, mbSignedData, ECDSA_NID)) {
		bool bVerify = BTCKey::ECDSA65Verify_sha256(pubKey, md, mbSignedData, ECDSA_NID);
		REQUIRE(true == bVerify);
	}
}