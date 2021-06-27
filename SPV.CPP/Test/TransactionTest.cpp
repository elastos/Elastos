// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#define CATCH_CONFIG_MAIN

#include <catch.hpp>
#include "TestHelper.h"

#include <Plugin/Transaction/Transaction.h>
#include <Plugin/Transaction/Attribute.h>
#include <Plugin/Transaction/IDTransaction.h>
#include <Plugin/Transaction/Payload/DIDInfo.h>
#include <Common/Utils.h>
#include <Common/Log.h>
#include <Common/ElementSet.h>

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

	SECTION("transaction set") {
		ElementSet<TransactionPtr> txSet;

		for (size_t i = 0; i < 30; ++i) {
			TransactionPtr tx1(new Transaction());
			initTransaction(*tx1, Transaction::TxVersion::V09);
			REQUIRE(txSet.Insert(tx1));
			REQUIRE(txSet.Size() == i + 1);
			REQUIRE(txSet.Contains(tx1));
			REQUIRE(txSet.Contains(tx1->GetHash()));

			TransactionPtr tx2(new Transaction(*tx1));
			REQUIRE(tx1->GetHash() == tx2->GetHash());
			REQUIRE(tx1.get() != tx2.get());
			REQUIRE(!txSet.Insert(tx2));
			REQUIRE(txSet.Size() == i + 1);
		}
	}

}

