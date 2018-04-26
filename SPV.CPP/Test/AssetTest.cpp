// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#define CATCH_CONFIG_MAIN

#include "catch.hpp"
#include "Payload/Asset.h"

using namespace Elastos::SDK;

TEST_CASE("Asset test", "[Asset]") {

	SECTION("Asset interface Test") {
		Asset asset;
		std::string name = "testName";
		std::string desc = "a test description";

		asset.setName(name);
		asset.setDescription(desc);
		asset.setAssetType(Asset::AssetType::Share);
		asset.setAssetRecordType(Asset::AssetRecordType::Balance);

		REQUIRE(asset.getName() == name);
		REQUIRE(asset.getDescription() == desc);
		REQUIRE(asset.getAssetType() == Asset::AssetType::Share);
		REQUIRE(asset.getAssetRecordType() == Asset::AssetRecordType::Balance);

		ByteStream byteStream;
		asset.Serialize(byteStream);

		byteStream.setPosition(0);

		Asset asset1;
		asset1.Deserialize(byteStream);

		REQUIRE(asset1.getName() == asset.getName());
		REQUIRE(asset1.getDescription() == asset.getDescription());
		REQUIRE(asset1.getAssetType() == asset.getAssetType());
		REQUIRE(asset1.getAssetRecordType() == asset.getAssetRecordType());
	}
}
