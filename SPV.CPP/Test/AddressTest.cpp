// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#define CATCH_CONFIG_MAIN

#include <BRTransaction.h>
#include <BRKey.h>
#include <SDK/KeyStore/ElaNewWalletJson.h>
#include <SDK/KeyStore/KeyStore.h>
#include <SDK/Common/Log.h>
#include <Core/BRBIP39Mnemonic.h>
#include <Core/BRBIP32Sequence.h>
#include <Core/BRKey.h>

#include "catch.hpp"
#include "SDK/Base/Address.h"
#include "SDK/Transaction/Transaction.h"

using namespace Elastos::ElaWallet;

TEST_CASE( "Address test", "[Address]" )
{
	SECTION("Default constructor") {
		Address address;
		REQUIRE(address.getRaw() != nullptr);
	}

	SECTION("Constructor init with BRAddress") {
		BRAddress raw ;
		Address address(raw);
		REQUIRE(address.getRaw() != nullptr);
	}

	SECTION("Create Address width String") {
		std::string content = "ETFELUtMYwPpb96QrYaP6tBztEsUbQrytP";
		Address address(content);
		REQUIRE(address.stringify() == content);
	}

	SECTION("Create Address width ScriptPubKey") {
		CHECK_THROWS_AS(Address::fromScriptPubKey(CMBlock(), ELA_STANDARD), std::logic_error);

		std::string content = "ETFELUtMYwPpb96QrYaP6tBztEsUbQrytP";
		Address myaddress(content);

		CMBlock script = myaddress.getPubKeyScript();
		boost::shared_ptr<Address> address1 = Address::fromScriptPubKey(script, ELA_STANDARD);
		Address* address2 = address1.get();
		REQUIRE(address2->toString() == content);

		content = "XQd1DCi6H62NQdWZQhJCRnrPn7sF9CTjaU";
		Address crossAddress(content);
		script = crossAddress.getPubKeyScript();
		address1 = Address::fromScriptPubKey(script, ELA_CROSSCHAIN);
		REQUIRE(address1->toString() == content);

	}

	SECTION("Create Address with ScriptSignature") {

		CHECK_THROWS_AS(Address::fromScriptSignature(CMBlock()), std::logic_error);
		//todo completed width Signature Script ; now is no signature Script
		std::string content = "ETFELUtMYwPpb96QrYaP6tBztEsUbQrytP";
		Address myaddress(content);

		CMBlock script = myaddress.getPubKeyScript();

		boost::shared_ptr<Address> address1 = Address::fromScriptSignature(script);
		REQUIRE(address1->toString() != content);
	}

	SECTION("Address Valid Test") {
		std::string content = "ETFELUtMYwPpb96QrYaP6tBztEsUbQrytP";
		Address address(content);
		REQUIRE(address.isValid() == true);

		content = "DTFELUtMYwPpb96QrYaP6tBztEsUbQrytE";
		Address address1(content);
		REQUIRE(address1.isValid() == false);
	}

}
