// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#define CATCH_CONFIG_MAIN

#include <catch.hpp>
#include "TestHelper.h"
#include <Plugin/Transaction/Asset.h>
#include <Plugin/Transaction/Payload/RegisterAsset.h>
#include <Common/Log.h>

using namespace Elastos::ElaWallet;

static void initAsset(AssetPtr &asset) {
	asset->SetName(getRandString(20));
	asset->SetDescription(getRandString(50));
	asset->SetPrecision(getRandUInt8());
	asset->SetAssetType(Asset::AssetType::Share);
	asset->SetAssetRecordType(Asset::AssetRecordType::Balance);
	asset->SetHash(uint256());
	asset->GetHash();
}

TEST_CASE("RegisterAsset test", "[RegisterAsset]") {
	Log::registerMultiLogger();

	SECTION("serialize and deserialize") {
		RegisterAsset p1, p2;

		AssetPtr asset(new Asset());
		initAsset(asset);
		p1.SetAsset(asset);
		p1.SetAmount(getRandUInt64());
		p1.SetController(getRandUInt168());

		ByteStream stream;

		p1.Serialize(stream, 0);

		REQUIRE(p2.Deserialize(stream, 0));

		const AssetPtr &asset1 = p1.GetAsset();
		const AssetPtr &asset2 = p2.GetAsset();
		REQUIRE(asset1->GetName()            == asset2->GetName());
		REQUIRE(asset1->GetDescription()     == asset2->GetDescription());
		REQUIRE(asset1->GetPrecision()       == asset2->GetPrecision());
		REQUIRE(asset1->GetAssetType()       == asset2->GetAssetType());
		REQUIRE(asset1->GetAssetRecordType() == asset2->GetAssetRecordType());
		REQUIRE(asset1->GetHash()            == asset2->GetHash());

		REQUIRE(p1.GetAmount() == p2.GetAmount());
		REQUIRE(p1.GetController() == p2.GetController());
	}

	SECTION("toJson fromJson test") {
		RegisterAsset p1, p2;

		AssetPtr asset(new Asset());
		initAsset(asset);
		p1.SetAsset(asset);
		p1.SetAmount(getRandUInt64());
		p1.SetController(getRandUInt168());

		ByteStream stream;

		nlohmann::json p1Json = p1.ToJson(0);

		p2.FromJson(p1Json, 0);

		AssetPtr asset1 = p1.GetAsset();
		AssetPtr asset2 = p2.GetAsset();
		REQUIRE(asset1->GetName()            == asset2->GetName());
		REQUIRE(asset1->GetDescription()     == asset2->GetDescription());
		REQUIRE(asset1->GetPrecision()       == asset2->GetPrecision());
		REQUIRE(asset1->GetAssetType()       == asset2->GetAssetType());
		REQUIRE(asset1->GetAssetRecordType() == asset2->GetAssetRecordType());
		REQUIRE(asset1->GetHash()            == asset2->GetHash());

		REQUIRE(p1.GetAmount() == p2.GetAmount());
		REQUIRE(p1.GetController() == p2.GetController());
	}
}
