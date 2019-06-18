// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#define CATCH_CONFIG_MAIN

#include <catch.hpp>
#include "TestHelper.h"

#include <SDK/Plugin/Transaction/TransactionOutput.h>
#include <SDK/Plugin/Transaction/Transaction.h>
#include <SDK/Plugin/Transaction/Payload/PayloadCoinBase.h>
#include <SDK/Plugin/Transaction/Attribute.h>
#include <SDK/Common/Utils.h>
#include <SDK/Common/Log.h>
#include <Core/BRTransaction.h>
#include <Core/BRTransaction.h>

using namespace Elastos::ElaWallet;


TEST_CASE("Transaction Serialize and Deserialize", "[Transaction]") {
	Log::registerMultiLogger();
	srand(time(nullptr));

	SECTION("transaction Serialize test") {
		Transaction tx1;
		initTransaction(tx1, Transaction::TxVersion::Default);

		ByteStream stream;
		tx1.Serialize(stream);

		REQUIRE(tx1.EstimateSize() == stream.GetBytes().size());

		Transaction tx2;
		REQUIRE(tx2.Deserialize(stream));

		verifyTransaction(tx1, tx2, false);

		tx2 = tx1;

		verifyTransaction(tx1, tx2, true);
	}

}

TEST_CASE("Convert to and from json", "[Transaction]") {
	srand(time(nullptr));

	SECTION("to and from json") {
		Transaction tx1;

		initTransaction(tx1, Transaction::TxVersion::V09);

		nlohmann::json txJson = tx1.ToJson();

		Transaction tx2;
		tx2.FromJson(txJson);

		verifyTransaction(tx1, tx2, true);
	}

	SECTION("from json string") {
		nlohmann::json j = R"({
			"Attributes":[{"Data":"31393532323632373233","Usage":0}],
			"BlockHeight":2147483647,
			"Fee":20000,
			"Inputs":[
			{
				"Index":1,
				"Sequence":0,
				"TxHash":"a693bd76ef3aa8c2001ae11cf7b26c3fa8a2a35385cb160e739ffb1edebe263b"
			}],
			"IsRegistered":false,
			"LockTime":0,
			"Outputs":[
			{
				"Address":"ERSqjfWDwTYw7iLrCZYLHKzSzEYzF4QZUz",
				"Amount":2300000000,
				"AssetId":"a3d0eaa466df74983b5d7c543de6904f4c9418ead5ffd6d25814234a96db37b0",
				"OutputLock":0,
				"OutputType":1,
				"Payload":
				{
					"Version":0,
					"VoteContent":[
					{
						"Candidates":[
							"03330ee8520088b7f578a9afabaef0c034fa31fe1354cb3a14410894f974132800",
							"033c495238ca2b6bb8b7f5ae172363caea9a55cf245ffb3272d078126b1fe3e7cd",
							"0337e6eaabfab6321d109d48e135190560898d42a1d871bfe8fecc67f4c3992250",
							"03c78467b91805c95ada2530513069bef1f1f1e7b756861381ab534efa6d94e40a",
							"021d59a84d2243111e39e8c2af0a5089127d142d52b18c3e4bf744e0c6f8af44e0",
							"036417ab256114a32bcff38f3e10f0384cfa9238afa41a163017687b3ce1fa17f2",
							"02e578a6f4295765ad3be4cdac9be15de5aedaf1ae76e86539bb54c397e467cd5e",
							"02ddd829f3495a2ce76d908c3e6e7d4505e12c4718c5af4b4cbff309cfd3aeab88",
							"03c7b1f234d5d16472fcdd24d121e4cd224e1074f558a3eb1a6a146aa91dcf9c0d",
							"03b688e0124580de452c400e01c628a690527e8742b6fa4645026dbc70155d7c8b",
							"03bc2c2b75009a3a551e98bf206730501ecdf46e71b0405840ff1d5750094bd4ff",
							"0230d383546d154d67cfafc6091c0736c0b26a8c7c16e879ef8011d91df976f1fb",
							"028fb1a85f6a30a516b9e3516d03267403a3af0c96d73b0284ca0c1165318531ff",
							"02db921cfb4bf504c83038212aafe52cc1d0a07eb71a399a0d2162fe0cd4d47720",
							"033fb33f39276b93d3474cf7999887bed16c3211ee7f904399eeead4c480d7d592",
							"030e4b487daf8e14dbd7023e3f6f475d00145a1f1cc87be4b8d58a4291ab0a3b1a",
							"0234048d3ee92a7d34fbe3da22bc69583b1785e8f6684c9f4f11804c518cb4e53d",
							"0203c80103bb094b5870f6b99b0bc6ab857fa87bab1896fc845108bba7aafbfe3c",
							"0210694f4ab518037bc2dcc3f5e1a1030e8a36821ab019c10f29d4a894b8034498",
							"02771568d40c1b20f3cbc2f4de327d3f61ae1a97a3e4a014838d267c818f2f999e",
							"02d1c315626710a4f556ee56f1978787e07d464b2287170e7789f2cb1ca60ece11",
							"03ba357f743e5dcab39dcd60a0a62f9ad573eae0d911291fd30846891f5ce29987",
							"038796d13f0ed94b2587ba2e13ca99b3cafd4d5cea2b08b2d06b841ed10d177a51"
						],
						"Type":0
					}]
				},
				"ProgramHash":"215af4f0f51ff9f011b1e21703a22dedc71fab3c8d"
			},
			{
				"Address":"ERSqjfWDwTYw7iLrCZYLHKzSzEYzF4QZUz",
				"Amount":2687533220000,
				"AssetId":"a3d0eaa466df74983b5d7c543de6904f4c9418ead5ffd6d25814234a96db37b0",
				"OutputLock":0,
				"OutputType":0,
				"Payload":null,
				"ProgramHash":"215af4f0f51ff9f011b1e21703a22dedc71fab3c8d"
			}],
			"PayLoad":null,
			"PayloadVersion":0,
			"Programs":[
			{
				"Code":"2102676d08015cdabae42647de70667a3f33f56037495d1d10a00092f9361b5c6cc0ac",
				"Parameter":"40be01660cb4501c7741ea7437a284cdcb5dca33c1e01b6b1a47e4814135c36430eeb19f8b2c89162bb45396e5f34c3a65f5ac05ec90f342a6ead2a48b6beb8a2f"
			}],
			"Remark":"",
			"Timestamp":118,
			"TxHash":"caed058f3a852547f5d44323ed7c97efd9876fab171f3db24416faee5fe1b63a",
			"Type":2,
			"Version":9
		})"_json;

		Transaction tx;
		tx.FromJson(j);

		uint256 hash = tx.GetHash();

		REQUIRE(j["TxHash"] == hash.GetHex());
		REQUIRE(tx.IsSigned());
	}

	SECTION("Deserialize from raw tx") {
		std::string rawtx = "09020001000a31393532323632373233013b26bede1efb9f730e16cb8553a3a2a83f6cb2f71ce11a00c2a83aef76bd93a601000000000002b037db964a231458d2d6ffd5ea18944c4f90e63d547c5d3b9874df66a4ead0a3003717890000000000000000215af4f0f51ff9f011b1e21703a22dedc71fab3c8d01000100172103330ee8520088b7f578a9afabaef0c034fa31fe1354cb3a14410894f97413280021033c495238ca2b6bb8b7f5ae172363caea9a55cf245ffb3272d078126b1fe3e7cd210337e6eaabfab6321d109d48e135190560898d42a1d871bfe8fecc67f4c39922502103c78467b91805c95ada2530513069bef1f1f1e7b756861381ab534efa6d94e40a21021d59a84d2243111e39e8c2af0a5089127d142d52b18c3e4bf744e0c6f8af44e021036417ab256114a32bcff38f3e10f0384cfa9238afa41a163017687b3ce1fa17f22102e578a6f4295765ad3be4cdac9be15de5aedaf1ae76e86539bb54c397e467cd5e2102ddd829f3495a2ce76d908c3e6e7d4505e12c4718c5af4b4cbff309cfd3aeab882103c7b1f234d5d16472fcdd24d121e4cd224e1074f558a3eb1a6a146aa91dcf9c0d2103b688e0124580de452c400e01c628a690527e8742b6fa4645026dbc70155d7c8b2103bc2c2b75009a3a551e98bf206730501ecdf46e71b0405840ff1d5750094bd4ff210230d383546d154d67cfafc6091c0736c0b26a8c7c16e879ef8011d91df976f1fb21028fb1a85f6a30a516b9e3516d03267403a3af0c96d73b0284ca0c1165318531ff2102db921cfb4bf504c83038212aafe52cc1d0a07eb71a399a0d2162fe0cd4d4772021033fb33f39276b93d3474cf7999887bed16c3211ee7f904399eeead4c480d7d59221030e4b487daf8e14dbd7023e3f6f475d00145a1f1cc87be4b8d58a4291ab0a3b1a210234048d3ee92a7d34fbe3da22bc69583b1785e8f6684c9f4f11804c518cb4e53d210203c80103bb094b5870f6b99b0bc6ab857fa87bab1896fc845108bba7aafbfe3c210210694f4ab518037bc2dcc3f5e1a1030e8a36821ab019c10f29d4a894b80344982102771568d40c1b20f3cbc2f4de327d3f61ae1a97a3e4a014838d267c818f2f999e2102d1c315626710a4f556ee56f1978787e07d464b2287170e7789f2cb1ca60ece112103ba357f743e5dcab39dcd60a0a62f9ad573eae0d911291fd30846891f5ce2998721038796d13f0ed94b2587ba2e13ca99b3cafd4d5cea2b08b2d06b841ed10d177a51b037db964a231458d2d6ffd5ea18944c4f90e63d547c5d3b9874df66a4ead0a3a08076bd7102000000000000215af4f0f51ff9f011b1e21703a22dedc71fab3c8d0000000000014140be01660cb4501c7741ea7437a284cdcb5dca33c1e01b6b1a47e4814135c36430eeb19f8b2c89162bb45396e5f34c3a65f5ac05ec90f342a6ead2a48b6beb8a2f232102676d08015cdabae42647de70667a3f33f56037495d1d10a00092f9361b5c6cc0ac";
		bytes_t bytes(rawtx);
		ByteStream stream(bytes);

		Transaction tx;
		REQUIRE(tx.Deserialize(stream));

		uint256 hash = tx.GetHash();
		REQUIRE("caed058f3a852547f5d44323ed7c97efd9876fab171f3db24416faee5fe1b63a" == hash.GetHex());
	}
}
