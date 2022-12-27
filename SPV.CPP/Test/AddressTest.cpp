// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#define CATCH_CONFIG_MAIN

#include <catch.hpp>
#include <WalletCore/Address.h>
#include <WalletCore/HDKeychain.h>
#include <WalletCore/Mnemonic.h>
#include <Common/Log.h>
#include <WalletCore/Base58.h>
#include <support/BRKey.h>
#include <ethereum/ewm/BREthereumAccount.h>
#include <WalletCore/Key.h>
#include <WalletCore/Mnemonic.h>
#include <utf8proc.h>

using namespace Elastos::ElaWallet;

TEST_CASE("Address test", "[Address]") {
	Log::registerMultiLogger();

	SECTION("Address with public key derived from mnemonic") {

		std::string phrase = "闲 齿 兰 丹 请 毛 训 胁 浇 摄 县 诉";
		std::string phrasePasswd = "";

		uint512 seed = Mnemonic::DeriveSeed(phrase, phrasePasswd);

		HDKeychain child = HDKeychain(CTElastos, HDSeed(seed.bytes()).getExtendedKey(CTElastos, true)).getChild("44'/0'/0'/0/0");

		REQUIRE("Ed8ZSxSB98roeyuRZwwekrnRqcgnfiUDeQ" == Address(PrefixStandard, child.pubkey()).String());
	}

	SECTION("eth sidechain address test") {
        std::string m = "abandon abandon abandon abandon abandon abandon abandon abandon abandon abandon abandon about";
        std::string addr = "0x9858EfFD232B4033E47d90003D41EC34EcaEda94";
        uint512 seed = Mnemonic::DeriveSeed(m, "");
        UInt512 ethseed = *(UInt512 *) seed.begin();
        BREthereumAccount account = createAccountWithBIP32Seed(ethseed);

        // test private key
        BRKey prvkey1 = derivePrivateKeyFromSeed(ethseed, 0);
        HDKeychain root = HDKeychain(CTBitcoin, HDSeed(seed.bytes()).getExtendedKey(CTBitcoin, true));
        HDKeychain prvkey2 = root.getChild("44'/60'/0'/0/0");
        REQUIRE(prvkey2.privkey().getHex() == bytes_t(prvkey1.secret.u8, prvkey1.secret.u8 + sizeof(UInt256)).getHex());

        // test pub key
        BRKey k1 = accountGetPrimaryAddressPublicKey(account);
        bytes_t puk1(k1.pubKey, k1.pubKey + sizeof(k1.pubKey));
        bytes_t puk2 = prvkey2.uncompressed_pubkey();
        REQUIRE(puk1.getHex() == puk2.getHex());

        // test address
        char *paddr = accountGetPrimaryAddressString (account);
        std::string address1(paddr);
        free(paddr);
        REQUIRE(address1 == addr);
        accountFree(account);

        // test address
        BREthereumAddress raw = addressCreateKey(&k1);
        char *s = addressGetEncodedString(&raw, 1);
        std::string address2(s);
        REQUIRE(address2 == addr);
        free(s);
    }

	SECTION("btc address test") {
	    //test data from https://iancoleman.io/bip39/
	    std::string m = "abandon abandon abandon abandon abandon abandon abandon abandon abandon abandon abandon about";
	    std::string s = "5eb00bbddcf069084889a8ab9155568165f5c453ccb85e70811aaed6f6da5fc19a5ac40b389cd370d086206dec8aa6c43daea6690f20ad3d8d48b2d2ce9e38e4";
	    std::string xprvroot = "xprv9s21ZrQH143K3GJpoapnV8SFfukcVBSfeCficPSGfubmSFDxo1kuHnLisriDvSnRRuL2Qrg5ggqHKNVpxR86QEC8w35uxmGoggxtQTPvfUu";
	    std::string bip32xprv = "xprvA1Lvv1qpvx3f8iuRHfaEG45fyvDc3h7Ur5afz5SyRfkAsZ2765KfFfmg6Q9oEJDgf4UdYHphzzJybLykZfznUMKL2KNUU8pLRQgstN5kmFe";
	    std::string bip32xpub = "xpub6ELHKXNimKbxMCytPh7EdC2QXx46T9qLDJWGnTraz1H9kMMFdcduoU69wh9cxP12wDxqAAfbaESWGYt5rREsX1J8iR2TEunvzvddduAPYcY";
	    std::string addr1 = "1LqBGSKuX5yYUonjxT5qGfpUsXKYYWeabA";
        std::string addr2 = "1Ak8PffB2meyfYnbXZR9EGfLfFZVpzJvQP";
        std::string pubkey1 = "03aaeb52dd7494c361049de67cc680e83ebcbbbdbeb13637d92cd845f70308af5e";
        std::string pubkey2 = "02dfcaec532010d704860e20ad6aff8cf3477164ffb02f93d45c552dadc70ed24f";
        std::string prvkey1 = "L4p2b9VAf8k5aUahF1JCJUzZkgNEAqLfq8DDdQiyAprQAKSbu8hf";
        std::string prvkey2 = "KzJgGiEeGUVWmPR97pVWDnCVraZvM2fnrCVrg2irV4353HciE6Un";

	    uint512 seed = Mnemonic::DeriveSeed(m, "");
	    bytes_t seedBytes = seed.bytes();
        REQUIRE(seedBytes.getHex() == s);

        HDKeychain root = HDKeychain(CTBitcoin, HDSeed(seedBytes).getExtendedKey(CTBitcoin, true));
        REQUIRE(xprvroot == Base58::CheckEncode(root.extkey()));

        HDKeychain bip32 = root.getChild("44'/0'/0'/0");
        REQUIRE(bip32xprv == Base58::CheckEncode(bip32.extkey()));
        REQUIRE(bip32xpub == Base58::CheckEncode(bip32.getPublic().extkey()));

        HDKeychain hdkey1 = bip32.getChild(0);
        HDKeychain hdkey2 = bip32.getChild(1);

        char addr[100];
        BRKey key;
        bytes_t prvbytes;

        key.secret = *(UInt256*) hdkey1.privkey().data();
        memcpy(key.pubKey, hdkey1.pubkey().data(), hdkey1.pubkey().size());
        key.compressed = 1;
        BRKeyLegacyAddr(&key, addr, sizeof(addr), BITCOIN_ADDRESS_PARAMS);
        REQUIRE(std::string(addr) == addr1);
        REQUIRE(hdkey1.pubkey().getHex() == pubkey1);

        prvbytes = hdkey1.privkey();
        prvbytes.insert(prvbytes.begin(), 0x80);
        prvbytes.push_back(1);
        REQUIRE(Base58::CheckEncode(prvbytes) == prvkey1);

        key.secret = *(UInt256*) hdkey2.privkey().data();
        memcpy(key.pubKey, hdkey2.pubkey().data(), hdkey2.pubkey().size());
        key.compressed = 1;
        BRKeyLegacyAddr(&key, addr, sizeof(addr), BITCOIN_ADDRESS_PARAMS);
        REQUIRE(std::string(addr) == addr2);
        REQUIRE(hdkey2.pubkey().getHex() == pubkey2);

        bytes_t bytes = hash160(hdkey2.pubkey());
        bytes.insert(bytes.begin(), BITCOIN_PUBKEY_PREFIX);
        REQUIRE(Base58::CheckEncode(bytes) == addr2);

        prvbytes = hdkey2.privkey();
        prvbytes.insert(prvbytes.begin(), 0x80);
        prvbytes.push_back(1);
        REQUIRE(Base58::CheckEncode(prvbytes) == prvkey2);
	}

    SECTION("btc french mnemonic test") {
        //test data from https://iancoleman.io/bip39/
        std::string m = "tenir jouissif brique tangible marathon kimono sécher pleurer période aboutir choisir berceau";
        std::string xprvroot = "xprv9s21ZrQH143K31wMPD5ehQ1mYBAthJQh7DVPJyCry9Vu9t6QZhVoMSu8iJNDUMkqMorMu1n96hvbE2A2mhGehvSV8p8n9YH23DpE7dXRBXm";
        std::string addr = "189R7P7RVLNsn7wtfDvNtPven9WeB9Vgw1";

        uint512 seed;

        REQUIRE_NOTHROW(seed = Mnemonic::DeriveSeed(m, ""));

        bytes_t seedBytes = seed.bytes();

        HDKeychain root = HDKeychain(CTBitcoin, HDSeed(seedBytes).getExtendedKey(CTBitcoin, true));
        REQUIRE(xprvroot == Base58::CheckEncode(root.extkey()));

        bytes_t bytes = hash160(root.getChild("44'/0'/0'/0/0").pubkey());
        bytes.insert(bytes.begin(), BITCOIN_PUBKEY_PREFIX);
        REQUIRE(Base58::CheckEncode(bytes) == addr);
    }

	SECTION("eth private key to asddress") {
        bytes_t prvkey = bytes_t("1ab42cc412b618bdea3a599e3c9bae199ebf030895b039e9db1e30dafb12b727");
        Key k(CTBitcoin, prvkey);
        bytes_t ethpubkey = k.PubKey(false);
        REQUIRE(ethpubkey.size() == 65);

        BRKey kk;
        memset(&kk, 0, sizeof(kk));

        memcpy(kk.pubKey, ethpubkey.data(), ethpubkey.size());
        BREthereumAddress raw = addressCreateKey(&kk);
        char *s = addressGetEncodedString(&raw, 1);
        REQUIRE(std::string(s) == "0x9858EfFD232B4033E47d90003D41EC34EcaEda94");
	}
}
