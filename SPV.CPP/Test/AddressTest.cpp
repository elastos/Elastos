// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#define CATCH_CONFIG_MAIN

#include <catch.hpp>
#include <WalletCore/Address.h>
#include <WalletCore/HDKeychain.h>
#include <WalletCore/BIP39.h>
#include <Common/Log.h>
#include <WalletCore/Base58.h>
#include <support/BRKey.h>

using namespace Elastos::ElaWallet;

TEST_CASE("Address test", "[Address]") {
	Log::registerMultiLogger();

	SECTION("Address with public key derived from mnemonic") {

		std::string phrase = "闲 齿 兰 丹 请 毛 训 胁 浇 摄 县 诉";
		std::string phrasePasswd = "";

		uint512 seed = BIP39::DeriveSeed(phrase, phrasePasswd);

		HDKeychain child = HDKeychain(CTElastos, HDSeed(seed.bytes()).getExtendedKey(CTElastos, true)).getChild("44'/0'/0'/0/0");

		REQUIRE("Ed8ZSxSB98roeyuRZwwekrnRqcgnfiUDeQ" == Address(PrefixStandard, child.pubkey()).String());
	}

	SECTION("btc address test") {
	    //test data from https://iancoleman.io/bip39/
	    std::string m = "flat universe quantum uniform emerge blame lemon detail april sting aerobic disease";
	    std::string s = "091454fb8f3cb75b0769de91ec854b6da6485f4adf32c0aef57a31d5268b5765def0e6e99ca0f29652796f73176d322f5800b8e3031fa06cc9ae58bc55c42e37";
	    std::string xprvroot = "xprv9s21ZrQH143K2jaN5BgmoMDgRz8L8MCgG3gLJcFZ73iaWXUh7uQJ3ZeEAFDVayUU6vV8E2mDWBfnTcdYqEECCoKEcCo9g6WzCuJY8q7Pxin";
	    std::string bip32xprv = "xprvA2HgDDozXrCzsgVcZQ4Dkwy8kJzKp2hM8ZtkYUNo8bDyahXKozLsm66n15DPmeGBDBBgWoewdWw1pu4tFCSS2nbzPSbvE6a74xHULAzJFgQ";
	    std::string bip32xpub = "xpub6FH2cjLtNDmJ6Aa5fRbE85usJLppDVRCVnpMLrnQgvkxTVrUMXf8JtRFrKQc3F65HCikCm9aXBTpXYwhNvCJSixZrrtFYQmr8Hr8hd3UnVb";
	    std::string addr1 = "1BzBrj4gdLfiw4sVwZaH2ftSmH8pcBDuUX";
        std::string addr2 = "1MtF6uf4ppre9iNm9SoFdVkhNy5oUuF79P";
        std::string pubkey1 = "03d7bdaee7a232534bc4fb2f19c40b46fe0d8a621846006aaf7b672373f40952b7";
        std::string pubkey2 = "02a8f87e78c10407eda01983c84b74e9a71c75749aec3d23620bbdf9e10a1a9556";
        std::string prvkey1 = "L568vL3Dfs47rursz5rv1PUX9RyY6C2KnhHybTDxf88cH14k2TYR";
        std::string prvkey2 = "KzFPvp8pGg8y9A24RvzHjkidZfjwsUPtFVXfLi2FnQjXwY1H8VF7";

	    uint512 seed = BIP39::DeriveSeed(m, "");
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
        BRKeyLegacyAddr(&key, addr, sizeof(addr));
        REQUIRE(std::string(addr) == addr1);
        REQUIRE(hdkey1.pubkey().getHex() == pubkey1);

        prvbytes = hdkey1.privkey();
        prvbytes.insert(prvbytes.begin(), 0x80);
        prvbytes.push_back(1);
        REQUIRE(Base58::CheckEncode(prvbytes) == prvkey1);

        key.secret = *(UInt256*) hdkey2.privkey().data();
        memcpy(key.pubKey, hdkey2.pubkey().data(), hdkey2.pubkey().size());
        key.compressed = 1;
        BRKeyLegacyAddr(&key, addr, sizeof(addr));
        REQUIRE(std::string(addr) == addr2);
        REQUIRE(hdkey2.pubkey().getHex() == pubkey2);

        prvbytes = hdkey2.privkey();
        prvbytes.insert(prvbytes.begin(), 0x80);
        prvbytes.push_back(1);
        REQUIRE(Base58::CheckEncode(prvbytes) == prvkey2);
	}
}