TEST_CASE("Convert to and from json", "[Transaction]") {
	srand(time(nullptr));

	SECTION("to and from json") {
		Transaction tx1;

		initTransaction(tx1, Transaction::TxVersion::V09);

		nlohmann::json txJson = tx1;

		Transaction tx2 = txJson;

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
				"ContainDetail": false,
				"Address": "",
				"Amount": "0",
				"TxHash":"a693bd76ef3aa8c2001ae11cf7b26c3fa8a2a35385cb160e739ffb1edebe263b"
			}],
			"IsRegistered":false,
			"LockTime":0,
			"Outputs":[
			{
				"FixedIndex": 0,
				"Address":"ERSqjfWDwTYw7iLrCZYLHKzSzEYzF4QZUz",
				"Amount":"2300000000",
				"AssetId":"a3d0eaa466df74983b5d7c543de6904f4c9418ead5ffd6d25814234a96db37b0",
				"OutputLock":0,
				"OutputType":1,
				"Payload":
				{
					"Version":0,
					"VoteContent":[
					{
						"Candidates":[
							{"Candidate":"03330ee8520088b7f578a9afabaef0c034fa31fe1354cb3a14410894f974132800"},
							{"Candidate":"033c495238ca2b6bb8b7f5ae172363caea9a55cf245ffb3272d078126b1fe3e7cd"},
							{"Candidate":"0337e6eaabfab6321d109d48e135190560898d42a1d871bfe8fecc67f4c3992250"},
							{"Candidate":"03c78467b91805c95ada2530513069bef1f1f1e7b756861381ab534efa6d94e40a"},
							{"Candidate":"021d59a84d2243111e39e8c2af0a5089127d142d52b18c3e4bf744e0c6f8af44e0"},
							{"Candidate":"036417ab256114a32bcff38f3e10f0384cfa9238afa41a163017687b3ce1fa17f2"},
							{"Candidate":"02e578a6f4295765ad3be4cdac9be15de5aedaf1ae76e86539bb54c397e467cd5e"},
							{"Candidate":"02ddd829f3495a2ce76d908c3e6e7d4505e12c4718c5af4b4cbff309cfd3aeab88"},
							{"Candidate":"03c7b1f234d5d16472fcdd24d121e4cd224e1074f558a3eb1a6a146aa91dcf9c0d"},
							{"Candidate":"03b688e0124580de452c400e01c628a690527e8742b6fa4645026dbc70155d7c8b"},
							{"Candidate":"03bc2c2b75009a3a551e98bf206730501ecdf46e71b0405840ff1d5750094bd4ff"},
							{"Candidate":"0230d383546d154d67cfafc6091c0736c0b26a8c7c16e879ef8011d91df976f1fb"},
							{"Candidate":"028fb1a85f6a30a516b9e3516d03267403a3af0c96d73b0284ca0c1165318531ff"},
							{"Candidate":"02db921cfb4bf504c83038212aafe52cc1d0a07eb71a399a0d2162fe0cd4d47720"},
							{"Candidate":"033fb33f39276b93d3474cf7999887bed16c3211ee7f904399eeead4c480d7d592"},
							{"Candidate":"030e4b487daf8e14dbd7023e3f6f475d00145a1f1cc87be4b8d58a4291ab0a3b1a"},
							{"Candidate":"0234048d3ee92a7d34fbe3da22bc69583b1785e8f6684c9f4f11804c518cb4e53d"},
							{"Candidate":"0203c80103bb094b5870f6b99b0bc6ab857fa87bab1896fc845108bba7aafbfe3c"},
							{"Candidate":"0210694f4ab518037bc2dcc3f5e1a1030e8a36821ab019c10f29d4a894b8034498"},
							{"Candidate":"02771568d40c1b20f3cbc2f4de327d3f61ae1a97a3e4a014838d267c818f2f999e"},
							{"Candidate":"02d1c315626710a4f556ee56f1978787e07d464b2287170e7789f2cb1ca60ece11"},
							{"Candidate":"03ba357f743e5dcab39dcd60a0a62f9ad573eae0d911291fd30846891f5ce29987"},
							{"Candidate":"038796d13f0ed94b2587ba2e13ca99b3cafd4d5cea2b08b2d06b841ed10d177a51"}
						],
						"Type":0
					}]
				},
				"ProgramHash":"8d3cab1fc7ed2da20317e2b111f0f91ff5f0f45a21"
			},
			{
				"FixedIndex": 1,
				"Address":"ERSqjfWDwTYw7iLrCZYLHKzSzEYzF4QZUz",
				"Amount":"2687533220000",
				"AssetId":"a3d0eaa466df74983b5d7c543de6904f4c9418ead5ffd6d25814234a96db37b0",
				"OutputLock":0,
				"OutputType":0,
				"Payload":null,
				"ProgramHash":"8d3cab1fc7ed2da20317e2b111f0f91ff5f0f45a21"
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

		Transaction tx = j;

		tx.ResetHash();
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

TEST_CASE("new tx with type and payload", "[IDTransaction]") {
	std::vector<nlohmann::json> list;
	list.push_back(R"(
{"header":{"specification":"elastos/did/1.0","operation":"create"},"payload":"eyJpZCI6ImRpZDplbGFzdG9zOmlmVVBhcG83dlJUQXQyYzd5dGQ0QnJib295SzdCN0dwNFIiLCJwdWJsaWNLZXkiOlt7ImlkIjoiI3ByaW1hcnkiLCJwdWJsaWNLZXlCYXNlNTgiOiJrTjYxNHZzNVBjR21nUjRycW9uSDQyekNyTFJ0VHNoWm1XUVdzOGI4OGc5YyJ9XSwiYXV0aGVudGljYXRpb24iOlsiI3ByaW1hcnkiXSwidmVyaWZpYWJsZUNyZWRlbnRpYWwiOlt7ImlkIjoiI3Byb2ZpbGUiLCJ0eXBlIjpbIkJhc2ljUHJvZmlsZUNyZWRlbnRpYWwiLCJTZWxmUHJvY2xhaW1lZENyZWRlbnRpYWwiXSwiaXNzdWFuY2VEYXRlIjoiMjAxOS0xMi0yMVQwODo1MzoxNVoiLCJleHBpcmF0aW9uRGF0ZSI6IjIwMjQtMTItMjFUMDg6NTM6MTVaIiwiY3JlZGVudGlhbFN1YmplY3QiOnsiZW1haWwiOiJqb2huQGV4YW1wbGUuY29tIiwiZ2VuZGVyIjoiTWFsZSIsImxhbmd1YWdlIjoiRW5nbGlzaCIsIm5hbWUiOiJKb2huIiwibmF0aW9uIjoiU2luZ2Fwb3JlIiwidHdpdHRlciI6IkBqb2huIn0sInByb29mIjp7InZlcmlmaWNhdGlvbk1ldGhvZCI6IiNwcmltYXJ5Iiwic2lnbmF0dXJlIjoiZGk3c05QVXVEbWljSUJJcF8wVjJPRnRLWjBHeWJkUzJacUtteEhHMXJ3YUs0MGlxSC11R1BzVmQ2OTN3VTNxTVlueDNsZm9VQ3U2SWtzRlBocTd6Q1EifX1dLCJleHBpcmVzIjoiMjAyNC0xMi0yMVQwODo1MzoxNVoiLCJwcm9vZiI6eyJjcmVhdGVkIjoiMjAxOS0xMi0yMVQwODo1MzoxNVoiLCJzaWduYXR1cmVWYWx1ZSI6IndOcGxLLU5IUGk1b3JaZHI3S2NlUTJBVHJQYmkzQWZ4RG1FcTRGUVlQT0dhNEFCQVFLYWlOWmpRSWFkMVRIV3VJUnp5aGd0QVFWWUo5NmxTRG1sMGJnIn19","proof":{"verificationMethod":"#primary","signature":"506QI2nkxhwLmqxJW3zMIOyIhsdx66H5ZdxKOYhHeb1BGFFLaXVR2yfXPLgNfueNPQpX8oK2A1YpmMVyc3XYfQ"}}
)"_json);

	list.push_back(R"(
{"header":{"specification":"elastos/did/1.0","operation":"update","previousTxid":"97b366a8c7193be0d1d9073de920dd0b"},"payload":"eyJpZCI6ImRpZDplbGFzdG9zOmlmVVBhcG83dlJUQXQyYzd5dGQ0QnJib295SzdCN0dwNFIiLCJwdWJsaWNLZXkiOlt7ImlkIjoiI3ByaW1hcnkiLCJwdWJsaWNLZXlCYXNlNTgiOiJrTjYxNHZzNVBjR21nUjRycW9uSDQyekNyTFJ0VHNoWm1XUVdzOGI4OGc5YyJ9XSwiYXV0aGVudGljYXRpb24iOlsiI3ByaW1hcnkiXSwidmVyaWZpYWJsZUNyZWRlbnRpYWwiOlt7ImlkIjoiI3Bhc3Nwb3J0IiwidHlwZSI6WyJCYXNpY1Byb2ZpbGVDcmVkZW50aWFsIiwiU2VsZlByb2NsYWltZWRDcmVkZW50aWFsIl0sImlzc3VhbmNlRGF0ZSI6IjIwMTktMTItMjFUMDg6NTM6MTVaIiwiZXhwaXJhdGlvbkRhdGUiOiIyMDI0LTEyLTIxVDA4OjUzOjE1WiIsImNyZWRlbnRpYWxTdWJqZWN0Ijp7Im5hdGlvbiI6IlNpbmdhcG9yZSIsInBhc3Nwb3J0IjoiUzY1MzI1OFowNyJ9LCJwcm9vZiI6eyJ2ZXJpZmljYXRpb25NZXRob2QiOiIjcHJpbWFyeSIsInNpZ25hdHVyZSI6Ino0SnZVOTk4dWZxS0FxWm5VSXVhSW5NUDVmcEk0YUU1QThUS085bkRBSi1ncXRMLXNVMmQzZjJjaXI5ajUySHk0cTRxcFJmcjAzdVo0cDZqS0RROFFBIn19LHsiaWQiOiIjcHJvZmlsZSIsInR5cGUiOlsiQmFzaWNQcm9maWxlQ3JlZGVudGlhbCIsIlNlbGZQcm9jbGFpbWVkQ3JlZGVudGlhbCJdLCJpc3N1YW5jZURhdGUiOiIyMDE5LTEyLTIxVDA4OjUzOjE1WiIsImV4cGlyYXRpb25EYXRlIjoiMjAyNC0xMi0yMVQwODo1MzoxNVoiLCJjcmVkZW50aWFsU3ViamVjdCI6eyJlbWFpbCI6ImpvaG5AZXhhbXBsZS5jb20iLCJnZW5kZXIiOiJNYWxlIiwibGFuZ3VhZ2UiOiJFbmdsaXNoIiwibmFtZSI6IkpvaG4iLCJuYXRpb24iOiJTaW5nYXBvcmUiLCJ0d2l0dGVyIjoiQGpvaG4ifSwicHJvb2YiOnsidmVyaWZpY2F0aW9uTWV0aG9kIjoiI3ByaW1hcnkiLCJzaWduYXR1cmUiOiJkaTdzTlBVdURtaWNJQklwXzBWMk9GdEtaMEd5YmRTMlpxS214SEcxcndhSzQwaXFILXVHUHNWZDY5M3dVM3FNWW54M2xmb1VDdTZJa3NGUGhxN3pDUSJ9fV0sImV4cGlyZXMiOiIyMDI0LTEyLTIxVDA4OjUzOjE1WiIsInByb29mIjp7ImNyZWF0ZWQiOiIyMDE5LTEyLTIxVDA4OjUzOjE1WiIsInNpZ25hdHVyZVZhbHVlIjoiakJ2TmxHVmFENDNsR3pYdFduYnBUXzc3NGlnd2RlandNUVdkeWpOYnZhN0lBb09LbWxuVzF5NkYxekVHekNpZzE0Q3FWQ2dMVmtkTzdNbWptYXBxcmcifX0","proof":{"verificationMethod":"#primary","signature":"qRcQALdDIPZUJOZW75td0_ROxVxIibbFVncfCKPmw_17-g-8-3LlKMcRvrojrjrekDFYua0-sDo5248C7ZomxA"}}
)"_json);

	list.push_back(R"(
{"header":{"specification":"elastos/did/1.0","operation":"update","previousTxid":"2c1dea205a25705d9085cdf03e28c72c"},"payload":"eyJpZCI6ImRpZDplbGFzdG9zOmlmVVBhcG83dlJUQXQyYzd5dGQ0QnJib295SzdCN0dwNFIiLCJwdWJsaWNLZXkiOlt7ImlkIjoiI3ByaW1hcnkiLCJwdWJsaWNLZXlCYXNlNTgiOiJrTjYxNHZzNVBjR21nUjRycW9uSDQyekNyTFJ0VHNoWm1XUVdzOGI4OGc5YyJ9XSwiYXV0aGVudGljYXRpb24iOlsiI3ByaW1hcnkiXSwidmVyaWZpYWJsZUNyZWRlbnRpYWwiOlt7ImlkIjoiI3Bhc3Nwb3J0IiwidHlwZSI6WyJCYXNpY1Byb2ZpbGVDcmVkZW50aWFsIiwiU2VsZlByb2NsYWltZWRDcmVkZW50aWFsIl0sImlzc3VhbmNlRGF0ZSI6IjIwMTktMTItMjFUMDg6NTM6MTVaIiwiZXhwaXJhdGlvbkRhdGUiOiIyMDI0LTEyLTIxVDA4OjUzOjE1WiIsImNyZWRlbnRpYWxTdWJqZWN0Ijp7Im5hdGlvbiI6IlNpbmdhcG9yZSIsInBhc3Nwb3J0IjoiUzY1MzI1OFowNyJ9LCJwcm9vZiI6eyJ2ZXJpZmljYXRpb25NZXRob2QiOiIjcHJpbWFyeSIsInNpZ25hdHVyZSI6Ino0SnZVOTk4dWZxS0FxWm5VSXVhSW5NUDVmcEk0YUU1QThUS085bkRBSi1ncXRMLXNVMmQzZjJjaXI5ajUySHk0cTRxcFJmcjAzdVo0cDZqS0RROFFBIn19LHsiaWQiOiIjcHJvZmlsZSIsInR5cGUiOlsiQmFzaWNQcm9maWxlQ3JlZGVudGlhbCIsIlNlbGZQcm9jbGFpbWVkQ3JlZGVudGlhbCJdLCJpc3N1YW5jZURhdGUiOiIyMDE5LTEyLTIxVDA4OjUzOjE1WiIsImV4cGlyYXRpb25EYXRlIjoiMjAyNC0xMi0yMVQwODo1MzoxNVoiLCJjcmVkZW50aWFsU3ViamVjdCI6eyJlbWFpbCI6ImpvaG5AZXhhbXBsZS5jb20iLCJnZW5kZXIiOiJNYWxlIiwibGFuZ3VhZ2UiOiJFbmdsaXNoIiwibmFtZSI6IkpvaG4iLCJuYXRpb24iOiJTaW5nYXBvcmUiLCJ0d2l0dGVyIjoiQGpvaG4ifSwicHJvb2YiOnsidmVyaWZpY2F0aW9uTWV0aG9kIjoiI3ByaW1hcnkiLCJzaWduYXR1cmUiOiJkaTdzTlBVdURtaWNJQklwXzBWMk9GdEtaMEd5YmRTMlpxS214SEcxcndhSzQwaXFILXVHUHNWZDY5M3dVM3FNWW54M2xmb1VDdTZJa3NGUGhxN3pDUSJ9fSx7ImlkIjoiI3Rlc3QiLCJ0eXBlIjpbIlNlbGZQcm9jbGFpbWVkQ3JlZGVudGlhbCIsIlRlc3RDcmVkZW50aWFsIl0sImlzc3VhbmNlRGF0ZSI6IjIwMTktMTItMjFUMDg6NTM6MTVaIiwiZXhwaXJhdGlvbkRhdGUiOiIyMDI0LTEyLTIxVDA4OjUzOjE1WiIsImNyZWRlbnRpYWxTdWJqZWN0Ijp7IkFiYyI6IkFiYyIsIkZvb2JhciI6IkZvb2JhciIsIlpvbyI6IlpvbyIsImFiYyI6ImFiYyIsImZvb2JhciI6ImZvb2JhciIsInpvbyI6InpvbyJ9LCJwcm9vZiI6eyJ2ZXJpZmljYXRpb25NZXRob2QiOiIjcHJpbWFyeSIsInNpZ25hdHVyZSI6InNFczNoakVJd2lxczBIalRxdFlsaTZ1VkRCWVVtSkRjak5mWGdHanRDb3JkZ3U5Z1l5amZFTEF4Q092Q2xQYXEtVlJqcGRBWVBDZnhJbEtnS0tCcDd3In19XSwiZXhwaXJlcyI6IjIwMjQtMTItMjFUMDg6NTM6MTVaIiwicHJvb2YiOnsiY3JlYXRlZCI6IjIwMTktMTItMjFUMDg6NTM6MTVaIiwic2lnbmF0dXJlVmFsdWUiOiJYMTB0OWRPci04OEN5TEE2RXItRjVNVTBTVG5oT3RMUWpzQmd2aF9iTV9hY0RpcW1QYldqSUJ4UE5oZTlmei1zQ0lWYkN3Rk16d3ZtMkcteUdvYWxtQSJ9fQ","proof":{"verificationMethod":"#primary","signature":"HaqDwD5J7fqOE0fSCYXqXiDa1tkJJBwKVzf9Nt_kEqKgD8yLPqybM41FS4mEOC_H04s0QkvDNKxVTCN8CeIjiA"}}
)"_json);

	for (size_t i = 0; i < list.size(); ++i) {
		PayloadPtr payload = PayloadPtr(new DIDInfo());
		payload->FromJson(list[i], 0);
		TransactionPtr tx1 = TransactionPtr(new IDTransaction(IDTransaction::didTransaction, payload));
		initTransaction(*tx1, Transaction::TxVersion::Default);

		ByteStream stream;
		tx1->Serialize(stream);

		TransactionPtr tx2 = TransactionPtr(new IDTransaction());
		tx2->Deserialize(stream);

		verifyTransaction(*tx1, *tx2, false);

		DIDInfo *didInfo = dynamic_cast<DIDInfo *>(tx2->GetPayload());

#if 0
		REQUIRE(didInfo->IsValid(0));
		const DIDHeaderInfo &header = didInfo->DIDHeader();
		REQUIRE(header.Specification() == "elastos/did/1.0");
		if (i == 0) {
			REQUIRE(header.Operation() == "create");
		} else {
			REQUIRE(header.Operation() == "update");
		}

		const DIDPayloadInfo &didPayloadInfo = didInfo->DIDPayload();
		REQUIRE(didPayloadInfo.ID() == "did:elastos:ifUPapo7vRTAt2c7ytd4BrbooyK7B7Gp4R");
		REQUIRE(didPayloadInfo.PublicKeyInfo().size() == 1);
		REQUIRE(didPayloadInfo.PublicKeyInfo()[0].ID() == "did:elastos:ifUPapo7vRTAt2c7ytd4BrbooyK7B7Gp4R#primary");
		REQUIRE(didPayloadInfo.PublicKeyInfo()[0].PublicKeyBase58() == "kN614vs5PcGmgR4rqonH42zCrLRtTshZmWQWs8b88g9c");
#endif
	}

}
