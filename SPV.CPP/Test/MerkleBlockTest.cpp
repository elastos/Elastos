// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#define CATCH_CONFIG_MAIN

#include <Common/ByteStream.h>
#include <Common/Utils.h>
#include <Common/Log.h>
#include <Plugin/Interface/IMerkleBlock.h>
#include <Plugin/Registry.h>
#include <Plugin/Block/SidechainMerkleBlock.h>
#include <Plugin/Block/MerkleBlock.h>
#include <Plugin/ELAPlugin.h>
#include <Plugin/IDPlugin.h>
#include <Plugin/TokenPlugin.h>

#include <catch.hpp>
#include "TestHelper.h"

using namespace Elastos::ElaWallet;

TEST_CASE("MerkleBlock construct test", "[MerkleBlock]") {
	Log::registerMultiLogger();

#ifdef SPV_ENABLE_STATIC
	Log::info("Registering plugin ...");
	REGISTER_MERKLEBLOCKPLUGIN(ELA, getELAPluginComponent);
	REGISTER_MERKLEBLOCKPLUGIN(IDChain, getIDPluginComponent);
	REGISTER_MERKLEBLOCKPLUGIN(TokenChain, getTokenPluginComponent);
#endif

	srand(time(nullptr));

	SECTION("serialize and deserialize") {
		MerkleBlockPtr merkleBlock = Registry::Instance()->CreateMerkleBlock("ELA");
		REQUIRE(merkleBlock != nullptr);
		setMerkleBlockValues(static_cast<MerkleBlock *>(merkleBlock.get()));

		ByteStream stream;
		merkleBlock->Serialize(stream, MERKLEBLOCK_VERSION_1);

		MerkleBlock mb;
		mb.Deserialize(stream, MERKLEBLOCK_VERSION_1);

		verifyELAMerkleBlock(static_cast<const MerkleBlock &>(*merkleBlock), mb);
	}
}
