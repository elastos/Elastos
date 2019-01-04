// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#define CATCH_CONFIG_MAIN

#include "catch.hpp"
#include "TestHelper.h"
#include <SDK/Plugin/Transaction/Asset.h>
#include <SDK/Plugin/Transaction/Payload/PayloadRegisterAsset.h>

using namespace Elastos::ElaWallet;

static void initAsset(Asset &asset) {
	asset.setName(getRandString(20));
	asset.setDescription(getRandString(50));
	asset.setPrecision(getRandUInt8());
	asset.setAssetType(Asset::AssetType::Share);
	asset.setAssetRecordType(Asset::AssetRecordType::Balance);
}

TEST_CASE("PayloadRegisterAsset test", "[PayloadRegisterAsset]") {

	SECTION("serialize and deserialize") {
		PayloadRegisterAsset p1, p2;

		Asset asset;
		initAsset(asset);
		p1.setAsset(asset);
		p1.setAmount(getRandUInt64());
		p1.setController(getRandUInt168());

		ByteStream stream;

		p1.Serialize(stream, 0);

		stream.setPosition(0);
		REQUIRE(p2.Deserialize(stream, 0));

		Asset asset1 = p1.getAsset();
		Asset asset2 = p2.getAsset();
		REQUIRE(asset1.getName() == asset2.getName());
		REQUIRE(asset1.getDescription() == asset2.getDescription());
		REQUIRE(asset1.getPrecision() == asset2.getPrecision());
		REQUIRE(asset1.getAssetType() == asset2.getAssetType());
		REQUIRE(asset1.getAssetRecordType() == asset2.getAssetRecordType());
		REQUIRE(UInt256Eq(&asset1.GetHash(), &asset2.GetHash()));

		REQUIRE(p1.getAmount() == p2.getAmount());
		REQUIRE(UInt168Eq(&p1.getController(), &p2.getController()));
	}

	SECTION("toJson fromJson test") {
		PayloadRegisterAsset p1, p2;

		Asset asset;
		initAsset(asset);
		p1.setAsset(asset);
		p1.setAmount(getRandUInt64());
		p1.setController(getRandUInt168());

		ByteStream stream;

		nlohmann::json p1Json = p1.toJson();

		p2.fromJson(p1Json);

		Asset asset1 = p1.getAsset();
		Asset asset2 = p2.getAsset();
		REQUIRE(asset1.getName() == asset2.getName());
		REQUIRE(asset1.getDescription() == asset2.getDescription());
		REQUIRE(asset1.getPrecision() == asset2.getPrecision());
		REQUIRE(asset1.getAssetType() == asset2.getAssetType());
		REQUIRE(asset1.getAssetRecordType() == asset2.getAssetRecordType());
		REQUIRE(UInt256Eq(&asset1.GetHash(), &asset2.GetHash()));

		REQUIRE(p1.getAmount() == p2.getAmount());
		REQUIRE(UInt168Eq(&p1.getController(), &p2.getController()));
	}
}
