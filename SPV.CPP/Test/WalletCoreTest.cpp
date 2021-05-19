// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#define CATCH_CONFIG_MAIN

#include <catch.hpp>
#include <Common/typedefs.h>
#include <Common/Log.h>

#include <WalletCore/AES.h>
#include <WalletCore/Base58.h>
#include <WalletCore/HDKeychain.h>
#include <WalletCore/Key.h>

#include <nlohmann/json.hpp>

using namespace Elastos::ElaWallet;

TEST_CASE("WalletCore test", "[WalletCore]") {
	Log::registerMultiLogger();
	SECTION("Sign message with keystore") {
		std::string msg = "hello world";
		nlohmann::json encryptedKeystore = nlohmann::json::parse("{\"iv\":\"kgn/MQV6TYL0HPMqm51FqA==\",\"v\":1,\"iter\":10000,\"ks\":128,\"ts\":64,\"mode\":\"ccm\",\"adata\":\"\",\"cipher\":\"aes\",\"salt\":\"4+aks85XBtI=\",\"ct\":\"iDuHXNSZMOjeTm6nZDYKPYrTObuILSmRSL9niqwHjfhx7fFXALVm2feOG04qj2HbfFMYTdrfPRmqmMUYbbx9O8Ti+5XRgI8uRTAbrbz0mvpalDcZrzVy6QWsVGGACpt3nllQhYlZ5vTyaKM0hhrYk/+xAEDc7V7SVStnVu3DJxF07VWYh4sQxv8g7y3UM2Jw1jlnLZCp2Y1guUUsih6rRnzGCNbM7bvtxDVyEgBrVDL/8sWGhIy/P0va3d7N+4LDTO2smQ5Rx+AumRTwa1oAxcy4H+kACP5TzHtJqAc2hB2w+fQ70Jf+XKET5CGaNTWCzi+hjosgEl6blD1bWf0oFMu1sl/27OGY2kgssvkxELuQHQcQU5aiy3jFzkaVLh/sM4I3rTeqxamQNTU/440Pv6SmRxOg6O6udlfanft8QgRfSFkzBi1bTgE2XOFh00qgumPks3Lz5lSLXSGpPxiN647XiWgUxCG60BSTd/c2KAx/0VdF3Q99v9tR3Z7lT2nLhoOj9VdXveVTspNFCxWbe4HoERvT3x4Fv0P+ie9QiqeJn3CEGgi6Vv34ZL0yOyisS5v8M73tsmY2QWBCw+eZ/M1RFEVitF+wnB/6c7LaEfNJjJXgi485qp9epF+rKpV6OyWacVbSImHRGY+TtLcn7O/RP1Uy4sVevQ2q5xin7NkAs8HjtpBP7kfbcKEZNwYvRTgeaqIzuA2o13mUlYceLN9NZdKuHTKYrwZ1nCROMBhifleGVhc55VRLcUgbhCNUDUPsjJRlFW2F4flY/GSGuOrLRGJvdt/ga4S68qiF8Ep82aC9x8ZXVkAVfoXmrX3VRTw7aVT8f8qD6nMONpsXxSgOGyx6U6wFrZcJdO+Zzw3e4J9axEx8+h+qb7cfsdjE+y6fKwA1Ke8SMmwiHXYDbDsX+vvZvFU4S97AweqlP4BQvw1hhAbYTbi75mEGMgDTE6oaQVRbbyy90jOBlNvHRVcR/BbBjlXs6VUiWlamy6FuQcONFGFxZ2cBVd1jJ+V9CiQcmqRRM3NC8H/icFzHhKOP8yy+FKeFbwSLq1p3sUuTEaQnynYNkCeR768zZMGxtc2ffqKtETsjjZuyNseKtUG7xRmv+X12x0E5NktYnPeBXMf/SHIMAed3RtjKxQuEjMcXmgfAFH70E1W1U9Q2T0sG4Qi6eEKFkJFZsh69KxMy8UpWr+yP+aqYCU+46oPz3OysNtCM27cQC+RylsuqhUkpkoVS1HZwPvIl6v4pnf7maCH1VfcrwCHa3KRymPLFaztUIF4OM0Z6WpSLBg2n1x/2YLpp9aBYrCubwvhhAREu10ZU6rRrdBHpQj8760//Z0UCVy+/uuUAClxG2oDC682bhSKv3KhfJMNGmcI5YnRTUdRJ6k3kGLvSNYQTnuhn1s8TSfWjqSneMJtxaQCOs9mNTtZH3JMwVdceVeCrxWL6WRhRaK6GrNsKKw3mgPmE9s7FWKOOgMY6SJW2GnkfY7ucdlxaGDOCubWM2hx/+y3EBHF/oYdfflAAAZ3ery35oQmRwXrLrvye7JwCQCFQph7mdouoZ3lxvhbkF87dh2Xf1jt2Ros9shAAR+/RoSJMJRLz3jggorfbWmiHjVv2OAxGRIRDCGo4dRdE0se0AFX9Aw2v9ga3wQR/jCMIQGUh3HUKgREosPaPWkDBXsKgQUf+uRyUgasgWVUllYnIpiPBjV9qj2h2lsztImupNAKm/ha3p9rzDNtKUZjdhyrTUtNpcbN/+VZfhBgrr5oYzl5m5JFB7JAzrgaVnA==\"}");
		std::string passwd = "s12345678";

		// Decrypt
		std::string iv   = encryptedKeystore["iv"];
		std::string ct   = encryptedKeystore["ct"];
		std::string salt = encryptedKeystore["salt"];
		std::string mode = encryptedKeystore["mode"];
		std::string aad  = encryptedKeystore["adata"];
		int ks = encryptedKeystore["ks"];

		bytes_t plain;
		REQUIRE_NOTHROW(plain = AES::DecryptCCM(ct, passwd, salt, iv, aad, ks));

		nlohmann::json plaintext = nlohmann::json::parse(std::string((char *)plain.data(), plain.size()));

		// Get root private key
		if (plaintext.find("xPrivKey") == plaintext.end() || plaintext["xPrivKey"] == "") {
			Log::error("Unsupport keystore");
		}

		std::string xPrivKey = plaintext["xPrivKey"];

		bytes_t extkey;
		REQUIRE(Base58::CheckDecode(xPrivKey, extkey));

		HDKeychain rootKey(extkey);

		// Sign
#if 1
		HDKeychain requestKey = rootKey.getChild("1'/0");
		Key key = requestKey;
#else
//		Key key;
//		bytes_t prvKey = rootKey.getChild("1'/0").privkey();
//		key.SetPrvKey(prvKey);
#endif
		bytes_t signature = key.Sign(msg);

		// Verify
		Key verifyKey;
		verifyKey.SetPubKey(requestKey.pubkey());

		REQUIRE(verifyKey.Verify(msg, signature));
	}
}
