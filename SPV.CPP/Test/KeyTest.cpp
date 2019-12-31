// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#define CATCH_CONFIG_MAIN

#include <catch.hpp>

#include "TestHelper.h"

#include <Common/Log.h>
#include <Common/Utils.h>
#include <WalletCore/Mnemonic.h>
#include <WalletCore/BIP39.h>
#include <WalletCore/Key.h>

#include <boost/shared_ptr.hpp>
#include <boost/filesystem/path.hpp>

using namespace Elastos::ElaWallet;

TEST_CASE("Key sign pressure test", "[KeySign]") {
	Log::registerMultiLogger();

#ifdef PRESSURE_TEST
#define LOOP_COUNT 10000
#else
#define LOOP_COUNT 1
#endif
	std::string phrase = "闲 齿 兰 丹 请 毛 训 胁 浇 摄 县 诉";
	std::string phrasePasswd = "";

	uint512 seed = BIP39::DeriveSeed(phrase, phrasePasswd);

	HDKeychain child = HDKeychain(HDSeed(seed.bytes()).getExtendedKey(true)).getChild("44'/0'/0'/0/0");

	Key key1, key2;

	key1.SetPrvKey(child.privkey());
	key2.SetPubKey(child.pubkey());

	for (int i = 0; i < LOOP_COUNT; ++i) {
		std::string message = getRandString(120);

		bytes_t signedData = key1.Sign(message);
		REQUIRE(key2.Verify(message, signedData));
	}
}

