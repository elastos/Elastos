// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#define CATCH_CONFIG_MAIN

#include "catch.hpp"
#include "TestHelper.h"
#include <SDK/Plugin/Transaction/Asset.h>

using namespace Elastos::ElaWallet;

TEST_CASE("Asset test", "[Asset]") {

	srand(time(nullptr));

	SECTION("Asset serialize and deserialize test") {
		Asset asset;

		asset.setName(getRandString(50));
		asset.setDescription(getRandString(80));
		asset.setPrecision(getRandUInt8());
		asset.setAssetType(Asset::AssetType::Share);
		asset.setAssetRecordType(Asset::AssetRecordType::Balance);

		ByteStream stream;
		asset.Serialize(stream);

		stream.setPosition(0);

		Asset asset1;
		asset1.Deserialize(stream);

		REQUIRE(asset1.getName() == asset.getName());
		REQUIRE(asset1.getDescription() == asset.getDescription());
		REQUIRE(asset1.getPrecision() == asset.getPrecision());
		REQUIRE(asset1.getAssetType() == asset.getAssetType());
		REQUIRE(asset1.getAssetRecordType() == asset.getAssetRecordType());
	}

	SECTION("toJson fromJson test") {
		Asset asset;

		asset.setName(getRandString(80));
		asset.setDescription(getRandString(40));
		asset.setPrecision(getRandUInt8());
		asset.setAssetType(Asset::AssetType::Share);
		asset.setAssetRecordType(Asset::AssetRecordType::Balance);

		nlohmann::json j = asset.toJson();

		Asset asset1;
		asset1.fromJson(j);

		REQUIRE(asset1.getName() == asset.getName());
		REQUIRE(asset1.getDescription() == asset.getDescription());
		REQUIRE(asset1.getPrecision() == asset.getPrecision());
		REQUIRE(asset1.getAssetType() == asset.getAssetType());
		REQUIRE(asset1.getAssetRecordType() == asset.getAssetRecordType());
	}

	SECTION("operator= test") {
		Asset a1, a2;

		a1.setName(getRandString(100));
		a1.setDescription(getRandString(70));
		a1.setPrecision(getRandUInt8());
		a1.setAssetType(Asset::AssetType::Token);
		a1.setAssetRecordType(Asset::AssetRecordType::Unspent);

		a2 = a1;

		REQUIRE(a1.getName()            == a2.getName());
		REQUIRE(a1.getDescription()     == a2.getDescription());
		REQUIRE(a1.getPrecision()       == a2.getPrecision());
		REQUIRE(a1.getAssetType()       == a2.getAssetType());
		REQUIRE(a1.getAssetRecordType() == a2.getAssetRecordType());
	}

}
