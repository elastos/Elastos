// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#define CATCH_CONFIG_MAIN

#include "catch.hpp"

#include <SDK/Common/Utils.h>
#include <SDK/Account/Account.h>

#include <Core/BRAddress.h>
#include <SDK/Common/Log.h>

using namespace Elastos::ElaWallet;

TEST_CASE("Derive id public and private key", "[Id agent]") {
	Log::registerMultiLogger();
	std::string phrasePasswd = "phrasePassword";
	std::string payPasswd = "payPassword";
	std::string chainId = "chainid";
	std::string mnemonic = "you train view salon cancel impulse phrase oxygen sport crack peasant observe";
	std::string language = "english";

	SECTION("Address derived from public and private key should be same") {
		LocalStorePtr localstore(new LocalStore("Data/1", mnemonic, phrasePasswd, false, payPasswd));
		Account account(localstore);

		const HDKeychain &mpk = account.MasterPubKey();

		bytes_t pubKey = mpk.getChild("0/0").pubkey();

		std::string pubId = Address(PrefixIDChain, pubKey).String();

		pubKey = account.RootKey(payPasswd).getChild("44'/0'/0'/0/0").pubkey();

		std::string privId = Address(PrefixIDChain, pubKey).String();

		REQUIRE(pubId == privId);
	}
}

TEST_CASE("Derive public and private key", "[HD wallet]") {
	SECTION("Address") {
		int coinIndex = 0;
		std::string payPasswd = "payPassword";
		std::string mnemonic = "flat universe quantum uniform emerge blame lemon detail april sting aerobic disease";
		LocalStorePtr localstore(new LocalStore("Data/1", mnemonic, "", false, payPasswd));
		for (size_t i = 0; i < 1; ++i) {
			AccountPtr account(new Account(localstore));
			bytes_t pubkey = account->RootKey(payPasswd).getChild("44'/0'/0'/0/0").pubkey();
			REQUIRE("EJuHg2CdT9a9bqdKUAtbrAn6DGwXtKA6uh" == Address(PrefixStandard, pubkey).String());
		}
	}
}
