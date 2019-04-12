// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#define CATCH_CONFIG_MAIN

#include <catch.hpp>
#include "TestHelper.h"

#include <Plugin/Block/AuxPow.h>
#include <Common/Log.h>

using namespace Elastos::ElaWallet;

TEST_CASE("AuxPow test", "[AuxPow]") {
	Log::registerMultiLogger();

	srand((unsigned int) time(nullptr));

	SECTION("Serialize and deserialize") {

		AuxPow auxPow = createDummyAuxPow();

		ByteStream byteStream;
		auxPow.Serialize(byteStream);

		AuxPow auxPowVerify;
		auxPowVerify.Deserialize(byteStream);

		verrifyAuxPowEqual(auxPow, auxPowVerify, false);
	}
}
