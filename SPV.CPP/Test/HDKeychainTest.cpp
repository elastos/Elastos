// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#define CATCH_CONFIG_MAIN

#include <catch.hpp>
#include "TestHelper.h"

#include <Common/Log.h>
#include <Common/Utils.h>
#include <Common/Lockable.h>
#include <WalletCore/AES.h>
#include <WalletCore/BIP39.h>
#include <WalletCore/Mnemonic.h>
#include <WalletCore/HDKeychain.h>
#include <WalletCore/Base58.h>
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/classification.hpp>

using namespace Elastos::ElaWallet;

TEST_CASE("HDKeychain test", "[HDKeychain]") {
	Log::registerMultiLogger();

	SECTION("Web standard HD keystore") {
		nlohmann::json keystore = R"({
			"coin": "ela",
			"network": "livenet",
			"xPrivKey": "xprv9s21ZrQH143K2C3tdyYrBmtXj6z83YHanKhuMm3PCXoGidNEkfKQYMBtG1Z4GA65VGJMQvBw4y1SpqdeTT25pgNKBSARK5DRp1goQAXrrNp",
			"xPubKey": "xpub6D5r16bFTY3FfNht7kobqQzkAHsUxzfKingYXXYUoTfNDSqCW2yjhHdt9yWRwtxx4zWoJ1m3pEo6hzQTswEA2UeEB16jEnYiHoDFwGH9c9z",
			"requestPrivKey": "763e69ed2899f4302ed2655687f55e5a453dfe52c6bb2d13aea7002ff0cf3c3f",
			"requestPubKey": "0370a77a257aa81f46629865eb8f3ca9cb052fcfd874e8648cfbea1fbf071b0280",
			"copayerId": "03c3e253ca50aa7437560ef4fe9e71c2613297ab37331074028f7af5a6a789fc",
			"publicKeyRing": [
			{
				"xPubKey": "xpub6D5r16bFTY3FfNht7kobqQzkAHsUxzfKingYXXYUoTfNDSqCW2yjhHdt9yWRwtxx4zWoJ1m3pEo6hzQTswEA2UeEB16jEnYiHoDFwGH9c9z",
					"requestPubKey": "0370a77a257aa81f46629865eb8f3ca9cb052fcfd874e8648cfbea1fbf071b0280"
			}
			],
			"walletId": "79d91cc2-0002-4091-b752-d517c71724b5",
			"walletName": "个人钱包",
			"m": 1,
			"n": 1,
			"walletPrivKey": "77fd3b22162b6abf9896cc319ecb01e78946e0e431cf681e0be07386b3d089e1",
			"personalEncryptingKey": "9e8NKebjCQheiNkc4WWTMA==",
			"sharedEncryptingKey": "BIEMo98/cEXf+pY1MnmDHQ==",
			"copayerName": "我",
			"mnemonic": "闲 齿 兰 丹 请 毛 训 胁 浇 摄 县 诉",
			"entropySource": "5480ac5511c2daaa99641ce3547d6a36439b5911bb769d6f79cbec927d81d771",
			"mnemonicHasPassphrase": false,
			"derivationStrategy": "BIP44",
			"account": 0,
			"compliantDerivation": true,
			"addressType": "P2PKH"
		})"_json;

		std::string xPrivKey = keystore["xPrivKey"];
		std::string xPubKey = keystore["xPubKey"];
		std::string requestPrivKey = keystore["requestPrivKey"];
		std::string requestPubKey = keystore["requestPubKey"];
		std::string mnemonic = keystore["mnemonic"];

		std::string phrase = "闲 齿 兰 丹 请 毛 训 胁 浇 摄 县 诉";
		std::string phrasePassword = "";
		std::string payPassword = "12345678";
		uint32_t coinIndex = 0;

		uint512 seed = BIP39::DeriveSeed(phrase, phrasePassword);

		HDSeed hdseed(seed.bytes());
		HDKeychain rootprv(hdseed.getExtendedKey(true));

		REQUIRE(Base58::CheckEncode(rootprv.extkey()) == xPrivKey);

		HDKeychain mpk = rootprv.getChild("44'/0'/0'").getPublic();
		REQUIRE(Base58::CheckEncode(mpk.extkey()) == xPubKey);

		HDKeychain child1 = rootprv.getChild("44'/0'/0'/0/0");
		REQUIRE("Ed8ZSxSB98roeyuRZwwekrnRqcgnfiUDeQ" == Address(PrefixStandard, child1.pubkey()).String());
		REQUIRE("Ed8ZSxSB98roeyuRZwwekrnRqcgnfiUDeQ" == Address(PrefixStandard, mpk.getChild("0/0").pubkey()).String());

		HDKeychain child2 = rootprv.getChild("44'/0'/0'/0/1");
		REQUIRE("EPbdmxUVBzfNrVdqJzZEySyWGYeuKAeKqv" == Address(PrefixStandard, child2.pubkey()).String());
		REQUIRE("EPbdmxUVBzfNrVdqJzZEySyWGYeuKAeKqv" == Address(PrefixStandard, mpk.getChild("0/1").pubkey()).String());

		bytes_t extprv;
		REQUIRE(Base58::CheckDecode(xPrivKey, extprv));
		HDKeychain xprv(extprv);
		REQUIRE(requestPrivKey == xprv.getChild("1'/0").privkey().getHex());
		REQUIRE(requestPubKey == xprv.getChild("1'/0").pubkey().getHex());

		bytes_t extpub;
		REQUIRE(Base58::CheckDecode(xPubKey, extpub));
		HDKeychain xpub(extpub);
		REQUIRE("Ed8ZSxSB98roeyuRZwwekrnRqcgnfiUDeQ" == Address(PrefixStandard, xpub.getChild("0/0").pubkey()).String());
		REQUIRE("EPbdmxUVBzfNrVdqJzZEySyWGYeuKAeKqv" == Address(PrefixStandard, xpub.getChild("0/1").pubkey()).String());
	}

	SECTION("Import old version of spvsdk HD keystore") {
		nlohmann::json keystore = R"({
			"CoSigners": [],
			"CoinInfoList": [
			  {
			    "ChainCode": "cf1cefa0de2e45492eda3780255605b61a75c80ab0593fb099999df000d379ce",
			    "ChainID": "ELA",
			    "EarliestPeerTime": 1547695777,
			    "EnableP2P": true,
			    "FeePerKB": 10000,
			    "Index": 0,
			    "MinFee": 10000,
			    "PublicKey": "0267140bd9e8592c4b2427a580879eb69e6caeac71f51e27094bd164dbb4799e3f",
			    "ReconnectSeconds": 300,
			    "SingleAddress": false,
			    "UsedMaxAddressIndex": 0,
			    "VisibleAssets": [
			      "a3d0eaa466df74983b5d7c543de6904f4c9418ead5ffd6d25814234a96db37b0"
			    ],
			    "WalletType": 1
			  },
			  {
			    "ChainCode": "cf1cefa0de2e45492eda3780255605b61a75c80ab0593fb099999df000d379ce",
			    "ChainID": "IdChain",
			    "EarliestPeerTime": 1550222329,
			    "EnableP2P": true,
			    "FeePerKB": 10000,
			    "Index": 0,
			    "MinFee": 10000,
			    "PublicKey": "0267140bd9e8592c4b2427a580879eb69e6caeac71f51e27094bd164dbb4799e3f",
			    "ReconnectSeconds": 300,
			    "SingleAddress": false,
			    "UsedMaxAddressIndex": 0,
			    "VisibleAssets": [
			      "a3d0eaa466df74983b5d7c543de6904f4c9418ead5ffd6d25814234a96db37b0"
			    ],
			    "WalletType": 3
			  }
			],
			"IsSingleAddress": false,
			"Language": "chinese",
			"PhrasePassword": "",
			"PrivateKey": "",
			"RequiredSignCount": 0,
			"Type": "Standard",
			"account": 0,
			"addressType": "",
			"coin": "",
			"compliantDerivation": false,
			"copayerId": "",
			"copayerName": "",
			"derivationStrategy": "",
			"entropySource": "",
			"m": 0,
			"mnemonic": "闲 齿 兰 丹 请 毛 训 胁 浇 摄 县 诉",
			"mnemonicHasPassphrase": false,
			"n": 0,
			"network": "",
			"personalEncryptingKey": "",
			"publicKeyRing": [],
			"requestPrivKey": "",
			"requestPubKey": "",
			"sharedEncryptingKey": "",
			"walletId": "",
			"walletName": "",
			"walletPrivKey": "",
			"xPrivKey": "",
			"xPubKey": ""
		})"_json;

		std::string chainCode = keystore["CoinInfoList"][0]["ChainCode"];
		std::string pubKey = keystore["CoinInfoList"][0]["PublicKey"];
		std::string mnemonic = keystore["mnemonic"];
		std::string phrasePasswd = keystore["PhrasePassword"];

		uint512 seed = BIP39::DeriveSeed(mnemonic, phrasePasswd);

		HDSeed hdseed(seed.bytes());
		HDKeychain rootprv(hdseed.getExtendedKey(true));

		REQUIRE("0370a77a257aa81f46629865eb8f3ca9cb052fcfd874e8648cfbea1fbf071b0280" == rootprv.getChild("1'/0").pubkey().getHex());

		bytes_t pubkeyBytes(pubKey), chainCodeBytes(chainCode);

		HDKeychain mpk(pubkeyBytes, chainCodeBytes);
		REQUIRE("Ed8ZSxSB98roeyuRZwwekrnRqcgnfiUDeQ" == Address(PrefixStandard, mpk.getChild("0/0").pubkey()).String());
	}

	SECTION("Load from local store") {
		nlohmann::json localstore = R"({
			"Account":{
				"IDChainCode":"2f471c38379a18d72a8dce7624de15416067dd23629b39b37a74a2d8b1188ae1",
				"IDMasterKeyPubKey":"02f5d68f43e36b9d33ee7312565f6661dde331d7e5f627fd4cb86df5bded9479c8",
				"Language":"chinese",
				"Mnemonic":"d0VCHDqNdsy3hnNtUMjNdJ4rD3oNpCuO4LV0qiaQNfeA+CqF+GHXdistXIamJw8tk7cHjsNdEA==",
				"PhrasePassword":"JnjXImPpx6Y=",
				"PublicKey":"0370a77a257aa81f46629865eb8f3ca9cb052fcfd874e8648cfbea1fbf071b0280"
			},
			"AccountType":"Standard",
			"IdAgent":{
				"iT3TsRp6ZwWYoanDKsFngEBwcMQnaRBmG6":{
					"Index":49,
					"PublicKey":"027dc977834740b6e2ddf2194b8117ec923df9b9fee26969b17e6024251a0c9b97",
					"Purpose":0
				},
				"iT7pWs92KYmpSVRuukjLpRNsGcNrqX53Ea":{
					"Index":15,
					"PublicKey":"03d6c7772508f446f0d34d3baf41e851e663ae7e811dc4b7b6c6c0de20d754434d",
					"Purpose":0
				},
				"iTFNgWHCJ51SsFZQuZEGXrgDUiZcTazVSm":{
					"Index":21,
					"PublicKey":"027acd6c664821dc9ef018b7f0e6fc7ca9959f2ab7adb7eed68418e8ef4db23699",
					"Purpose":0
				}
			},
			"IsSingleAddress":false,
			"MasterPubKey":{
				"ELA":"00000000cf1cefa0de2e45492eda3780255605b61a75c80ab0593fb099999df000d379ce0267140bd9e8592c4b2427a580879eb69e6caeac71f51e27094bd164dbb4799e3f",
				"IdChain":"00000000cf1cefa0de2e45492eda3780255605b61a75c80ab0593fb099999df000d379ce0267140bd9e8592c4b2427a580879eb69e6caeac71f51e27094bd164dbb4799e3f",
				"TokenChain":"00000000cf1cefa0de2e45492eda3780255605b61a75c80ab0593fb099999df000d379ce0267140bd9e8592c4b2427a580879eb69e6caeac71f51e27094bd164dbb4799e3f"
			},
			"SubWallets":[
			{
				"ChainCode":"cf1cefa0de2e45492eda3780255605b61a75c80ab0593fb099999df000d379ce",
				"ChainID":"ELA",
				"EarliestPeerTime":1513936800,
				"EnableP2P":true,
				"FeePerKB":10000,
				"Index":0,
				"MinFee":10000,
				"PublicKey":"0267140bd9e8592c4b2427a580879eb69e6caeac71f51e27094bd164dbb4799e3f",
				"ReconnectSeconds":300,
				"SingleAddress":false,
				"UsedMaxAddressIndex":0,
				"VisibleAssets":[
					"a3d0eaa466df74983b5d7c543de6904f4c9418ead5ffd6d25814234a96db37b0"
				],
				"WalletType":1
			},
			{
				"ChainCode":"cf1cefa0de2e45492eda3780255605b61a75c80ab0593fb099999df000d379ce",
				"ChainID":"IdChain",
				"EarliestPeerTime":1513936800,
				"EnableP2P":true,
				"FeePerKB":10000,
				"Index":0,
				"MinFee":10000,
				"PublicKey":"0267140bd9e8592c4b2427a580879eb69e6caeac71f51e27094bd164dbb4799e3f",
				"ReconnectSeconds":300,
				"SingleAddress":false,
				"UsedMaxAddressIndex":0,
				"VisibleAssets":[
					"a3d0eaa466df74983b5d7c543de6904f4c9418ead5ffd6d25814234a96db37b0"
				],
				"WalletType":3
			}
			],
			"VotePublicKey":{
				"ELA":"03d916c2072fd8fb57224e9747e0f1e36a2c117689cedf39e0132f3cb4f8ee673d",
				"IdChain":"",
				"TokenChain":""
			}
		})"_json;

		std::string masterPubKey = localstore["MasterPubKey"]["ELA"];
		std::string mnemonic = localstore["Account"]["Mnemonic"];
		std::string phrasePasswd = localstore["Account"]["PhrasePassword"];

		bytes_t bytes(masterPubKey);
		ByteStream stream(bytes);
		stream.Skip(4);
		bytes_t chainCode, pubKey;
		REQUIRE(stream.ReadBytes(chainCode, 32));
		REQUIRE(stream.ReadBytes(pubKey, 33));

		HDKeychain mpk(pubKey, chainCode);
		REQUIRE("Ed8ZSxSB98roeyuRZwwekrnRqcgnfiUDeQ" == Address(PrefixStandard, mpk.getChild("0/0").pubkey()).String());


		REQUIRE_NOTHROW(bytes = AES::DecryptCCM(mnemonic, "s12345678"));
		std::string phrase((char *)bytes.data(), bytes.size());

		REQUIRE_NOTHROW(bytes = AES::DecryptCCM(phrasePasswd, "s12345678"));
		std::string phrasepass((char *)bytes.data(), bytes.size());

		uint512 seed = BIP39::DeriveSeed(phrase, phrasepass);

		HDSeed hdseed(seed.bytes());
		HDKeychain rootprv(hdseed.getExtendedKey(true));

		REQUIRE("0370a77a257aa81f46629865eb8f3ca9cb052fcfd874e8648cfbea1fbf071b0280" == rootprv.getChild("1'/0").pubkey().getHex());
	}
}
