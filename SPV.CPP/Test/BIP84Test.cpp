/*
 * Copyright (c) 2019 Elastos Foundation
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#define CATCH_CONFIG_MAIN

#include <catch.hpp>
#include <WalletCore/Address.h>
#include <WalletCore/HDKeychain.h>
#include <WalletCore/BIP39.h>
#include <Common/Log.h>
#include <WalletCore/Base58.h>
#include <support/BRKey.h>
#include <ethereum/ewm/BREthereumAccount.h>

using namespace Elastos::ElaWallet;

TEST_CASE("BIP84", "test") {
    Log::registerMultiLogger();

    SECTION("standard bip84 test") {
        std::string mnemonic = "abandon abandon abandon abandon abandon abandon abandon abandon abandon abandon abandon about";
        std::string rootpriv = "zprvAWgYBBk7JR8Gjrh4UJQ2uJdG1r3WNRRfURiABBE3RvMXYSrRJL62XuezvGdPvG6GFBZduosCc1YP5wixPox7zhZLfiUm8aunE96BBa4Kei5";
        std::string rootpub  = "zpub6jftahH18ngZxLmXaKw3GSZzZsszmt9WqedkyZdezFtWRFBZqsQH5hyUmb4pCEeZGmVfQuP5bedXTB8is6fTv19U1GQRyQUKQGUTzyHACMF";

        // Account 0, root = m/84'/0'/0'
        std::string xpriv = "zprvAdG4iTXWBoARxkkzNpNh8r6Qag3irQB8PzEMkAFeTRXxHpbF9z4QgEvBRmfvqWvGp42t42nvgGpNgYSJA9iefm1yYNZKEm7z6qUWCroSQnE";
        std::string xpub  = "zpub6rFR7y4Q2AijBEqTUquhVz398htDFrtymD9xYYfG1m4wAcvPhXNfE3EfH1r1ADqtfSdVCToUG868RvUUkgDKf31mGDtKsAYz2oz2AGutZYs";

        // Account 0, first receiving address = m/84'/0'/0'/0/0
        std::string privkey00 = "KyZpNDKnfs94vbrwhJneDi77V6jF64PWPF8x5cdJb8ifgg2DUc9d";
        std::string pubkey00  = "0330d54fd0dd420a6e5f8d3624f5f3482cae350f79d5f0753bf5beef9c2d91af3c";
        std::string address00 = "bc1qcr8te4kr609gcawutmrza0j4xv80jy8z306fyu";

        // Account 0, second receiving address = m/84'/0'/0'/0/1
        std::string privkey01 = "Kxpf5b8p3qX56DKEe5NqWbNUP9MnqoRFzZwHRtsFqhzuvUJsYZCy";
        std::string pubkey01  = "03e775fd51f0dfb8cd865d9ff1cca2a158cf651fe997fdc9fee9c1d3b5e995ea77";
        std::string address01 = "bc1qnjg0jd8228aq7egyzacy8cys3knf9xvrerkf9g";

        // Account 0, first change address = m/84'/0'/0'/1/0
        std::string privkey10 = "KxuoxufJL5csa1Wieb2kp29VNdn92Us8CoaUG3aGtPtcF3AzeXvF";
        std::string pubkey10  = "03025324888e429ab8e3dbaf1f7802648b9cd01e9b418485c5fa4c1b9b5700e1a6";
        std::string address10 = "bc1q8c6fshw2dlwun7ekn9qwf37cu2rn755upcp6el";

        HDKeychain::setVersions(0x04b2430c, 0x04b24746);
        uint512 seed = BIP39::DeriveSeed(mnemonic, "");
        HDKeychain rootkey = HDKeychain(CTBitcoin, HDSeed(seed.bytes()).getExtendedKey(CTBitcoin, true));
        HDKeychain xkey = rootkey.getChild("84'/0'/0'");
        HDKeychain key00 = xkey.getChild("0/0");
        HDKeychain key01 = xkey.getChild("0/1");
        HDKeychain key10 = xkey.getChild("1/0");
        bytes_t bytes;

        REQUIRE(Base58::CheckEncode(rootkey.extkey()) == rootpriv);
        REQUIRE(Base58::CheckEncode(rootkey.getPublic().extkey()) == rootpub);

        REQUIRE(Base58::CheckEncode(xkey.extkey()) == xpriv);
        REQUIRE(Base58::CheckEncode(xkey.getPublic().extkey()) == xpub);

        // key00
        bytes = key00.privkey();
        bytes.insert(bytes.begin(), 0x80);
        bytes.push_back(1);
        REQUIRE(Base58::CheckEncode(bytes) == privkey00);
        REQUIRE(key00.pubkey().getHex() == pubkey00);
        BRAddress braddr;
        memset(braddr.s, 0, sizeof(braddr));
        bytes = hash160(key00.pubkey());
        BRAddressFromHash160(braddr.s, sizeof(braddr), BITCOIN_ADDRESS_PARAMS, bytes.data());
        REQUIRE(address00 == std::string(braddr.s));

        // key01
        bytes = key01.privkey();
        bytes.insert(bytes.begin(), 0x80);
        bytes.push_back(1);
        REQUIRE(Base58::CheckEncode(bytes) == privkey01);
        REQUIRE(key01.pubkey().getHex() == pubkey01);
        memset(braddr.s, 0, sizeof(braddr));
        bytes = hash160(key01.pubkey());
        BRAddressFromHash160(braddr.s, sizeof(braddr), BITCOIN_ADDRESS_PARAMS, bytes.data());
        REQUIRE(address01 == std::string(braddr.s));

        // key10
        bytes = key10.privkey();
        bytes.insert(bytes.begin(), 0x80);
        bytes.push_back(1);
        REQUIRE(Base58::CheckEncode(bytes) == privkey10);
        REQUIRE(key10.pubkey().getHex() == pubkey10);
        memset(braddr.s, 0, sizeof(braddr));
        bytes = hash160(key10.pubkey());
        BRAddressFromHash160(braddr.s, sizeof(braddr), BITCOIN_ADDRESS_PARAMS, bytes.data());
        REQUIRE(address10 == std::string(braddr.s));
    }
}