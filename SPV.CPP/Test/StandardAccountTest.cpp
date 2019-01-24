// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#define CATCH_CONFIG_MAIN

#include "catch.hpp"

#include <SDK/Common/Utils.h>
#include <SDK/Account/StandardAccount.h>
#include <SDK/BIPs/BIP32Sequence.h>

#include <Core/BRAddress.h>

using namespace Elastos::ElaWallet;

TEST_CASE("Derive id public and private key", "[Id agent]") {
	std::string phrasePassword = "phrasePassword";
	std::string payPassword = "payPassword";
	std::string chainId = "chainid";
	std::string mnemonic = "you train view salon cancel impulse phrase oxygen sport crack peasant observe";
	std::string language = "english";

	SECTION("Address derived from public and private key should be same") {
		StandardAccount account("Data", mnemonic, phrasePassword, payPassword);

		const MasterPubKey &mpk = account.GetIDMasterPubKey();

		CMBlock pubKey = BIP32Sequence::PubKey(mpk, 1, 0);

		Key key;
		key.SetPubKey(pubKey);
		std::string pubId = key.GetAddress(PrefixIDChain);

		UInt512 seed = account.DeriveSeed(payPassword);
		UInt256 chainCode;
		key = BIP32Sequence::PrivKeyPath(&seed, sizeof(seed), chainCode, 3, 0 | BIP32_HARD, 1, 0);
		std::string privId = key.GetAddress(PrefixIDChain);

		REQUIRE(pubId == privId);
	}
}

TEST_CASE("Derive public and private key", "[HD wallet]") {
	SECTION("Address") {
		int coinIndex = 0;
		std::string payPassword = "payPassword";
		std::string mnemonic = "flat universe quantum uniform emerge blame lemon detail april sting aerobic disease";
		for (size_t i = 0; i < 1; ++i) {
			StandardAccount account("Data", mnemonic, "", "payPassword");
			UInt512 seed = account.DeriveSeed(payPassword);
			UInt256 chainCode;
			Key key = BIP32Sequence::PrivKeyPath(&seed, sizeof(seed), chainCode, 5, 44 | BIP32_HARD,
												 coinIndex | BIP32_HARD, 0 | BIP32_HARD, 0, 0);
			var_clean(&chainCode);
			var_clean(&seed);
			REQUIRE("EJuHg2CdT9a9bqdKUAtbrAn6DGwXtKA6uh" == key.GetAddress(PrefixStandard));
		}
	}
}
