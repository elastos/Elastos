// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.
#define CATCH_CONFIG_MAIN


#include <catch.hpp>

#include <SpvService/LocalStore.h>
#include <WalletCore/Base58.h>
#include <WalletCore/HDKeychain.h>
#include <WalletCore/AES.h>
#include <Account/Account.h>
#include <Common/Log.h>

#include <nlohmann/json.hpp>

#include <fstream>

using namespace Elastos::ElaWallet;

TEST_CASE("LocalStore test", "[LocalStore]") {
	Log::registerMultiLogger();
	const std::string payPasswd = "Abcd1234";

	SECTION("old version of local store") {
		SECTION("single address | with passphrase | two subwallets") {
			const std::string mnemonic = "ride chalk song document stem want vocal win birth hotel pottery kitchen";
			const std::string passphrase = "Abcd1234";
			nlohmann::json localstoreJson = nlohmann::json::parse(
				"{\"Account\":{\"IDChainCode\":\"52a3b1c2569a62377000cba9992467d9d17b80e568b5c3a14687b924ede17ec2\",\"IDMasterKeyPubKey\":\"0326f9c3f284f870731f636bc21dc48923205533e04f1736fd2c2deef1e3935b1a\",\"Language\":\"\",\"Mnemonic\":\"svov3qpHaLRbgCpOWTPJgcPajcrvSsCIH0V7Q3RfZHN8Q/S4c/eRUw6GAmzJpgANiZwimYG3Gs22dEqF7klIHOIm4qu/u+d9x6+5zYa6L+M=\",\"PhrasePassword\":\"gfEo37sWM+HRrRVVvzjzwQ==\",\"PublicKey\":\"039a84c9f9a0467bd0db82a0586218d3669522987b196774807858bb88a7882b45\"},\"AccountType\":\"Standard\",\"IsSingleAddress\":true,\"MasterPubKey\":{\"ELA\":\"0000000052a3b1c2569a62377000cba9992467d9d17b80e568b5c3a14687b924ede17ec20326f9c3f284f870731f636bc21dc48923205533e04f1736fd2c2deef1e3935b1a\",\"IdChain\":\"0000000052a3b1c2569a62377000cba9992467d9d17b80e568b5c3a14687b924ede17ec20326f9c3f284f870731f636bc21dc48923205533e04f1736fd2c2deef1e3935b1a\"},\"SubWallets\":[{\"ChainCode\":\"52a3b1c2569a62377000cba9992467d9d17b80e568b5c3a14687b924ede17ec2\",\"ChainID\":\"ELA\",\"EarliestPeerTime\":1557710709,\"EnableP2P\":true,\"FeePerKB\":10000,\"Index\":0,\"MinFee\":10000,\"PublicKey\":\"0326f9c3f284f870731f636bc21dc48923205533e04f1736fd2c2deef1e3935b1a\",\"ReconnectSeconds\":300,\"SingleAddress\":true,\"UsedMaxAddressIndex\":0,\"VisibleAssets\":[\"a3d0eaa466df74983b5d7c543de6904f4c9418ead5ffd6d25814234a96db37b0\"],\"WalletType\":1},{\"ChainCode\":\"52a3b1c2569a62377000cba9992467d9d17b80e568b5c3a14687b924ede17ec2\",\"ChainID\":\"IdChain\",\"EarliestPeerTime\":1554636477,\"EnableP2P\":true,\"FeePerKB\":10000,\"Index\":0,\"MinFee\":10000,\"PublicKey\":\"0326f9c3f284f870731f636bc21dc48923205533e04f1736fd2c2deef1e3935b1a\",\"ReconnectSeconds\":300,\"SingleAddress\":true,\"UsedMaxAddressIndex\":0,\"VisibleAssets\":[\"a3d0eaa466df74983b5d7c543de6904f4c9418ead5ffd6d25814234a96db37b0\"],\"WalletType\":3}],\"VotePublicKey\":{\"ELA\":\"028f533fb431cb4cffc9f6aec912b3fec958ef57a38382e86bf4a91295603363a3\",\"IdChain\":\"\"}}");
			LocalStorePtr store(new LocalStore(localstoreJson));

			Account account(store);
			account.RegenerateKey(payPasswd);

			bytes_t xpubkey;
			REQUIRE(Base58::CheckDecode(store->GetxPubKey(), xpubkey));

			HDKeychain mpk(xpubkey);
			Address addr(PrefixStandard, mpk.getChild("0/0").pubkey());

			bytes_t xprvkey = AES::DecryptCCM(store->GetxPrivKey(), payPasswd);
			HDKeychain rootprv(xprvkey);
			Address addrprv(PrefixStandard, rootprv.getChild("44'/0'/0'/0/0").pubkey());
			REQUIRE(addrprv.String() == addr.String());
			REQUIRE(addr.String() == "ENjw5MoRDuKRsuZ6WhhbzxmJ1yuSvecLSf");

			REQUIRE(store->SingleAddress());

			REQUIRE(store->GetM() == 1);
			REQUIRE(store->GetN() == 1);
			REQUIRE(store->HasPassPhrase());

			bytes_t bytes = AES::DecryptCCM(store->GetPassPhrase(), payPasswd);
			REQUIRE(bytes.size() == 0);

			bytes = AES::DecryptCCM(store->GetMnemonic(), payPasswd);
			REQUIRE(std::string((char *)bytes.data(), bytes.size()) == mnemonic);

			REQUIRE(!store->GetOwnerPubKey().empty());
			REQUIRE(!store->Readonly());
			REQUIRE(store->GetPublicKeyRing().size() == 1);
			if (store->GetN() > 1 && store->DerivationStrategy() == "BIP44")
				REQUIRE(store->GetPublicKeyRing()[0].GetxPubKey() == store->GetxPubKey());
			else if (store->DerivationStrategy() == "BIP45")
				REQUIRE(store->GetPublicKeyRing()[0].GetxPubKey() == store->GetxPubKeyHDPM());
			REQUIRE(store->GetPublicKeyRing()[0].GetRequestPubKey() == store->GetRequestPubKey());
			REQUIRE(store->GetSubWalletInfoList().size() == 2);
		}

		SECTION("not single address | with passphrase | one subwallets") {
			const std::string mnemonic = "symptom glove trigger fossil stage valid cherry bid dolphin kit promote vital";
			const std::string passphrase = "Abcd1234";
			nlohmann::json localstoreJson = R"({
			  "Account": {
			    "IDChainCode": "d48fd1d67e60e6bb609681e675360fb8b8ee75e5d7c690fbb52810b1d479c420",
			    "IDMasterKeyPubKey": "02e84f4fe1312f5034068bc2c0c19698881fa74142e2d272a6bfd60af5daa545e6",
			    "Language": "",
			    "Mnemonic": "s+omy/5LbfVQh2VLU33a087SidrwD8iTTEVmSjkMZ3N1UvS4ffiZWw6SA2ebthBfn51m0YqsE9H+bUvR8UVORbI/5LKzp+czNukolqJHj1U2VlRJyw==",
			    "PhrasePassword": "gfEo37sWM+HRrRVVvzjzwQ==",
			    "PublicKey": "02d7f03046c6b3c09c8d48eb9a0ed79e8c4ba20e714c625ce2e2f7ddb6eef16914"
			  },
			  "AccountType": "Standard",
			  "IsSingleAddress": false,
			  "MasterPubKey": {
			    "ELA": "00000000d48fd1d67e60e6bb609681e675360fb8b8ee75e5d7c690fbb52810b1d479c42002e84f4fe1312f5034068bc2c0c19698881fa74142e2d272a6bfd60af5daa545e6",
			    "IdChain": "00000000d48fd1d67e60e6bb609681e675360fb8b8ee75e5d7c690fbb52810b1d479c42002e84f4fe1312f5034068bc2c0c19698881fa74142e2d272a6bfd60af5daa545e6"
			  },
			  "SubWallets": [
			    {
			      "ChainCode": "d48fd1d67e60e6bb609681e675360fb8b8ee75e5d7c690fbb52810b1d479c420",
			      "ChainID": "ELA",
			      "EarliestPeerTime": 1557710709,
			      "EnableP2P": true,
			      "FeePerKB": 10000,
			      "Index": 0,
			      "MinFee": 10000,
			      "PublicKey": "02e84f4fe1312f5034068bc2c0c19698881fa74142e2d272a6bfd60af5daa545e6",
			      "ReconnectSeconds": 300,
			      "SingleAddress": false,
			      "UsedMaxAddressIndex": 0,
			      "VisibleAssets": [
			        "a3d0eaa466df74983b5d7c543de6904f4c9418ead5ffd6d25814234a96db37b0"
			      ],
			      "WalletType": 1
			    }
			  ],
			  "VotePublicKey": {
			    "ELA": "03257bdef862f98fa20cfb64046c612bcd2d69fa11dc146616324affd45060d15f",
			    "IdChain": ""
			  }
			})"_json;

			LocalStorePtr store(new LocalStore(localstoreJson));
			Account account(store);

			account.RegenerateKey(payPasswd);

			bytes_t xpubkey;
			REQUIRE(Base58::CheckDecode(store->GetxPubKey(), xpubkey));

			HDKeychain mpk(xpubkey);
			Address addr(PrefixStandard, mpk.getChild("0/0").pubkey());

			bytes_t xprvkey = AES::DecryptCCM(store->GetxPrivKey(), payPasswd);
			HDKeychain rootprv(xprvkey);
			Address addrprv(PrefixStandard, rootprv.getChild("44'/0'/0'/0/0").pubkey());
			REQUIRE(addrprv.String() == addr.String());
			REQUIRE(addr.String() == "EWkMVjB48znLGXCNNpLzf2rLmnX1iU91FP");

			REQUIRE(!store->SingleAddress());

			REQUIRE(store->GetM() == 1);
			REQUIRE(store->GetN() == 1);
			REQUIRE(store->HasPassPhrase());

			bytes_t bytes = AES::DecryptCCM(store->GetPassPhrase(), payPasswd);
			REQUIRE(bytes.size() == 0);

			bytes = AES::DecryptCCM(store->GetMnemonic(), payPasswd);
			REQUIRE(std::string((char *)bytes.data(), bytes.size()) == mnemonic);

			REQUIRE(!store->GetOwnerPubKey().empty());
			REQUIRE(!store->Readonly());
			REQUIRE(store->GetPublicKeyRing().size() == 1);
			if (store->GetN() > 1 && store->DerivationStrategy() == "BIP44")
				REQUIRE(store->GetPublicKeyRing()[0].GetxPubKey() == store->GetxPubKey());
			else if (store->DerivationStrategy() == "BIP45")
				REQUIRE(store->GetPublicKeyRing()[0].GetxPubKey() == store->GetxPubKeyHDPM());
			REQUIRE(store->GetPublicKeyRing()[0].GetRequestPubKey() == store->GetRequestPubKey());
			REQUIRE(store->GetSubWalletInfoList().size() == 1);
		}

		SECTION("not single address | without passphrase | two subwallets") {
			const std::string mnemonic = "卢 口 宴 点 睛 量 旦 奥 沫 洪 鲁 距";
			const std::string passphrase = "";
			const nlohmann::json localstoreJson = R"({
				  "Account": {
				    "IDChainCode": "ed55a7268e90643ba68ab15854ca9eed551fbd7f60d36d278987964c4f26d6e4",
				    "IDMasterKeyPubKey": "03810487f53d114c2415582c29e5b6db69a2ad1bff0d31576270a577dbffdbe332",
				    "Language": "",
				    "Mnemonic": "JR7pm2+ro/XSRb4d0d8XgUAodZ9rqCHc2aGpBvzatjL0hX/u+iBaH8dD6iIBc/RxpDeoxzlWDQ==",
				    "PhrasePassword": "",
				    "PublicKey": "03f1e471a39c79612b636dbd03dfa5c2c84f7217ceebde4641b61f98609a57ff1c"
				  },
				  "AccountType": "Standard",
				  "IsSingleAddress": false,
				  "MasterPubKey": {
				    "ELA": "00000000ed55a7268e90643ba68ab15854ca9eed551fbd7f60d36d278987964c4f26d6e403810487f53d114c2415582c29e5b6db69a2ad1bff0d31576270a577dbffdbe332",
				    "IdChain": "00000000ed55a7268e90643ba68ab15854ca9eed551fbd7f60d36d278987964c4f26d6e403810487f53d114c2415582c29e5b6db69a2ad1bff0d31576270a577dbffdbe332"
				  },
				  "SubWallets": [
				    {
				      "ChainCode": "ed55a7268e90643ba68ab15854ca9eed551fbd7f60d36d278987964c4f26d6e4",
				      "ChainID": "ELA",
				      "EarliestPeerTime": 1557491846,
				      "EnableP2P": true,
				      "FeePerKB": 10000,
				      "Index": 0,
				      "MinFee": 10000,
				      "PublicKey": "03810487f53d114c2415582c29e5b6db69a2ad1bff0d31576270a577dbffdbe332",
				      "ReconnectSeconds": 300,
				      "SingleAddress": false,
				      "UsedMaxAddressIndex": 0,
				      "VisibleAssets": [
				        "a3d0eaa466df74983b5d7c543de6904f4c9418ead5ffd6d25814234a96db37b0"
				      ],
				      "WalletType": 1
				    },
				    {
				      "ChainCode": "ed55a7268e90643ba68ab15854ca9eed551fbd7f60d36d278987964c4f26d6e4",
				      "ChainID": "IdChain",
				      "EarliestPeerTime": 1557809019,
				      "EnableP2P": true,
				      "FeePerKB": 10000,
				      "Index": 0,
				      "MinFee": 10000,
				      "PublicKey": "03810487f53d114c2415582c29e5b6db69a2ad1bff0d31576270a577dbffdbe332",
				      "ReconnectSeconds": 300,
				      "SingleAddress": false,
				      "UsedMaxAddressIndex": 0,
				      "VisibleAssets": [
				        "a3d0eaa466df74983b5d7c543de6904f4c9418ead5ffd6d25814234a96db37b0"
				      ],
				      "WalletType": 3
				    }
				  ],
				  "VotePublicKey": {
				    "ELA": "0359541169b236eaebe0a4352ab8c57294073e804ba9b622d9c1fd312b3e628a68",
				    "IdChain": ""
				  }
				})"_json;
			LocalStorePtr store(new LocalStore(localstoreJson));

			Account account(store);
			account.RegenerateKey(payPasswd);

			bytes_t xpubkey;
			REQUIRE(Base58::CheckDecode(store->GetxPubKey(), xpubkey));

			HDKeychain mpk(xpubkey);
			Address addr(PrefixStandard, mpk.getChild("0/0").pubkey());

			bytes_t xprvkey = AES::DecryptCCM(store->GetxPrivKey(), payPasswd);
			HDKeychain rootprv(xprvkey);
			Address addrprv(PrefixStandard, rootprv.getChild("44'/0'/0'/0/0").pubkey());
			REQUIRE(addrprv.String() == addr.String());
			REQUIRE(addr.String() == "EPmxXC5orLrXd1FJK8Skz5tmf2gDreh8fd");

			REQUIRE(!store->SingleAddress());

			REQUIRE(store->GetM() == 1);
			REQUIRE(store->GetN() == 1);
			REQUIRE(!store->HasPassPhrase());

			bytes_t bytes = AES::DecryptCCM(store->GetPassPhrase(), payPasswd);
			REQUIRE(bytes.size() == 0);

			bytes = AES::DecryptCCM(store->GetMnemonic(), payPasswd);
			REQUIRE(std::string((char *)bytes.data(), bytes.size()) == mnemonic);

			REQUIRE(!store->GetOwnerPubKey().empty());
			REQUIRE(!store->Readonly());
			REQUIRE(store->GetPublicKeyRing().size() == 1);
			if (store->GetN() > 1 && store->DerivationStrategy() == "BIP44")
				REQUIRE(store->GetPublicKeyRing()[0].GetxPubKey() == store->GetxPubKey());
			else if (store->DerivationStrategy() == "BIP45")
				REQUIRE(store->GetPublicKeyRing()[0].GetxPubKey() == store->GetxPubKeyHDPM());
			REQUIRE(store->GetPublicKeyRing()[0].GetRequestPubKey() == store->GetRequestPubKey());
			REQUIRE(store->GetSubWalletInfoList().size() == 2);
		}
	}

}