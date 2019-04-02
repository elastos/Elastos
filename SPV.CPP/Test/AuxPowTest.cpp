// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#define CATCH_CONFIG_MAIN

#include "catch.hpp"
#include "TestHelper.h"

#include <Core/BRMerkleBlock.h>
#include <Core/BRTransaction.h>
#include <SDK/Plugin/Block/AuxPow.h>
#include <SDK/Common/Log.h>

using namespace Elastos::ElaWallet;

TEST_CASE("AuxPow test", "[AuxPow]") {

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
