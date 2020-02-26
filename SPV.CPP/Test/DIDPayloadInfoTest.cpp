// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#define CATCH_CONFIG_MAIN

#include <catch.hpp>
#include "TestHelper.h"

#include <Plugin/Transaction/Payload/DIDInfo.h>

using namespace Elastos::ElaWallet;

static void initCredentialSubject(CredentialSubject &subject) {
	std::string did = "did:elastos:ifUQ59wFpHUKe5NZ6gjffx48sWEBt9YgQE";
	std::string name = getRandString(5);
	std::string nickName = getRandString(6);
	std::string gender = "man";
	std::string birthday = "2019.10.12";
	std::string avatar = "img.jpg";
	std::string email = "test@test.com";
	std::string phone = "13032454523";
	std::string nation = "china";
	std::string desc = "this is simple descript";
	std::string homePage = "homePage";
	std::string googleAccount = "google@google.com";
	std::string microsoftPassport = "MicrosoftPassport";
	std::string facebook = "facebook";
	std::string twitter = "twitter";
	std::string weibo = "test@sina.com";
	std::string wechat = "wechat2333";
	std::string alipay = "alipay@223.com";

	subject.SetID(did);
	subject.AddProperties("name", name);
	subject.AddProperties("nickName", nickName);
	subject.AddProperties("gender", gender);
	subject.AddProperties("birthday", birthday);
	subject.AddProperties("avatar", avatar);
	subject.AddProperties("email", email);
	subject.AddProperties("phone", phone);
	subject.AddProperties("nation", nation);
	subject.AddProperties("descript", desc);
	subject.AddProperties("url", homePage);
	subject.AddProperties("googleAccount", googleAccount);
	subject.AddProperties("microsoftPassport", microsoftPassport);
	subject.AddProperties("facebook", facebook);
	subject.AddProperties("twitter", twitter);
	subject.AddProperties("weibo", weibo);
	subject.AddProperties("wechat", wechat);
	subject.AddProperties("alipay", alipay);
}

static void initVerifiableCredential(VerifiableCredential &veriffiableCredential) {
	veriffiableCredential.SetID("did:elastos:ifUQ59wFpHUKe5NZ6gjffx48sWEBt9YgQE");

	std::vector<std::string> types = {"BasicProfileCredential", "ElastosIDteriaCredential", "BasicProfileCredential",
	                                  "InternetAccountCredential", "PhoneCredential"};
	veriffiableCredential.SetTypes(types);

	veriffiableCredential.SetIssuer(getRandString(4));

	veriffiableCredential.SetIssuerDate("2019.10.11");

	CredentialSubject subject;
	initCredentialSubject(subject);
	veriffiableCredential.SetCredentialSubject(subject);

	DIDProofInfo proofInfo;
	proofInfo.SetType("ECDSAsecp256r1");
	proofInfo.SetVerificationMethod("did:example:icJ4z2DULrHEzYSvjKNJpKyhqFDxvYV7pN#keys1");
	proofInfo.SetSignature(
			"c1ux5u6ZHGC5UkPI97ZhwYWUhwFgrIAV9AMTDl9/s07BLhZ9tZn6zTh4+VdiDA6R98HjvwzAuSIkISWTxz5N/A==");
	veriffiableCredential.SetProof(proofInfo);
}

