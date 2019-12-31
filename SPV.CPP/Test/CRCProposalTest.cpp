// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#define CATCH_CONFIG_MAIN

#include <catch.hpp>
#include "TestHelper.h"

#include <Plugin/Transaction/Payload/CRCProposal.h>

using namespace Elastos::ElaWallet;

static void initCRCProposal(CRCProposal &crcProposal) {
	CRCProposal::CRCProposalType type = CRCProposal::CRCProposalType(getRandUInt8() % 6);
	crcProposal.SetTpye(type);

	std::string pubKey = "031f7a5a6bf3b2450cd9da4048d00a8ef1cb4912b5057535f65f3cc0e0c36f13b4";
	crcProposal.SetSponsorPublicKey(pubKey);
	crcProposal.SetCRSponsorDID(getRandUInt168());
	crcProposal.SetDraftHash(getRanduint256());

	std::vector<BigInt> budgets;
	for (int i = 0; i < 4; ++i) {
		BigInt amount = getRandBigInt();
		budgets.push_back(amount);
	}

	crcProposal.SetBudgets(budgets);
	crcProposal.SetRecipient(getRandUInt168());
	crcProposal.SetSignature(getRandBytes(50));
	crcProposal.SetCRSignature(getRandBytes(60));

}

TEST_CASE("CRCProposal test", "[CRCProposal]") {
	SECTION("Serialize and Deserialize test") {
		CRCProposal crcProposal1;
		initCRCProposal(crcProposal1);

		ByteStream byteStream;
		crcProposal1.Serialize(byteStream, 0);

		REQUIRE(byteStream.GetBytes().size() == crcProposal1.EstimateSize(0));

		CRCProposal crcProposal2;
		REQUIRE(crcProposal2.Deserialize(byteStream, 0) == true);

		REQUIRE(crcProposal1.GetType() == crcProposal2.GetType());
		REQUIRE(crcProposal1.GetSponsorPublicKey() == crcProposal2.GetSponsorPublicKey());
		REQUIRE(crcProposal1.GetCRSponsorDID() == crcProposal2.GetCRSponsorDID());
		REQUIRE(crcProposal1.GetDraftHash() == crcProposal2.GetDraftHash());

		std::vector<BigInt> budgets1 = crcProposal1.GetBudgets();
		std::vector<BigInt> budgets2 = crcProposal2.GetBudgets();
		REQUIRE(budgets1.size() == budgets2.size());
		for (size_t i = 0; i < budgets1.size(); ++i) {
			REQUIRE(budgets1[i] == budgets2[i]);
		}

		REQUIRE(crcProposal1.GetRecipient() == crcProposal2.GetRecipient());
		REQUIRE(crcProposal1.GetSignature() == crcProposal2.GetSignature());
		REQUIRE(crcProposal1.GetCRSignature() == crcProposal2.GetCRSignature());
	}

	SECTION("ToJson FromJson test") {
		CRCProposal crcProposal1;
		initCRCProposal(crcProposal1);

		nlohmann::json j = crcProposal1.ToJson(0);
		REQUIRE(j.empty() == false);

		CRCProposal crcProposal2;
		crcProposal2.FromJson(j, 0);

		REQUIRE(crcProposal1.GetType() == crcProposal2.GetType());
		REQUIRE(crcProposal1.GetSponsorPublicKey() == crcProposal2.GetSponsorPublicKey());
		REQUIRE(crcProposal1.GetCRSponsorDID() == crcProposal2.GetCRSponsorDID());
		REQUIRE(crcProposal1.GetDraftHash() == crcProposal2.GetDraftHash());

		std::vector<BigInt> budgets1 = crcProposal1.GetBudgets();
		std::vector<BigInt> budgets2 = crcProposal2.GetBudgets();
		REQUIRE(budgets1.size() == budgets2.size());
		for (size_t i = 0; i < budgets1.size(); ++i) {
			REQUIRE(budgets1[i] == budgets2[i]);
		}

		REQUIRE(crcProposal1.GetRecipient() == crcProposal2.GetRecipient());
		REQUIRE(crcProposal1.GetSignature() == crcProposal2.GetSignature());
		REQUIRE(crcProposal1.GetCRSignature() == crcProposal2.GetCRSignature());
	}

}