// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#define CATCH_CONFIG_MAIN

#include <catch.hpp>
#include "TestHelper.h"

#include <Common/Log.h>
#include <WalletCore/Mnemonic.h>
#include <Plugin/Transaction/Payload/CRCProposal.h>
#include <boost/filesystem.hpp>
#include <WalletCore/BIP39.h>
#include <WalletCore/HDKeychain.h>
#include <WalletCore/Key.h>

using namespace Elastos::ElaWallet;

static void initCRCProposal(CRCProposal &crcProposal, CRCProposal::Type type) {
	std::string mnemonic = Mnemonic(boost::filesystem::path("Data")).Create("English", Mnemonic::WORDS_12);
	uint512 seed = BIP39::DeriveSeed(mnemonic, "");
	HDSeed hdseed(seed.bytes());
	HDKeychain rootkey(hdseed.getExtendedKey(true));
	HDKeychain masterKey = rootkey.getChild("44'/0'/0'");
	HDKeychain ownerKey = masterKey.getChild("0/0");
	HDKeychain newOwnerKey = masterKey.getChild("0/1");
	HDKeychain secretaryKey = masterKey.getChild("0/2");
	HDKeychain councilMemberKey = masterKey.getChild("0/3");
	uint8_t version = CRCProposalDefaultVersion;

	crcProposal.SetTpye(type);
	crcProposal.SetCategoryData(getRandString(100));
	crcProposal.SetOwnerPublicKey(ownerKey.pubkey().getHex());
	crcProposal.SetDraftHash(getRanduint256());
	std::vector<Budget> budgets;
	for (int i = 0; i < 4; ++i) {
		Budget::Type budgetType = Budget::Type(getRandUInt8() % Budget::maxType);
		Budget budget(budgetType, getRandUInt8(), getRandUInt64());
		budgets.push_back(budget);
	}
	crcProposal.SetBudgets(budgets);
	crcProposal.SetRecipient(Address(Prefix::PrefixStandard, ownerKey.pubkey()));
	crcProposal.SetTargetProposalHash(getRanduint256());
	crcProposal.SetNewRecipient(Address(Prefix::PrefixStandard, newOwnerKey.pubkey()));
	crcProposal.SetNewOwnerPublicKey(newOwnerKey.pubkey());
	crcProposal.SetSecretaryPublicKey(secretaryKey.pubkey());
	crcProposal.SetSecretaryDID(Address(Prefix::PrefixIDChain, secretaryKey.pubkey(), true));
	crcProposal.SetCRCouncilMemberDID(Address(Prefix::PrefixIDChain, councilMemberKey.pubkey(), true));

	Key key;
	uint256 digest;
	bytes_t signature;
	switch (type) {
		case CRCProposal::Type::elip:
		case CRCProposal::Type::normal:
			digest = crcProposal.DigestNormalOwnerUnsigned(version);
			key = ownerKey;
			signature = key.Sign(digest);
			crcProposal.SetSignature(signature);

			digest = crcProposal.DigestNormalCRCouncilMemberUnsigned(version);
			key = councilMemberKey;
			signature = key.Sign(digest);
			crcProposal.SetCRCouncilMemberSignature(signature);
			break;

		case CRCProposal::Type::secretaryGeneralElection:
			digest = crcProposal.DigestSecretaryElectionUnsigned(version);
			key = ownerKey;
			signature = key.Sign(digest);
			crcProposal.SetSignature(signature);

			key = secretaryKey;
			signature = key.Sign(digest);
			crcProposal.SetSecretarySignature(signature);

			digest = crcProposal.DigestSecretaryElectionCRCouncilMemberUnsigned(version);
			key = councilMemberKey;
			signature = key.Sign(digest);
			crcProposal.SetCRCouncilMemberSignature(signature);
			break;

		case CRCProposal::Type::changeProposalOwner:
			digest = crcProposal.DigestChangeOwnerUnsigned(version);
			key = ownerKey;
			signature = key.Sign(digest);
			crcProposal.SetSignature(signature);

			key = newOwnerKey;
			signature = key.Sign(digest);
			crcProposal.SetNewOwnerSignature(signature);

			digest = crcProposal.DigestChangeOwnerCRCouncilMemberUnsigned(version);
			key = councilMemberKey;
			signature = key.Sign(digest);
			crcProposal.SetCRCouncilMemberSignature(signature);
			break;

		case CRCProposal::Type::terminateProposal:
			digest = crcProposal.DigestTerminateProposalOwnerUnsigned(version);
			key = ownerKey;
			signature = key.Sign(digest);
			crcProposal.SetSignature(signature);

			digest = crcProposal.DigestTerminateProposalCRCouncilMemberUnsigned(version);
			key = councilMemberKey;
			signature = key.Sign(digest);
			crcProposal.SetCRCouncilMemberSignature(signature);
			break;

		default:
			break;
	}
}

TEST_CASE("CRCProposal test", "[CRCProposal]") {
	Log::registerMultiLogger();
	uint8_t version = CRCProposalDefaultVersion;
	SECTION("Serialize and Deserialize test") {
		CRCProposal p1, p2;
		std::vector<CRCProposal::Type> types = {
			CRCProposal::normal,
			CRCProposal::secretaryGeneralElection,
			CRCProposal::changeProposalOwner,
			CRCProposal::terminateProposal
		};

		for (size_t i = 0; i < types.size(); ++i) {
			ByteStream byteStream;
			initCRCProposal(p1, types[i]);
			p1.Serialize(byteStream, version);
			REQUIRE(byteStream.GetBytes().size() == p1.EstimateSize(version));
			REQUIRE(p2.Deserialize(byteStream, version));
			REQUIRE(p1 == p2);
		}
	}

	SECTION("ToJson FromJson test") {
		CRCProposal p1, p2;
		std::vector<CRCProposal::Type> types = {
			CRCProposal::normal,
			CRCProposal::secretaryGeneralElection,
			CRCProposal::changeProposalOwner,
			CRCProposal::terminateProposal
		};

		for (size_t i = 0; i < types.size(); ++i) {
			initCRCProposal(p1, types[i]);
			nlohmann::json j = p1.ToJson(version);
			p2.FromJson(j, version);
			REQUIRE(p1 == p2);
		}
	}

}