TEST_CASE("CredentialSubject test", "[CredentialSubject]") {
	SECTION("ToJson FromJson test") {
		CredentialSubject subject;
		initCredentialSubject(subject);

		nlohmann::json j = subject.ToJson(0);

		CredentialSubject subject2;
		subject2.FromJson(j, 0);

		REQUIRE(subject2.ID() == subject.ID());
		REQUIRE(subject2.GetValue("name") == subject.GetValue("name"));
		REQUIRE(subject2.GetValue("nickName") == subject.GetValue("nickName"));
		REQUIRE(subject2.GetValue("gender") == subject.GetValue("gender"));
		REQUIRE(subject2.GetValue("birthday") == subject.GetValue("birthday"));
		REQUIRE(subject2.GetValue("avatar") == subject.GetValue("avatar"));
		REQUIRE(subject2.GetValue("email") == subject.GetValue("email"));
		REQUIRE(subject2.GetValue("phone") == subject.GetValue("phone"));
		REQUIRE(subject2.GetValue("nation") == subject.GetValue("nation"));
		REQUIRE(subject2.GetValue("descript") == subject.GetValue("descript"));
		REQUIRE(subject2.GetValue("url") == subject.GetValue("url"));
		REQUIRE(subject2.GetValue("googleAccount") == subject.GetValue("googleAccount"));
		REQUIRE(subject2.GetValue("microsoftPassport") == subject.GetValue("microsoftPassport"));
		REQUIRE(subject2.GetValue("facebook") == subject.GetValue("facebook"));
		REQUIRE(subject2.GetValue("twitter") == subject.GetValue("twitter"));
		REQUIRE(subject2.GetValue("weibo") == subject.GetValue("weibo"));
		REQUIRE(subject2.GetValue("wechat") == subject.GetValue("wechat"));
		REQUIRE(subject2.GetValue("alipay") == subject.GetValue("alipay"));
	}
}

TEST_CASE("VerifiableCredential test", "[VerifiableCredential]") {
	SECTION("ToJson FromJson test") {
		VerifiableCredentialArray veriffiableCredentials;

		for (int i = 0; i < 3; ++i) {
			VerifiableCredential veriffiableCredential;
			initVerifiableCredential(veriffiableCredential);
			veriffiableCredentials.push_back(veriffiableCredential);
		}

		for (int i = 0; i < veriffiableCredentials.size(); ++i) {
			VerifiableCredential veriffiableCredential = veriffiableCredentials[i];

			nlohmann::json j = veriffiableCredential.ToJson(0);

			VerifiableCredential veriffiableCredential2;

			veriffiableCredential2.FromJson(j, 0);

			REQUIRE(veriffiableCredential2.ID() == veriffiableCredential.ID());

			std::vector<std::string> types = veriffiableCredential.Types();
			std::vector<std::string> types2 = veriffiableCredential2.Types();
			REQUIRE(types2.size() == types.size());
			for (size_t i = 0; i < types.size(); ++i) {
				REQUIRE(types[i] == types2[i]);
			}

			REQUIRE(veriffiableCredential2.GetIssuer() == veriffiableCredential.GetIssuer());
			REQUIRE(veriffiableCredential2.GetIssuerDate() == veriffiableCredential.GetIssuerDate());

			CredentialSubject subject = veriffiableCredential.GetCredentialSubject();
			CredentialSubject subject2 = veriffiableCredential2.GetCredentialSubject();

			REQUIRE(subject2.ID() == subject.ID());
			REQUIRE(subject2.GetValue("name") == subject.GetValue("name"));
			REQUIRE(subject2.GetValue("nickName") == subject.GetValue("nickName"));
			REQUIRE(subject2.GetValue("gender") == subject.GetValue("gender"));
			REQUIRE(subject2.GetValue("birthday") == subject.GetValue("birthday"));
			REQUIRE(subject2.GetValue("avatar") == subject.GetValue("avatar"));
			REQUIRE(subject2.GetValue("email") == subject.GetValue("email"));
			REQUIRE(subject2.GetValue("phone") == subject.GetValue("phone"));
			REQUIRE(subject2.GetValue("nation") == subject.GetValue("nation"));
			REQUIRE(subject2.GetValue("descript") == subject.GetValue("descript"));
			REQUIRE(subject2.GetValue("url") == subject.GetValue("url"));
			REQUIRE(subject2.GetValue("googleAccount") == subject.GetValue("googleAccount"));
			REQUIRE(subject2.GetValue("microsoftPassport") == subject.GetValue("microsoftPassport"));
			REQUIRE(subject2.GetValue("facebook") == subject.GetValue("facebook"));
			REQUIRE(subject2.GetValue("twitter") == subject.GetValue("twitter"));
			REQUIRE(subject2.GetValue("weibo") == subject.GetValue("weibo"));
			REQUIRE(subject2.GetValue("wechat") == subject.GetValue("wechat"));
			REQUIRE(subject2.GetValue("alipay") == subject.GetValue("alipay"));

			DIDProofInfo proofInfo = veriffiableCredential.Proof();
			DIDProofInfo proofInfo2 = veriffiableCredential2.Proof();
			REQUIRE(proofInfo2.Type() == proofInfo.Type());
			REQUIRE(proofInfo2.VerificationMethod() == proofInfo.VerificationMethod());
			REQUIRE(proofInfo2.Signature() == proofInfo.Signature());
		}
	}
}

