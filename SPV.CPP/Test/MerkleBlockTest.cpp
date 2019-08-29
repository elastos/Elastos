// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#define CATCH_CONFIG_MAIN

#include <SDK/Common/ByteStream.h>
#include <SDK/Common/Utils.h>
#include <SDK/Common/Log.h>
#include <SDK/Plugin/Interface/IMerkleBlock.h>
#include <SDK/Plugin/Registry.h>
#include <SDK/Plugin/Block/SidechainMerkleBlock.h>
#include <SDK/Plugin/Block/MerkleBlock.h>
#include <SDK/Plugin/ELAPlugin.h>
#include <SDK/Plugin/IDPlugin.h>

#include "catch.hpp"
#include "TestHelper.h"

using namespace Elastos::ElaWallet;

TEST_CASE("MerkleBlock construct test", "[MerkleBlock]") {
	Log::registerMultiLogger();

#ifndef BUILD_SHARED_LIBS
	Log::info("Registering plugin ...");
	REGISTER_MERKLEBLOCKPLUGIN(ELA, getELAPluginComponent);
	REGISTER_MERKLEBLOCKPLUGIN(SideStandard, getIDPluginComponent);
#endif

	srand(time(nullptr));

	SECTION("serialize and deserialize") {
		MerkleBlockPtr merkleBlock = Registry::Instance()->CreateMerkleBlock("ELA");
		REQUIRE(merkleBlock != nullptr);
		setMerkleBlockValues(static_cast<MerkleBlock *>(merkleBlock.get()));

		ByteStream stream;
		merkleBlock->Serialize(stream);

		MerkleBlock mb;
		mb.Deserialize(stream);

		verifyELAMerkleBlock(static_cast<const MerkleBlock &>(*merkleBlock), mb);
	}
}
