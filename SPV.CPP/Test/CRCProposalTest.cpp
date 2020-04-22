// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#define CATCH_CONFIG_MAIN

#include <catch.hpp>
#include "TestHelper.h"

#include <Plugin/Transaction/Payload/CRCProposal.h>

using namespace Elastos::ElaWallet;

static void initCRCProposal(CRCProposal &crcProposal) {
	CRCProposal::Type type = CRCProposal::Type(getRandUInt8() % 6);
	crcProposal.SetTpye(type);

	crcProposal.SetCategoryData(getRandString(100));

	std::string pubKey = "031f7a5a6bf3b2450cd9da4048d00a8ef1cb4912b5057535f65f3cc0e0c36f13b4";
	crcProposal.SetOwnerPublicKey(pubKey);

	crcProposal.SetDraftHash(getRanduint256());

	std::vector<Budget> budgets;
	for (int i = 0; i < 4; ++i) {
		Budget::Type budgetType = Budget::Type(getRandUInt8() % Budget::maxType);
		Budget budget(budgetType, getRandUInt8(), getRandUInt64());
		budgets.push_back(budget);
	}
	crcProposal.SetBudgets(budgets);

	crcProposal.SetRecipient(Address("EPbdmxUVBzfNrVdqJzZEySyWGYeuKAeKqv"));
	crcProposal.SetSignature(getRandBytes(50));
	crcProposal.SetCRCouncilMemberDID(Address("icwTktC5M6fzySQ5yU7bKAZ6ipP623apFY"));
	crcProposal.SetCRCouncilMemberSignature(getRandBytes(60));
}

static void verifyProposal(CRCProposal &p1, CRCProposal &p2) {
	REQUIRE(p1.GetType() == p2.GetType());
	REQUIRE(p1.GetCategoryData() == p2.GetCategoryData());
	REQUIRE(p1.GetOwnerPublicKey() == p2.GetOwnerPublicKey());
	REQUIRE(p1.GetDraftHash() == p2.GetDraftHash());

	const std::vector<Budget> &budgets1 = p1.GetBudgets();
	const std::vector<Budget> &budgets2 = p2.GetBudgets();
	REQUIRE(budgets1.size() == budgets2.size());
	for (size_t i = 0; i < budgets1.size(); ++i) {
		REQUIRE(budgets1[i].GetType() == budgets2[i].GetType());
		REQUIRE(budgets1[i].GetStage() == budgets2[i].GetStage());
		REQUIRE(budgets1[i].GetAmount() == budgets2[i].GetAmount());
	}

	REQUIRE(p1.GetRecipient() == p2.GetRecipient());
	REQUIRE(p1.GetSignature() == p2.GetSignature());
	REQUIRE(p1.GetCRCouncilMemberDID() == p2.GetCRCouncilMemberDID());
	REQUIRE(p1.GetCRCouncilMemberSignature() == p2.GetCRCouncilMemberSignature());
}

TEST_CASE("CRCProposal test", "[CRCProposal]") {
	SECTION("Serialize and Deserialize test") {
		CRCProposal p1;
		initCRCProposal(p1);

		ByteStream byteStream;
		p1.Serialize(byteStream, 0);

		REQUIRE(byteStream.GetBytes().size() == p1.EstimateSize(0));

		CRCProposal p2;
		REQUIRE(p2.Deserialize(byteStream, 0));

		verifyProposal(p1, p2);
	}

	SECTION("ToJson FromJson test") {
		CRCProposal p1;
		initCRCProposal(p1);

		nlohmann::json j = p1.ToJson(0);

		CRCProposal p2;
		p2.FromJson(j, 0);

		verifyProposal(p1, p2);
	}

}