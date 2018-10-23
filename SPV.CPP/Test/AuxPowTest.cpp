// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#define CATCH_CONFIG_MAIN

#include <Core/BRMerkleBlock.h>
#include <Core/BRTransaction.h>
#include "catch.hpp"
#include "SDK/Plugin/Block/AuxPow.h"
#include "Log.h"
#include "TestHelper.h"

using namespace Elastos::ElaWallet;

TEST_CASE("AuxPow test", "[AuxPow]") {

	srand((unsigned int) time(nullptr));

	SECTION("To json and from json") {
		AuxPow auxPow = createDummyAuxPow();

		nlohmann::json auxPowJson = auxPow.toJson();

		//SPDLOG_DEBUG(Log::getLogger(),"auxPow json = {}", auxPowJson.dump());

		/* from json and verify */
		AuxPow auxPowVerify;
		auxPowVerify.fromJson(auxPowJson);

		verrifyAuxPowEqual(auxPow, auxPowVerify);
	}
	SECTION("Serialize and deserialize") {

		AuxPow auxPow = createDummyAuxPow();

		ByteStream byteStream;
		auxPow.Serialize(byteStream);

		AuxPow auxPowVerify;
		byteStream.setPosition(0);
		auxPowVerify.Deserialize(byteStream);

		verrifyAuxPowEqual(auxPow, auxPowVerify, false);
	}
}
