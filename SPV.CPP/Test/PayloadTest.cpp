/*
 * Copyright (c) 2019 Elastos Foundation
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#define CATCH_CONFIG_MAIN

#include <catch.hpp>
#include "TestHelper.h"
#include <Common/Log.h>
#include <Plugin/Transaction/Payload/CoinBase.h>
#include <Plugin/Transaction/Payload/CRCouncilMemberClaimNode.h>
#include <WalletCore/Mnemonic.h>
#include <WalletCore/HDKeychain.h>
#include <WalletCore/BIP39.h>
#include <WalletCore/Key.h>
#include <Plugin/Transaction/Payload/RechargeToSideChain.h>
#include <Plugin/Transaction/Payload/CRCProposalReview.h>
#include <Plugin/Transaction/Payload/Record.h>
#include <Plugin/Transaction/Payload/RegisterAsset.h>
#include <Plugin/Transaction/Payload/SideChainPow.h>
#include <Plugin/Transaction/Payload/TransferCrossChainAsset.h>
#include <Plugin/Transaction/Payload/WithdrawFromSideChain.h>
#include <Plugin/Transaction/Payload/NextTurnDPoSInfo.h>
#include <Plugin/Transaction/Payload/CRCProposal.h>
#include <Plugin/Transaction/Payload/CRCProposalTracking.h>
#include <Plugin/Transaction/Payload/CRInfo.h>
#include <Plugin/Transaction/Payload/UnregisterCR.h>
#include <Plugin/Transaction/Payload/RegisterIdentification.h>

using namespace Elastos::ElaWallet;

static void initCRCouncilMemberClaimNodePayload(CRCouncilMemberClaimNode &payload, uint8_t version) {
	std::string mnemonic = Mnemonic(boost::filesystem::path("Data")).Create("English", Mnemonic::WORDS_12);
	uint512 seed = BIP39::DeriveSeed(mnemonic, "");
	HDSeed hdseed(seed.bytes());
	HDKeychain rootkey(CTElastos, hdseed.getExtendedKey(CTElastos, true));
	HDKeychain masterKey = rootkey.getChild("44'/0'/0'");
	HDKeychain nodePublicKey = masterKey.getChild("0/0");
	HDKeychain councilMemberKey = masterKey.getChild("0/1");

	payload.SetNodePublicKey(nodePublicKey.pubkey());
	payload.SetCRCouncilMemberDID(Address(PrefixIDChain, councilMemberKey.pubkey(), true));

	Key key;
	const uint256 &digest = payload.DigestUnsigned(version);
	key = councilMemberKey;
	bytes_t signature = key.Sign(digest);
	payload.SetCRCouncilMemberSignature(signature);
}

static void initCRCProposalReviewPayload(CRCProposalReview &review) {
	review.SetProposalHash(getRanduint256());
	uint8_t result = getRandUInt8() % 3;
	review.SetVoteResult(CRCProposalReview::VoteResult(result));
	bytes_t opinionData = getRandBytes(50);
	review.SetOpinionData(opinionData);
	uint256 opinionHash(sha256_2(opinionData));
	review.SetOpinionHash(opinionHash);
	review.SetDID(Address("icwTktC5M6fzySQ5yU7bKAZ6ipP623apFY"));
	review.SetSignature(getRandBytes(90));
}

static void initRegisterAsset(AssetPtr &asset) {
	asset->SetName(getRandString(20));
	asset->SetDescription(getRandString(50));
	asset->SetPrecision(getRandUInt8());
	asset->SetAssetType(Asset::AssetType::Share);
	asset->SetAssetRecordType(Asset::AssetRecordType::Balance);
	asset->SetHash(uint256());
	asset->GetHash();
}

static void initCRCProposalPayload(CRCProposal &crcProposal, CRCProposal::Type type, uint8_t version) {
	std::string mnemonic = Mnemonic(boost::filesystem::path("Data")).Create("English", Mnemonic::WORDS_12);
	uint512 seed = BIP39::DeriveSeed(mnemonic, "");
	HDSeed hdseed(seed.bytes());
	HDKeychain rootkey(CTElastos, hdseed.getExtendedKey(CTElastos, true));
	HDKeychain masterKey = rootkey.getChild("44'/0'/0'");
	HDKeychain ownerKey = masterKey.getChild("0/0");
	HDKeychain newOwnerKey = masterKey.getChild("0/1");
	HDKeychain secretaryKey = masterKey.getChild("0/2");
	HDKeychain councilMemberKey = masterKey.getChild("0/3");

	crcProposal.SetTpye(type);
	crcProposal.SetCategoryData(getRandString(100));
	crcProposal.SetOwnerPublicKey(ownerKey.pubkey().getHex());
	bytes_t draftData = getRandBytes(100);
	crcProposal.SetDraftData(draftData);
	uint256 draftHash(sha256_2(draftData));
	crcProposal.SetDraftHash(draftHash);
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

static void initCRCProposalTrackingPayload(CRCProposalTracking &tracking) {
	CRCProposalTracking::Type type = CRCProposalTracking::Type(getRandUInt8() % 6);
	tracking.SetProposalHash(getRanduint256());
	bytes_t messageData = getRandBytes(100);
	tracking.SetMessageData(messageData);
	uint256 messageHash(sha256_2(messageData));
	tracking.SetMessageHash(messageHash);
	tracking.SetStage(getRandUInt8());
	tracking.SetOwnerPubKey(getRandBytes(33));
	tracking.SetNewOwnerPubKey(getRandBytes(33));
	tracking.SetOwnerSign(getRandBytes(64));
	tracking.SetNewOwnerSign(getRandBytes(64));
	tracking.SetType(type);
	bytes_t opinionData = getRandBytes(100);
	tracking.SetSecretaryGeneralOpinionData(opinionData);
	uint256 opinionHash(sha256_2(opinionData));
	tracking.SetSecretaryGeneralOpinionHash(opinionHash);
	tracking.SetSecretaryGeneralSignature(getRandBytes(64));
}

TEST_CASE("Payload Test", "[Transaction Payload]") {
	Log::registerMultiLogger();

	SECTION("CoinBase") {
		uint8_t version = 0;

		CoinBase p1(getRandBytes(50)), p2;
		ByteStream stream;
		p1.Serialize(stream, version);
		REQUIRE(p2.Deserialize(stream, version));
		REQUIRE((p1.Equal(p2, version)));

		p1 = CoinBase(getRandBytes(50));
		nlohmann::json p1Json = p1.ToJson(version);
		p2.FromJson(p1Json, version);
		REQUIRE((p1.Equal(p2, version)));
	}

	SECTION("CRCouncilMemberClaimNode") {
		uint8_t version = CRCouncilMemberClaimNodeVersion;

		CRCouncilMemberClaimNode p1, p2;
		ByteStream stream;

		initCRCouncilMemberClaimNodePayload(p1, version);
		REQUIRE(p1.IsValid(version));
		p1.Serialize(stream, version);
		REQUIRE(p2.Deserialize(stream, version));
		REQUIRE(p1.Equal(p2, version));

		initCRCouncilMemberClaimNodePayload(p1, version);
		REQUIRE(p1.IsValid(version));
		nlohmann::json j = p1.ToJson(version);
		REQUIRE_NOTHROW(p2.FromJson(j, version));
		REQUIRE(p1.Equal(p2, version));
	}

	SECTION("RechargeToSideChain") {
		std::vector<uint8_t> versions = {RechargeToSideChain::V0, RechargeToSideChain::V1};
		for (uint8_t &version : versions) {
			RechargeToSideChain p1(getRandBytes(50), getRandBytes(100), getRanduint256());
			ByteStream stream;
			p1.Serialize(stream, version);
			RechargeToSideChain p2;
			REQUIRE(p2.Deserialize(stream, version));
			REQUIRE(p1.Equal(p2, version));

			p1 = RechargeToSideChain(getRandBytes(50), getRandBytes(100), getRanduint256());
			nlohmann::json j = p1.ToJson(version);
			p2.FromJson(j, version);
			REQUIRE(p1.Equal(p2, version));
		}
	}

	SECTION("CRCProposalReview") {
		std::vector<uint8_t> versions = {CRCProposalReviewDefaultVersion, CRCProposalReviewVersion01};
		for (uint8_t &version : versions) {
			CRCProposalReview p1;
			initCRCProposalReviewPayload(p1);
			ByteStream byteStream;
			p1.Serialize(byteStream, version);
			REQUIRE(byteStream.GetBytes().size() == p1.EstimateSize(version));
			CRCProposalReview p2;
			REQUIRE(p2.Deserialize(byteStream, version));
			REQUIRE(p1.Equal(p2, version));

			initCRCProposalReviewPayload(p1);
			nlohmann::json j = p1.ToJson(version);
			p2.FromJson(j, version);
			REQUIRE(p1.Equal(p2, version));
		}
	}

	SECTION("RECORD") {
		uint8_t version = 0;
		Record p1(getRandString(20), getRandBytes(50)), p2;
		ByteStream stream;
		p1.Serialize(stream, version);
		REQUIRE(p2.Deserialize(stream, version));
		REQUIRE(p1.Equal(p2, version));

		p1 = Record(getRandString(20), getRandBytes(50));
		nlohmann::json p1Json = p1.ToJson(version);
		p2.FromJson(p1Json, version);
		REQUIRE(p1.Equal(p2, version));
	}

	SECTION("RegisterAsset") {
		uint8_t version = 0;
		RegisterAsset p1, p2;
		AssetPtr asset(new Asset());
		initRegisterAsset(asset);
		p1.SetAsset(asset);
		p1.SetAmount(getRandUInt64());
		p1.SetController(getRandUInt168());
		ByteStream stream;
		p1.Serialize(stream, version);
		REQUIRE(p2.Deserialize(stream, version));
		REQUIRE(p1.Equal(p2, version));

		initRegisterAsset(asset);
		p1.SetAsset(asset);
		p1.SetAmount(getRandUInt64());
		p1.SetController(getRandUInt168());
		nlohmann::json p1Json = p1.ToJson(version);
		p2.FromJson(p1Json, version);
		REQUIRE(p1.Equal(p2, version));
	}

	SECTION("SideChainPow") {
		uint8_t version = 0;
		SideChainPow p1, p2;
		p1.SetSideBlockHash(getRanduint256());
		p1.SetSideGenesisHash(getRanduint256());
		p1.SetBlockHeight(getRandUInt32());
		p1.SetSignedData(getRandBytes(100));
		ByteStream stream;
		p1.Serialize(stream, version);
		REQUIRE(p2.Deserialize(stream, version));
		REQUIRE(p1.Equal(p2, version));

		p1.SetSideBlockHash(getRanduint256());
		p1.SetSideGenesisHash(getRanduint256());
		p1.SetBlockHeight(getRandUInt32());
		p1.SetSignedData(getRandBytes(100));
		nlohmann::json p1Json = p1.ToJson(version);
		p2.FromJson(p1Json, version);
		REQUIRE(p1.Equal(p2, version));
	}

	SECTION("WithdrawFromSideChain") {
		uint8_t version = 0;
		std::vector<TransferInfo> transferInfo = {
			TransferInfo(getRandString(33), getRandUInt16(), getRandBigInt()),
			TransferInfo(getRandString(33), getRandUInt16(), getRandBigInt()),
			TransferInfo(getRandString(33), getRandUInt16(), getRandBigInt()),
			TransferInfo(getRandString(33), getRandUInt16(), getRandBigInt())
		};
		TransferCrossChainAsset p1(transferInfo), p2;
		ByteStream stream;
		p1.Serialize(stream, version);
		REQUIRE(p2.Deserialize(stream, version));
		REQUIRE(p1.Equal(p2, version));

		transferInfo = {
			TransferInfo(getRandString(33), getRandUInt16(), getRandBigInt()),
			TransferInfo(getRandString(33), getRandUInt16(), getRandBigInt()),
			TransferInfo(getRandString(33), getRandUInt16(), getRandBigInt()),
			TransferInfo(getRandString(33), getRandUInt16(), getRandBigInt())
		};
		nlohmann::json j = p1.ToJson(version);
		p2.FromJson(j, version);
		REQUIRE(p1.Equal(p2, version));
	}

	SECTION("WithdrawFromSideChain") {
		uint8_t version = 0;
		WithdrawFromSideChain p2, p1(getRandUInt32(), getRandString(33), {getRanduint256(), getRanduint256()});
		ByteStream stream;
		p1.Serialize(stream, version);
		REQUIRE(p2.Deserialize(stream, version));
		REQUIRE(p1.Equal(p2, version));

		p1 = WithdrawFromSideChain(getRandUInt32(), getRandString(33), {getRanduint256(), getRanduint256()});
		nlohmann::json j = p1.ToJson(version);
		p2.FromJson(j, version);
		REQUIRE(p1.Equal(p2, version));
	}

	SECTION("NextTurnDPoSInfo") {
		uint8_t version = 0;
		NextTurnDPoSInfo p2, p1(getRandUInt32(), {getRandBytes(33), getRandBytes(33)}, {getRandBytes(33), getRandBytes(33)});
		ByteStream stream;
		p1.Serialize(stream, version);
		REQUIRE(p2.Deserialize(stream, version));

		p1 = NextTurnDPoSInfo(getRandUInt32(), {getRandBytes(33), getRandBytes(33)}, {getRandBytes(33), getRandBytes(33)});
		nlohmann::json j = p1.ToJson(version);
		p2.FromJson(j, version);
		REQUIRE(p1.Equal(p2, version));
	}

	SECTION("CRCProposal") {
		CRCProposal p1, p2;
		std::vector<uint8_t> versions = {CRCProposalDefaultVersion, CRCProposalVersion01};
		std::vector<CRCProposal::Type> types = {
			CRCProposal::normal,
			CRCProposal::secretaryGeneralElection,
			CRCProposal::changeProposalOwner,
			CRCProposal::terminateProposal
		};

		for (uint8_t &version : versions) {
			for (CRCProposal::Type type : types) {
				ByteStream byteStream;
				initCRCProposalPayload(p1, type, version);
				p1.Serialize(byteStream, version);
				REQUIRE(byteStream.GetBytes().size() == p1.EstimateSize(version));
				REQUIRE(p2.Deserialize(byteStream, version));
				REQUIRE(p1.Equal(p2, version));

				initCRCProposalPayload(p1, type, version);
				nlohmann::json j = p1.ToJson(version);
				p2.FromJson(j, version);
				REQUIRE(p1.Equal(p2, version));
			}
		}
	}

	SECTION("CRCProposalTracking") {
		std::vector<uint8_t> versions = {CRCProposalTrackingDefaultVersion, CRCProposalTrackingVersion01};

		for (uint8_t &version : versions) {
			CRCProposalTracking p1, p2;
			initCRCProposalTrackingPayload(p1);
			ByteStream byteStream;
			p1.Serialize(byteStream, version);
			REQUIRE(byteStream.GetBytes().size() == p1.EstimateSize(version));
			REQUIRE(p2.Deserialize(byteStream, version));
			REQUIRE(p1.Equal(p2, version));

			initCRCProposalTrackingPayload(p1);
			nlohmann::json j = p1.ToJson(version);
			p2.FromJson(j, version);
			REQUIRE(p1.Equal(p2, version));
		}
	}

	SECTION("CRInfo") {
		std::vector<uint8_t> versions = {CRInfoVersion, CRInfoDIDVersion};

		for (uint8_t version : versions) {
			CRInfo p2, p1(getRandBytes(25), getRandUInt168(), getRandUInt168(), getRandString(20), getRandString(50), getRandUInt64(), getRandBytes(64));;
			ByteStream stream;
			p1.Serialize(stream, version);
			REQUIRE(p2.Deserialize(stream, version));
			REQUIRE(p1.Equal(p2, version));

			p1 = CRInfo(getRandBytes(25), getRandUInt168(), getRandUInt168(), getRandString(20), getRandString(50), getRandUInt64(), getRandBytes(64));;
			nlohmann::json j = p1.ToJson(version);
			p2.FromJson(j, version);
			REQUIRE(p1.Equal(p1, version));
		}
	}

	SECTION("UnregisterCR") {
		uint8_t version = 0;
		UnregisterCR p2, p1(getRandUInt168(), getRandBytes(64));
		ByteStream stream;
		p1.Serialize(stream, version);
		REQUIRE(p2.Deserialize(stream, version));
		REQUIRE(p1.Equal(p2, version));

		p1 = UnregisterCR(getRandUInt168(), getRandBytes(64));
		nlohmann::json j = p1.ToJson(version);
		p2.FromJson(j, version);
		REQUIRE(p1.Equal(p2, version));
	}

	SECTION("RegisterIdentification") {
		uint8_t version = 0;
		RegisterIdentification p1, p2;
		p1.SetID(getRandString(33));
		p1.SetSign(getRandBytes(64));
		std::vector<RegisterIdentification::SignContent> contents = {
			RegisterIdentification::SignContent(
				getRandString(50),
				{
					RegisterIdentification::ValueItem(getRanduint256(), getRandString(50), getRandString(50)),
					RegisterIdentification::ValueItem(getRanduint256(), getRandString(50), getRandString(50)),
					RegisterIdentification::ValueItem(getRanduint256(), getRandString(50), getRandString(50))
				}),
			RegisterIdentification::SignContent(
				getRandString(50),
				{
					RegisterIdentification::ValueItem(getRanduint256(), getRandString(50), getRandString(50)),
					RegisterIdentification::ValueItem(getRanduint256(), getRandString(50), getRandString(50)),
					RegisterIdentification::ValueItem(getRanduint256(), getRandString(50), getRandString(50))
				}),
			RegisterIdentification::SignContent(
				getRandString(50),
				{
					RegisterIdentification::ValueItem(getRanduint256(), getRandString(50), getRandString(50)),
					RegisterIdentification::ValueItem(getRanduint256(), getRandString(50), getRandString(50)),
					RegisterIdentification::ValueItem(getRanduint256(), getRandString(50), getRandString(50))
				})
		};
		p1.SetContent(contents);
		ByteStream stream;
		p1.Serialize(stream, version);
		REQUIRE(p2.Deserialize(stream, version));
		REQUIRE(p1.Equal(p2, version));

		p1.SetID(getRandString(33));
		p1.SetSign(getRandBytes(64));
		contents = {
			RegisterIdentification::SignContent(
				getRandString(50),
				{
					RegisterIdentification::ValueItem(getRanduint256(), getRandString(50), getRandString(50)),
					RegisterIdentification::ValueItem(getRanduint256(), getRandString(50), getRandString(50)),
					RegisterIdentification::ValueItem(getRanduint256(), getRandString(50), getRandString(50))
				}),
			RegisterIdentification::SignContent(
				getRandString(50),
				{
					RegisterIdentification::ValueItem(getRanduint256(), getRandString(50), getRandString(50)),
					RegisterIdentification::ValueItem(getRanduint256(), getRandString(50), getRandString(50)),
					RegisterIdentification::ValueItem(getRanduint256(), getRandString(50), getRandString(50))
				}),
			RegisterIdentification::SignContent(
				getRandString(50),
				{
					RegisterIdentification::ValueItem(getRanduint256(), getRandString(50), getRandString(50)),
					RegisterIdentification::ValueItem(getRanduint256(), getRandString(50), getRandString(50)),
					RegisterIdentification::ValueItem(getRanduint256(), getRandString(50), getRandString(50))
				}),
			RegisterIdentification::SignContent(
				getRandString(50),
				{
					RegisterIdentification::ValueItem(getRanduint256(), getRandString(50), getRandString(50)),
					RegisterIdentification::ValueItem(getRanduint256(), getRandString(50), getRandString(50)),
					RegisterIdentification::ValueItem(getRanduint256(), getRandString(50), getRandString(50))
				})
		};
		p1.SetContent(contents);
		nlohmann::json j = p1.ToJson(version);
		p2.FromJson(j, version);
		REQUIRE(p1.Equal(p2, version));
	}

}
