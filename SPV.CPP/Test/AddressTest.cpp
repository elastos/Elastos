// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#define CATCH_CONFIG_MAIN

#include <BRTransaction.h>
#include <BRKey.h>

#include "catch.hpp"
#include "Address.h"
#include "Transaction.h"

using namespace Elastos::SDK;

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
		std::string content = "ETFELUtMYwPpb96QrYaP6tBztEsUbQrytP";
		Address myaddress(content);

		ByteData script = myaddress.getPubKeyScript();
		boost::shared_ptr<Address> address1 = Address::fromScriptPubKey(script);
		Address* address2 = address1.get();
		REQUIRE(address2->toString() == content);

	}

	SECTION("Create Address with ScriptSignature") {
		//todo completed width Signature Script ; now is no signature Script
		std::string content = "ETFELUtMYwPpb96QrYaP6tBztEsUbQrytP";
		Address myaddress(content);

		ByteData script = myaddress.getPubKeyScript();

		boost::shared_ptr<Address> address1 = Address::fromScriptSignature(script);
		Address* address2 = address1.get();
		REQUIRE(address2->toString() != content);
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
