// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#define CATCH_CONFIG_MAIN

#include "catch.hpp"
#include "Payload/PayloadRegisterAsset.h"

using namespace Elastos::SDK;

TEST_CASE("PayloadRegisterAsset test", "[PayloadRegisterAsset]") {

	SECTION("PayloadRegisterAsset interface test") {
		PayloadRegisterAsset payloadRegisterAsset;

		Asset asset;
		std::string name = "testName";
		std::string desc = "a test description";

		asset.setName(name);
		asset.setDescription(desc);
		asset.setAssetType(Asset::AssetType::Share);
		asset.setAssetRecordType(Asset::AssetRecordType::Balance);

		payloadRegisterAsset.setAsset(asset);
		payloadRegisterAsset.setAmount(888);
		UInt168 hash = *(UInt168 *) "\x21\xc2\xe2\x51\x72\xcb\x15\x19\x3c\xb1\xc6"
				"\xd4\x8f\x60\x7d\x42\xc1\xd2\xa2\x15\x16";
		payloadRegisterAsset.setController(hash);

		Asset asset1 = payloadRegisterAsset.getAsset();
		REQUIRE(asset1.getName() == name);
		REQUIRE(asset1.getDescription() == desc);
		REQUIRE(asset1.getAssetType() == Asset::AssetType::Share);
		REQUIRE(asset1.getAssetRecordType() == Asset::AssetRecordType::Balance);
		REQUIRE(payloadRegisterAsset.getAmount() == 888);

		const UInt168 &controller = payloadRegisterAsset.getController();
		int res = UInt168Eq(&hash, &controller);
		REQUIRE(res == 1);
	}

	SECTION("toJson fromJson test") {
		PayloadRegisterAsset payloadRegisterAsset;

		Asset asset;
		std::string name = "testName";
		std::string desc = "a test description";

		asset.setName(name);
		asset.setDescription(desc);
		asset.setAssetType(Asset::AssetType::Share);
		asset.setAssetRecordType(Asset::AssetRecordType::Balance);

		payloadRegisterAsset.setAsset(asset);
		payloadRegisterAsset.setAmount(888);
		UInt168 hash = *(UInt168 *) "\x21\xc2\xe2\x51\x72\xcb\x15\x19\x3c\xb1\xc6"
				"\xd4\x8f\x60\x7d\x42\xc1\xd2\xa2\x15\x16";
		payloadRegisterAsset.setController(hash);

		nlohmann::json jsonData = payloadRegisterAsset.toJson();

		PayloadRegisterAsset payloadRegisterAsset1;
		payloadRegisterAsset1.fromJson(jsonData);

		Asset asset1 = payloadRegisterAsset1.getAsset();
		REQUIRE(asset1.getName() == name);
		REQUIRE(asset1.getDescription() == desc);
		REQUIRE(asset1.getAssetType() == Asset::AssetType::Share);
		REQUIRE(asset1.getAssetRecordType() == Asset::AssetRecordType::Balance);
		REQUIRE(payloadRegisterAsset.getAmount() == 888);

		const UInt168 &controller = payloadRegisterAsset1.getController();
		int res = UInt168Eq(&hash, &controller);
		REQUIRE(res == 1);
	}
}