TEST_CASE("DIDPayloadInfo test", "[DIDPayloadInfo]") {
	SECTION("ToJson FromJson test") {
		DIDPayloadInfo didPayloadInfo;
		didPayloadInfo.SetID("did:elastos:icJ4z2DULrHEzYSvjKNJpKyhqFDxvYV7pN");

		DIDPubKeyInfoArray didPubKeyInfoArray;
		for (int i = 0; i < 3; ++i) {
			DIDPubKeyInfo pubKeyInfo;
			pubKeyInfo.SetID(getRandString(23));
			pubKeyInfo.SetType(getRandString(5));
			pubKeyInfo.SetController(PREFIX_DID + getRandString(20));
			pubKeyInfo.SetPublicKeyBase58(getRandHexString(25));
			didPubKeyInfoArray.push_back(pubKeyInfo);
		}
		didPayloadInfo.SetPublickKey(didPubKeyInfoArray);

		DIDPubKeyInfoArray authentication;
		for (int i = 0; i < 3; ++i) {
			DIDPubKeyInfo pubKeyInfo;
			pubKeyInfo.SetID(getRandString(23));
			pubKeyInfo.SetType(getRandString(5));
			pubKeyInfo.SetController(PREFIX_DID + getRandString(20));
			pubKeyInfo.SetPublicKeyBase58(getRandHexString(25));
			authentication.push_back(pubKeyInfo);
		}
		didPayloadInfo.SetAuthentication(authentication);

		DIDPubKeyInfoArray authorization;
		for (int i = 0; i < 3; ++i) {
			DIDPubKeyInfo pubKeyInfo;
			pubKeyInfo.SetID(getRandString(23));
			pubKeyInfo.SetType(getRandString(5));
			pubKeyInfo.SetController(PREFIX_DID + getRandString(20));
			pubKeyInfo.SetPublicKeyBase58(getRandHexString(25));
			authorization.push_back(pubKeyInfo);
		}
		didPayloadInfo.SetAuthorization(authorization);

		VerifiableCredentialArray veriffiableCredentials;
		for (int i = 0; i < 3; ++i) {
			VerifiableCredential veriffiableCredential;
			initVerifiableCredential(veriffiableCredential);
			veriffiableCredentials.push_back(veriffiableCredential);
		}
		didPayloadInfo.SetVerifiableCredential(veriffiableCredentials);

		ServiceEndpoints serviceEndpoints;
		for (int i = 0; i < 3; ++i) {
			ServiceEndpoint serviceEndpoint;
			serviceEndpoint.SetID("#openid");
			serviceEndpoint.SetType("OpenIdConnectVersion1.0Service");
			serviceEndpoint.SetService("https://openid.example.com/" + getRandString(6));
			serviceEndpoints.push_back(serviceEndpoint);
		}

		didPayloadInfo.SetServiceEndpoints(serviceEndpoints);

		didPayloadInfo.SetExpires("2019-01-01T19:20:18Z");

		nlohmann::json j = didPayloadInfo.ToJson(0);

		DIDPayloadInfo didPayloadInfo2;
		didPayloadInfo2.FromJson(j, 0);

		REQUIRE(didPayloadInfo.ID() == didPayloadInfo2.ID());

		const DIDPubKeyInfoArray &didPubKeyInfoArray2 = didPayloadInfo2.PublicKeyInfo();
		REQUIRE(didPubKeyInfoArray2.size() == didPubKeyInfoArray.size());
		for (int i = 0; i < didPubKeyInfoArray.size(); ++i) {
			REQUIRE(didPubKeyInfoArray[i].ID() == didPubKeyInfoArray2[i].ID());
			REQUIRE(didPubKeyInfoArray[i].Type() == didPubKeyInfoArray2[i].Type());
			REQUIRE(didPubKeyInfoArray[i].Controller() == didPubKeyInfoArray2[i].Controller());
			REQUIRE(didPubKeyInfoArray[i].PublicKeyBase58() == didPubKeyInfoArray2[i].PublicKeyBase58());
		}

		DIDPubKeyInfoArray authentication2 = didPayloadInfo2.Authentication();
		REQUIRE(authentication2.size() == authentication.size());
		for (int i = 0; i < authentication.size(); ++i) {
			REQUIRE(authentication[i].ID() == authentication2[i].ID());
			REQUIRE(authentication[i].Type() == authentication2[i].Type());
			REQUIRE(authentication[i].Controller() == authentication2[i].Controller());
			REQUIRE(authentication[i].PublicKeyBase58() == authentication2[i].PublicKeyBase58());
		}

		DIDPubKeyInfoArray authorization2 = didPayloadInfo2.Authorization();
		REQUIRE(authorization2.size() == authorization.size());
		for (int i = 0; i < authorization.size(); ++i) {
			REQUIRE(authorization[i].ID() == authorization2[i].ID());
			REQUIRE(authorization[i].Type() == authorization2[i].Type());
			REQUIRE(authorization[i].Controller() == authorization2[i].Controller());
			REQUIRE(authorization[i].PublicKeyBase58() == authorization2[i].PublicKeyBase58());
		}

		VerifiableCredentialArray veriffiableCredentials2 = didPayloadInfo2.GetVerifiableCredential();
		REQUIRE(veriffiableCredentials2.size() == veriffiableCredentials.size());

		for (int i = 0; i < veriffiableCredentials2.size(); ++i) {
			REQUIRE(veriffiableCredentials2[i].ID() == veriffiableCredentials[i].ID());

			std::vector<std::string> types = veriffiableCredentials[i].Types();
			std::vector<std::string> types2 = veriffiableCredentials2[i].Types();
			REQUIRE(types2.size() == types.size());
			for (size_t i = 0; i < types.size(); ++i) {
				REQUIRE(types[i] == types2[i]);
			}

			REQUIRE(veriffiableCredentials2[i].GetIssuer() == veriffiableCredentials[i].GetIssuer());
			REQUIRE(veriffiableCredentials2[i].GetIssuerDate() == veriffiableCredentials[i].GetIssuerDate());

			CredentialSubject subject = veriffiableCredentials[i].GetCredentialSubject();
			CredentialSubject subject2 = veriffiableCredentials2[i].GetCredentialSubject();

			REQUIRE(subject2.ID() == subject.ID());
			REQUIRE(subject2.GetValue("name") == subject.GetValue("name"));
			REQUIRE(subject2.GetValue("nickName") == subject.GetValue("nickName"));
			REQUIRE(subject2.GetValue("gender") == subject.GetValue("gender"));
			REQUIRE(subject2.GetValue("birthday") == subject.GetValue("birthday"));
			REQUIRE(subject2.GetValue("avatar") == subject.GetValue("avatar"));
			REQUIRE(subject2.GetValue("email") == subject.GetValue("email"));
			REQUIRE(subject2.GetValue("phone") == subject.GetValue("phone"));
			REQUIRE(subject2.GetValue("nation") == subject.GetValue("nation"));
			REQUIRE(subject2.GetValue("descript") == subject.GetValue("descript"));
			REQUIRE(subject2.GetValue("url") == subject.GetValue("url"));
			REQUIRE(subject2.GetValue("googleAccount") == subject.GetValue("googleAccount"));
			REQUIRE(subject2.GetValue("microsoftPassport") == subject.GetValue("microsoftPassport"));
			REQUIRE(subject2.GetValue("facebook") == subject.GetValue("facebook"));
			REQUIRE(subject2.GetValue("twitter") == subject.GetValue("twitter"));
			REQUIRE(subject2.GetValue("weibo") == subject.GetValue("weibo"));
			REQUIRE(subject2.GetValue("wechat") == subject.GetValue("wechat"));
			REQUIRE(subject2.GetValue("alipay") == subject.GetValue("alipay"));

			DIDProofInfo proofInfo = veriffiableCredentials[i].Proof();
			DIDProofInfo proofInfo2 = veriffiableCredentials2[i].Proof();
			REQUIRE(proofInfo2.Type() == proofInfo.Type());
			REQUIRE(proofInfo2.VerificationMethod() == proofInfo.VerificationMethod());
			REQUIRE(proofInfo2.Signature() == proofInfo.Signature());

			ServiceEndpoints serviceEndpoints2 = didPayloadInfo2.GetServiceEndpoint();
			REQUIRE(serviceEndpoints2.size() == serviceEndpoints.size());
			for (int j = 0; j < serviceEndpoints2.size(); ++j) {
				REQUIRE(serviceEndpoints2[i].ID().find(serviceEndpoints[i].ID()) != std::string::npos);
				REQUIRE(serviceEndpoints2[i].Type() == serviceEndpoints[i].Type());
				REQUIRE(serviceEndpoints2[i].GetService() == serviceEndpoints[i].GetService());
			}

		}

		REQUIRE(didPayloadInfo2.Expires() == didPayloadInfo.Expires());
	}

	SECTION("DIDPayloadInfo FromJson from json") {
		std::vector<nlohmann::json> list;
		list.push_back(R"(
{"id":"did:elastos:ifUPapo7vRTAt2c7ytd4BrbooyK7B7Gp4R","publicKey":[{"id":"did:elastos:ifUPapo7vRTAt2c7ytd4BrbooyK7B7Gp4R#primary","type":"ECDSAsecp256r1","controller":"did:elastos:ifUPapo7vRTAt2c7ytd4BrbooyK7B7Gp4R","publicKeyBase58":"kN614vs5PcGmgR4rqonH42zCrLRtTshZmWQWs8b88g9c"}],"authentication":["did:elastos:ifUPapo7vRTAt2c7ytd4BrbooyK7B7Gp4R#primary"],"verifiableCredential":[{"id":"did:elastos:ifUPapo7vRTAt2c7ytd4BrbooyK7B7Gp4R#passport","type":["BasicProfileCredential","SelfProclaimedCredential"],"issuer":"did:elastos:ifUPapo7vRTAt2c7ytd4BrbooyK7B7Gp4R","issuanceDate":"2019-12-21T08:53:15Z","expirationDate":"2024-12-21T08:53:15Z","credentialSubject":{"id":"did:elastos:ifUPapo7vRTAt2c7ytd4BrbooyK7B7Gp4R","nation":"Singapore","passport":"S653258Z07"},"proof":{"type":"ECDSAsecp256r1","verificationMethod":"did:elastos:ifUPapo7vRTAt2c7ytd4BrbooyK7B7Gp4R#primary","signature":"z4JvU998ufqKAqZnUIuaInMP5fpI4aE5A8TKO9nDAJ-gqtL-sU2d3f2cir9j52Hy4q4qpRfr03uZ4p6jKDQ8QA"}},{"id":"did:elastos:ifUPapo7vRTAt2c7ytd4BrbooyK7B7Gp4R#profile","type":["BasicProfileCredential","SelfProclaimedCredential"],"issuer":"did:elastos:ifUPapo7vRTAt2c7ytd4BrbooyK7B7Gp4R","issuanceDate":"2019-12-21T08:53:15Z","expirationDate":"2024-12-21T08:53:15Z","credentialSubject":{"id":"did:elastos:ifUPapo7vRTAt2c7ytd4BrbooyK7B7Gp4R","email":"john@example.com","gender":"Male","language":"English","name":"John","nation":"Singapore","twitter":"@john"},"proof":{"type":"ECDSAsecp256r1","verificationMethod":"did:elastos:ifUPapo7vRTAt2c7ytd4BrbooyK7B7Gp4R#primary","signature":"di7sNPUuDmicIBIp_0V2OFtKZ0GybdS2ZqKmxHG1rwaK40iqH-uGPsVd693wU3qMYnx3lfoUCu6IksFPhq7zCQ"}},{"id":"did:elastos:ifUPapo7vRTAt2c7ytd4BrbooyK7B7Gp4R#test","type":["SelfProclaimedCredential","TestCredential"],"issuer":"did:elastos:ifUPapo7vRTAt2c7ytd4BrbooyK7B7Gp4R","issuanceDate":"2019-12-21T08:53:15Z","expirationDate":"2024-12-21T08:53:15Z","credentialSubject":{"id":"did:elastos:ifUPapo7vRTAt2c7ytd4BrbooyK7B7Gp4R","Abc":"Abc","Foobar":"Foobar","Zoo":"Zoo","abc":"abc","foobar":"foobar","zoo":"zoo"},"proof":{"type":"ECDSAsecp256r1","verificationMethod":"did:elastos:ifUPapo7vRTAt2c7ytd4BrbooyK7B7Gp4R#primary","signature":"sEs3hjEIwiqs0HjTqtYli6uVDBYUmJDcjNfXgGjtCordgu9gYyjfELAxCOvClPaq-VRjpdAYPCfxIlKgKKBp7w"}}],"expires":"2024-12-21T08:53:15Z","proof":{"type":"ECDSAsecp256r1","created":"2019-12-21T08:53:15Z","creator":"did:elastos:ifUPapo7vRTAt2c7ytd4BrbooyK7B7Gp4R#primary","signatureValue":"X10t9dOr-88CyLA6Er-F5MU0STnhOtLQjsBgvh_bM_acDiqmPbWjIBxPNhe9fz-sCIVbCwFMzwvm2G-yGoalmA"}}
)"_json);

		list.push_back(R"(
{"id":"did:elastos:ifUPapo7vRTAt2c7ytd4BrbooyK7B7Gp4R","publicKey":[{"id":"did:elastos:ifUPapo7vRTAt2c7ytd4BrbooyK7B7Gp4R#primary","type":"ECDSAsecp256r1","controller":"did:elastos:ifUPapo7vRTAt2c7ytd4BrbooyK7B7Gp4R","publicKeyBase58":"kN614vs5PcGmgR4rqonH42zCrLRtTshZmWQWs8b88g9c"}],"authentication":["did:elastos:ifUPapo7vRTAt2c7ytd4BrbooyK7B7Gp4R#primary"],"verifiableCredential":[{"id":"did:elastos:ifUPapo7vRTAt2c7ytd4BrbooyK7B7Gp4R#passport","type":["BasicProfileCredential","SelfProclaimedCredential"],"issuer":"did:elastos:ifUPapo7vRTAt2c7ytd4BrbooyK7B7Gp4R","issuanceDate":"2019-12-21T08:53:15Z","expirationDate":"2024-12-21T08:53:15Z","credentialSubject":{"id":"did:elastos:ifUPapo7vRTAt2c7ytd4BrbooyK7B7Gp4R","nation":"Singapore","passport":"S653258Z07"},"proof":{"type":"ECDSAsecp256r1","verificationMethod":"did:elastos:ifUPapo7vRTAt2c7ytd4BrbooyK7B7Gp4R#primary","signature":"z4JvU998ufqKAqZnUIuaInMP5fpI4aE5A8TKO9nDAJ-gqtL-sU2d3f2cir9j52Hy4q4qpRfr03uZ4p6jKDQ8QA"}},{"id":"did:elastos:ifUPapo7vRTAt2c7ytd4BrbooyK7B7Gp4R#profile","type":["BasicProfileCredential","SelfProclaimedCredential"],"issuer":"did:elastos:ifUPapo7vRTAt2c7ytd4BrbooyK7B7Gp4R","issuanceDate":"2019-12-21T08:53:15Z","expirationDate":"2024-12-21T08:53:15Z","credentialSubject":{"id":"did:elastos:ifUPapo7vRTAt2c7ytd4BrbooyK7B7Gp4R","email":"john@example.com","gender":"Male","language":"English","name":"John","nation":"Singapore","twitter":"@john"},"proof":{"type":"ECDSAsecp256r1","verificationMethod":"did:elastos:ifUPapo7vRTAt2c7ytd4BrbooyK7B7Gp4R#primary","signature":"di7sNPUuDmicIBIp_0V2OFtKZ0GybdS2ZqKmxHG1rwaK40iqH-uGPsVd693wU3qMYnx3lfoUCu6IksFPhq7zCQ"}}],"expires":"2024-12-21T08:53:15Z","proof":{"type":"ECDSAsecp256r1","created":"2019-12-21T08:53:15Z","creator":"did:elastos:ifUPapo7vRTAt2c7ytd4BrbooyK7B7Gp4R#primary","signatureValue":"jBvNlGVaD43lGzXtWnbpT_774igwdejwMQWdyjNbva7IAoOKmlnW1y6F1zEGzCig14CqVCgLVkdO7Mmjmapqrg"}}
)"_json);

		list.push_back(R"(
{"id":"did:elastos:ifUPapo7vRTAt2c7ytd4BrbooyK7B7Gp4R","publicKey":[{"id":"did:elastos:ifUPapo7vRTAt2c7ytd4BrbooyK7B7Gp4R#primary","type":"ECDSAsecp256r1","controller":"did:elastos:ifUPapo7vRTAt2c7ytd4BrbooyK7B7Gp4R","publicKeyBase58":"kN614vs5PcGmgR4rqonH42zCrLRtTshZmWQWs8b88g9c"}],"authentication":["did:elastos:ifUPapo7vRTAt2c7ytd4BrbooyK7B7Gp4R#primary"],"verifiableCredential":[{"id":"did:elastos:ifUPapo7vRTAt2c7ytd4BrbooyK7B7Gp4R#profile","type":["BasicProfileCredential","SelfProclaimedCredential"],"issuer":"did:elastos:ifUPapo7vRTAt2c7ytd4BrbooyK7B7Gp4R","issuanceDate":"2019-12-21T08:53:15Z","expirationDate":"2024-12-21T08:53:15Z","credentialSubject":{"id":"did:elastos:ifUPapo7vRTAt2c7ytd4BrbooyK7B7Gp4R","email":"john@example.com","gender":"Male","language":"English","name":"John","nation":"Singapore","twitter":"@john"},"proof":{"type":"ECDSAsecp256r1","verificationMethod":"did:elastos:ifUPapo7vRTAt2c7ytd4BrbooyK7B7Gp4R#primary","signature":"di7sNPUuDmicIBIp_0V2OFtKZ0GybdS2ZqKmxHG1rwaK40iqH-uGPsVd693wU3qMYnx3lfoUCu6IksFPhq7zCQ"}}],"expires":"2024-12-21T08:53:15Z","proof":{"type":"ECDSAsecp256r1","created":"2019-12-21T08:53:15Z","creator":"did:elastos:ifUPapo7vRTAt2c7ytd4BrbooyK7B7Gp4R#primary","signatureValue":"wNplK-NHPi5orZdr7KceQ2ATrPbi3AfxDmEq4FQYPOGa4ABAQKaiNZjQIad1THWuIRzyhgtAQVYJ96lSDml0bg"}}
)"_json);

		for(size_t i = 0; i < list.size(); ++i) {
			DIDPayloadInfo didPayloadInfo;
			didPayloadInfo.FromJson(list[i], 0);
			REQUIRE(didPayloadInfo.IsValid());
			REQUIRE(didPayloadInfo.ID() == "did:elastos:ifUPapo7vRTAt2c7ytd4BrbooyK7B7Gp4R");
			REQUIRE(didPayloadInfo.PublicKeyInfo().size() == 1);
			REQUIRE(didPayloadInfo.PublicKeyInfo()[0].ID() == "did:elastos:ifUPapo7vRTAt2c7ytd4BrbooyK7B7Gp4R#primary");
			REQUIRE(didPayloadInfo.PublicKeyInfo()[0].PublicKeyBase58() == "kN614vs5PcGmgR4rqonH42zCrLRtTshZmWQWs8b88g9c");
		}
	}
}