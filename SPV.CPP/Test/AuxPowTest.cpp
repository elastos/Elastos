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

	SECTION("To json and from json") {
		AuxPow auxPow = createDummyAuxPow();

		nlohmann::json auxPowJson = auxPow.ToJson();

		//SPDLOG_DEBUG(Log::getLogger(),"auxPow json = {}", auxPowJson.dump());

		/* from json and verify */
		AuxPow auxPowVerify;
		auxPowVerify.FromJson(auxPowJson);

		verrifyAuxPowEqual(auxPow, auxPowVerify);
	}
	SECTION("Serialize and deserialize") {

		AuxPow auxPow = createDummyAuxPow();

		ByteStream byteStream;
		auxPow.Serialize(byteStream);

		AuxPow auxPowVerify;
		byteStream.SetPosition(0);
		auxPowVerify.Deserialize(byteStream);

		verrifyAuxPowEqual(auxPow, auxPowVerify, false);
	}
}
