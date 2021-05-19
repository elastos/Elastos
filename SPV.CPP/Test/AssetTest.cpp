// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#define CATCH_CONFIG_MAIN

#include <catch.hpp>
#include "TestHelper.h"
#include <Plugin/Transaction/Asset.h>
#include <Common/Log.h>

using namespace Elastos::ElaWallet;

TEST_CASE("Asset test", "[Asset]") {
	Log::registerMultiLogger();

	srand(time(nullptr));

	SECTION("Asset serialize and deserialize test") {
		Asset asset;

		asset.SetName(getRandString(50));
		asset.SetDescription(getRandString(80));
		asset.SetPrecision(getRandUInt8());
		asset.SetAssetType(Asset::AssetType::Share);
		asset.SetAssetRecordType(Asset::AssetRecordType::Balance);

		ByteStream stream;
		asset.Serialize(stream);

		Asset asset1;
		asset1.Deserialize(stream);

		REQUIRE(asset1.GetName() == asset.GetName());
		REQUIRE(asset1.GetDescription() == asset.GetDescription());
		REQUIRE(asset1.GetPrecision() == asset.GetPrecision());
		REQUIRE(asset1.GetAssetType() == asset.GetAssetType());
		REQUIRE(asset1.GetAssetRecordType() == asset.GetAssetRecordType());
	}

	SECTION("toJson fromJson test") {
		Asset asset;

		asset.SetName(getRandString(80));
		asset.SetDescription(getRandString(40));
		asset.SetPrecision(getRandUInt8());
		asset.SetAssetType(Asset::AssetType::Share);
		asset.SetAssetRecordType(Asset::AssetRecordType::Balance);

		nlohmann::json j = asset.ToJson();

		Asset asset1;
		asset1.FromJson(j);

		REQUIRE(asset1.GetName() == asset.GetName());
		REQUIRE(asset1.GetDescription() == asset.GetDescription());
		REQUIRE(asset1.GetPrecision() == asset.GetPrecision());
		REQUIRE(asset1.GetAssetType() == asset.GetAssetType());
		REQUIRE(asset1.GetAssetRecordType() == asset.GetAssetRecordType());
	}

	SECTION("operator= test") {
		Asset a1, a2;

		a1.SetName(getRandString(100));
		a1.SetDescription(getRandString(70));
		a1.SetPrecision(getRandUInt8());
		a1.SetAssetType(Asset::AssetType::Token);
		a1.SetAssetRecordType(Asset::AssetRecordType::Unspent);

		a2 = a1;

		REQUIRE(a1.GetName()            == a2.GetName());
		REQUIRE(a1.GetDescription()     == a2.GetDescription());
		REQUIRE(a1.GetPrecision()       == a2.GetPrecision());
		REQUIRE(a1.GetAssetType()       == a2.GetAssetType());
		REQUIRE(a1.GetAssetRecordType() == a2.GetAssetRecordType());
	}

}
