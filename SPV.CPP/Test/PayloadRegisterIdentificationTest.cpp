// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#define CATCH_CONFIG_MAIN

#include <catch.hpp>
#include <Plugin/Transaction/Payload/RegisterIdentification.h>
#include <Common/Utils.h>
#include <Common/Log.h>

using namespace Elastos::ElaWallet;

const std::string ID = "ij8rfb6A4Ri7c5CRE1nDVdVCUMuUxkk2c6";
const std::string Content1_Path1 = "kyc/person/identityCard";
const std::string Content1_Proof1 = "\"signature\":\"30450220499a5de3f84e7e919c26b6a8543fd24129634c65ee4d38fe2e3386ec8a5dae57022100b7679de8d181a454e2def8f55de423e9e15bebcde5c58e871d20aa0d91162ff6\",\"notary\":\"COOIX\"";
const std::string Content1_DataHash1 = "bd117820c4cf30b0ad9ce68fe92b0117ca41ac2b6a49235fabd793fc3a9413c0";
const std::string Content2_Path2 = "kyc/person/phone";
const std::string Content2_Proof2 = "\"signature\":\"3046022100e888040388d0f569183eea8e6f608e00f38a0ed38e1d2f227a8638ef3efd8f6c022100ca1c2ec20a9a933c072f6e4e6b9660a5aa1bcace8af384abaa30941010d4a9cf\",\"notary\":\"COOIX\"";
const std::string Content2_DataHash2 = "cf985a76d1eef80aa7aa4ce144edad2c042c217ecabb0f86cd91bd4dae3b7215";

TEST_CASE("RegisterIdentification fromJson test", "[fromJson&toJson]") {
	Log::registerMultiLogger();

	SECTION("Parse from existing json") {
		nlohmann::json rawJson = "{\n"
								 "    \"Id\": \"ij8rfb6A4Ri7c5CRE1nDVdVCUMuUxkk2c6\",\n"
								 "    \"Contents\": [\n"
								 "        {\n"
								 "            \"Path\": \"kyc/person/identityCard\",\n"
								 "            \"Values\": [\n"
								 "                {\n"
								 "                    \"Proof\": \"\\\"signature\\\":\\\"30450220499a5de3f84e7e919c26b6a8543fd24129634c65ee4d38fe2e3386ec8a5dae57022100b7679de8d181a454e2def8f55de423e9e15bebcde5c58e871d20aa0d91162ff6\\\",\\\"notary\\\":\\\"COOIX\\\"\",\n"
								 "                    \"DataHash\": \"bd117820c4cf30b0ad9ce68fe92b0117ca41ac2b6a49235fabd793fc3a9413c0\"\n"
								 "                },\n"
								 "                {\n"
								 "                    \"Proof\": \"\\\"signature\\\":\\\"30450220499a5de3f84e7e919c26b6a8543fd24129634c65ee4d38fe2e3386ec8a5dae57022100b7679de8d181a454e2def8f55de423e9e15bebcde5c58e871d20aa0d91162ff6\\\",\\\"notary\\\":\\\"COOIX\\\"\",\n"
								 "                    \"DataHash\": \"bd117820c4cf30b0ad9ce68fe92b0117ca41ac2b6a49235fabd793fc3a9413c0\"\n"
								 "                }\n"
								 "            ]\n"
								 "        },\n"
								 "        {\n"
								 "            \"Path\": \"kyc/person/phone\",\n"
								 "            \"Values\": [\n"
								 "                {\n"
								 "                    \"Proof\": \"\\\"signature\\\":\\\"3046022100e888040388d0f569183eea8e6f608e00f38a0ed38e1d2f227a8638ef3efd8f6c022100ca1c2ec20a9a933c072f6e4e6b9660a5aa1bcace8af384abaa30941010d4a9cf\\\",\\\"notary\\\":\\\"COOIX\\\"\",\n"
								 "                    \"DataHash\": \"cf985a76d1eef80aa7aa4ce144edad2c042c217ecabb0f86cd91bd4dae3b7215\"\n"
								 "                }\n"
								 "            ]\n"
								 "        }\n"
								 "    ]\n"
								 "}"_json;

		std::string str = rawJson.dump();

		RegisterIdentification payload;
		REQUIRE_NOTHROW(payload.FromJson(rawJson, 0));
	}

	SECTION("Convert from and to json") {

		RegisterIdentification payload;
		payload.SetID(ID);

		RegisterIdentification::SignContent content;
		content.Path = Content1_Path1;
		RegisterIdentification::ValueItem item;
		item.Proof = Content1_Proof1;
		item.DataHash = uint256(Content1_DataHash1);
		content.Values.push_back(item);
		payload.AddContent(content);

		RegisterIdentification::SignContent content2;
		content2.Path = Content2_Path2;
		RegisterIdentification::ValueItem item2;
		item2.Proof = Content2_Proof2;
		item2.DataHash = uint256(Content2_DataHash2);
		content2.Values.push_back(item2);
		payload.AddContent(content2);

		nlohmann::json j = payload.ToJson(0);

		RegisterIdentification payload2;
		payload2.FromJson(j, 0);

		REQUIRE(payload.GetID() == payload2.GetID());

		REQUIRE(payload.GetPath(0) == payload2.GetPath(0));
		REQUIRE(payload.GetProof(0, 0) == payload2.GetProof(0, 0));
		REQUIRE(payload.GetDataHash(0, 0) == payload2.GetDataHash(0, 0));

		REQUIRE(payload.GetPath(1) == payload2.GetPath(1));
		REQUIRE(payload.GetProof(1, 0) == payload2.GetProof(1, 0));
		REQUIRE(payload.GetDataHash(1, 0) == payload2.GetDataHash(1, 0));
	}
}

TEST_CASE("RegisterIdentification serialize and deserialize test", "[Serialize&Deserialize]") {

	SECTION("Deserialize from a serialized payload") {

		RegisterIdentification payload;
		payload.SetID(ID);

		RegisterIdentification::SignContent content;
		content.Path = Content1_Path1;
		RegisterIdentification::ValueItem item;
		item.Proof = Content1_Proof1;
		item.DataHash = uint256(Content1_DataHash1);
		content.Values.push_back(item);
		payload.AddContent(content);

		RegisterIdentification::SignContent content2;
		content2.Path = Content2_Path2;
		RegisterIdentification::ValueItem item2;
		item2.Proof = Content2_Proof2;
		item2.DataHash = uint256(Content2_DataHash2);
		content2.Values.push_back(item2);
		payload.AddContent(content2);

		ByteStream stream;
		payload.Serialize(stream, 0);

		RegisterIdentification payload2;
		payload2.Deserialize(stream, 0);

		REQUIRE(payload.GetID() == payload2.GetID());

		REQUIRE(payload.GetPath(0) == payload2.GetPath(0));
		REQUIRE(payload.GetProof(0, 0) == payload2.GetProof(0, 0));
		REQUIRE(payload.GetDataHash(0, 0) == payload2.GetDataHash(0, 0));

		REQUIRE(payload.GetPath(1) == payload2.GetPath(1));
		REQUIRE(payload.GetProof(1, 0) == payload2.GetProof(1, 0));
		REQUIRE(payload.GetDataHash(1, 0) == payload2.GetDataHash(1, 0));
	}
}
