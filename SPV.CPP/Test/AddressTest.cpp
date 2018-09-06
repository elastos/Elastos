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
#include "Address.h"
#include "SDK/Transaction/Transaction.h"

using namespace Elastos::ElaWallet;

TEST_CASE( "Address test", "[Address]" )
{
	std::string backupPassword = "heropoon";
	nlohmann::json keystore = "{\"iv\":\"kha3Vctm00fA1hjOdP0YQA==\",\"v\":1,\"iter\":10000,\"ks\":128,\"ts\":64,\"mode\":\"ccm\",\"adata\":\"\",\"cipher\":\"aes\",\"salt\":\"J4aFQZSWvFo=\",\"ct\":\"mDVCy7fnV3RnehbDlafq4ALab43ZkVkzm5x2sHVjQscmQt4jWXUQQJP/A/+SYixFvj/fqZ4u+alT5/x08OAcWqnr/ft19ItfMB+bBd7vT1T93DygjRj9nRKMSyPu9rIarCU1fRPbxmNKF7dE7E2njtPcEZ+FAUKorj0P5AGvzWQM8QIlVbxz6TQTu120Ar1ukzlHc+xXTA8pw6ZlJcZQ6i7khI/hnVQzkuXr7R7y09yvU4LIDob/KmzsP7hSMha3hVUFLtvnTl5ElHlll7nywERMNYVt2khNSDfPACsFyaVlPHopKGsdp66IjJBemTVna6dH/n0HEIGFzsQ/dr//vsk9/BUoq7eLMart0StzaNm4R43q/3h0f4+SwqG0hk2aBGCG0QEgSor/S02ts50P80pbuGOQ7zARfmxkkEpm5z4iT0krPyJEsccitDtYBHmUEaSnT1zffkchtJsaZJFFx/9s5UpLkD2JrtYKvAM/txuBmacPFyxzUIdCoAH1n2z1rBvdd2hUPOXqds9IvHIEhx0aXR7fjRivS1JBpjqFHRKOEkGjkb+lN+HGtjooQxdOcmN1lVR5/oHETKqelupSosdNOiw2z2Ytpd5ovzX14PyGqnY8um4d/CzG7q5h8KDsGlrxPPvVEtl+WuAQ6zO/4YxLP+llukuY4b+vyT+v6c40qP5+acnkjb3zJ301EAo/PMIMf8IX2NssPVcd+XuRIospbvKMpTh05VjoXFr6rNu+/zho527C8JDe4tG4Fjs1yWOwQVM1Qk2ear2Rnwbf/lnYmU2TcO7zd69iRkiY7rhuLUTerbCjqVgZUTixri0HvsU0g8y5YZ5Thor7ghr/vV/YOywlgTAxknnr6/mtrVE1YXRP071lDsQ0734FK1L6oUfzoMBVlecUV8DUMQO6U4A34Y9THqHbhMBem05US7D0AQ8HtRuhg7c28LUOugxd6LTmbo4RNnre0bMCGNI6ZGhtBHXn1lMWWLe1bFy6tj9Uo6oVE/vg7pykEA/zJ3BhE4TJv8J3HIsAUEgVe5f7StDieODk8Y4uYAp2XLTi8QhiDt4dvaLHVkLz1Vjn3GQ15882bI6/H3/ezUTfxgTrdfGm8T7hFrYQx7QibYQRYgs+9UWWbHVKbqA5T0DA87yUFIcVgVv+CrubxrIr61vF7nu3EpLKCstqcKTM3LKjPxJlCKqCJAPDSgvpwjCWmI6fftZQ1SlCgIWAT3yGOy+FBaHr25+ZhT2iKh6UTUD3Yxr7mzo1/wzQGgpMqCLSKCmJUuHDzjdzjAu13RL9i+YA4Pk0FjkGhRLjhg/XT5trsLA2Hl8ZvUtZK0BXPPUYCXlFjLB6By1CkH4CPQJU69yV+MWO7NsJ59fQy5wl+5NS4Ks7XyNyAPacK3BkHuFlulGKCJyeQ9mLy9qKaXsdgUwThvVfgnRSl5qFoUvdZIHCgPESVRudP3lptlrTX1+ryuSLztNj8DE4zXUJRAgmua/QwVXrlBN6Y+I6tctjrSXONWtNVCp5HYmeaGbDo1w0oUnOQkpgP/4RjEfvDa5InRjJ6NxhuhOXfqWiYUNiHL9AXJPlulSnda/nOylOneJqxCFP4/dFF/0fXBon+pVf4HMc0M7KrRxwGq2fvk2OhIU/lo6ouZQCIfQuV0JM/PHEm/PK0YMbyQx+FvGiZzBtTiVMQyKUOxAgNDJMyJCWvhBm/+ktzIQgtpvBq9NI1pkF+GFd2AgIoArhA4DidpLBM477HXAntj+PGeSc4FvS6gk=\"}"_json;

	KeyStore keyStore;
	if (!keyStore.open(keystore, backupPassword)) {
		throw std::logic_error("Import key error.");
	}

	const ElaNewWalletJson &keyStoreJson = keyStore.json();
	const std::string mnemonicString = keyStoreJson.getMnemonic();

	Log::getLogger()->info("mnemonicString = {}", mnemonicString);
	
	Log::getLogger()->info("=====================private key");
	UInt512 seed;
	BRBIP39DeriveKey(&seed, mnemonicString.c_str(), "");

	BRKey key;
	BRBIP32PrivKey(&key, &seed, sizeof(seed), 0, 0);
	getPubKeyFromPrivKey(key.pubKey, &key.secret);
	
	BRMasterPubKey mp = BRBIP32MasterPubKey(&seed, sizeof(seed));
	BRKey key1;
	BRBIP32PubKey(key1.pubKey, 33, mp, 0, 0);

//	SECTION("Default constructor") {
//		Address address;
//		REQUIRE(address.getRaw() != nullptr);
//	}
//
//	SECTION("Constructor init with BRAddress") {
//		BRAddress raw ;
//		Address address(raw);
//		REQUIRE(address.getRaw() != nullptr);
//	}
//
//	SECTION("Create Address width String") {
//		std::string content = "ETFELUtMYwPpb96QrYaP6tBztEsUbQrytP";
//		Address address(content);
//		REQUIRE(address.stringify() == content);
//	}
//
//	SECTION("Create Address width ScriptPubKey") {
//		CHECK_THROWS_AS(Address::fromScriptPubKey(CMBlock(), ELA_STANDARD), std::logic_error);
//
//		std::string content = "ETFELUtMYwPpb96QrYaP6tBztEsUbQrytP";
//		Address myaddress(content);
//
//		CMBlock script = myaddress.getPubKeyScript();
//		boost::shared_ptr<Address> address1 = Address::fromScriptPubKey(script, ELA_STANDARD);
//		Address* address2 = address1.get();
//		REQUIRE(address2->toString() == content);
//
//		content = "XQd1DCi6H62NQdWZQhJCRnrPn7sF9CTjaU";
//		Address crossAddress(content);
//		script = crossAddress.getPubKeyScript();
//		address1 = Address::fromScriptPubKey(script, ELA_CROSSCHAIN);
//		REQUIRE(address1->toString() == content);
//
//	}
//
//	SECTION("Create Address with ScriptSignature") {
//
//		CHECK_THROWS_AS(Address::fromScriptSignature(CMBlock()), std::logic_error);
//		//todo completed width Signature Script ; now is no signature Script
//		std::string content = "ETFELUtMYwPpb96QrYaP6tBztEsUbQrytP";
//		Address myaddress(content);
//
//		CMBlock script = myaddress.getPubKeyScript();
//
//		boost::shared_ptr<Address> address1 = Address::fromScriptSignature(script);
//		REQUIRE(address1->toString() != content);
//	}
//
//	SECTION("Address Valid Test") {
//		std::string content = "ETFELUtMYwPpb96QrYaP6tBztEsUbQrytP";
//		Address address(content);
//		REQUIRE(address.isValid() == true);
//
//		content = "DTFELUtMYwPpb96QrYaP6tBztEsUbQrytE";
//		Address address1(content);
//		REQUIRE(address1.isValid() == false);
//	}

}
