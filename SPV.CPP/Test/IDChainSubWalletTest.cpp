// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#define CATCH_CONFIG_MAIN

#include <catch.hpp>

#include "TestHelper.h"
#include <SDK/Common/Log.h>
#include <SDK/Database/DatabaseManager.h>
#include <SDK/Implement/MasterWallet.h>
#include <SDK/WalletCore/Crypto/AES.h>
#include <SDK/WalletCore/BIPs/HDKeychain.h>
#include <SDK/WalletCore/BIPs/Key.h>
#include <SDK/Wallet/UTXO.h>
#include <SDK/Plugin/Transaction/IDTransaction.h>
#include <SDK/Plugin/Transaction/Payload/DIDInfo.h>

#include <Interface/MasterWalletManager.h>
#include <Interface/IIDChainSubWallet.h>

using namespace Elastos::ElaWallet;

class TestMasterWalletManager : public MasterWalletManager {
public:
	TestMasterWalletManager() :
			MasterWalletManager(MasterWalletMap(), "Data", "Data") {
		_p2pEnable = false;
	}

	TestMasterWalletManager(const std::string &rootPath) :
			MasterWalletManager(rootPath) {
		_p2pEnable = false;
	}
};

static const std::string masterWalletId = "masterWalletId";
static const std::string payPassword = "payPassword";

TEST_CASE("Wallet GetResolveDIDInfo test", "[GetResolveDIDInfo]") {
	Log::registerMultiLogger();
	std::string path = "Data/" + masterWalletId + "/";
	boost::filesystem::remove_all(path);
	boost::filesystem::remove_all("Data/testWallet");
	boost::filesystem::create_directories(path);

	// prepare wallet data
	std::string keyPath = "44'/0'/0'/0/0";
	LocalStore ls(nlohmann::json::parse(
		"{\"account\":0,\"coinInfo\":[{\"ChainID\":\"IDChain\",\"EarliestPeerTime\":1513936800,\"FeePerKB\":10000,\"VisibleAssets\":[\"a3d0eaa466df74983b5d7c543de6904f4c9418ead5ffd6d25814234a96db37b0\"]}],\"derivationStrategy\":\"BIP44\",\"m\":1,\"mnemonic\":\"P0C/7w2/h13rLqA8qgI+BwwDZrcK9g8ixjdPFFIEC6+G62Qsm4WsmoNbxJE+shQ2jy7tTsPsDYLKCow9hsGrWchJuBV5ULwwcnRhYimP9TlAYA6uTdk3aQgolw==\",\"mnemonicHasPassphrase\":true,\"n\":1,\"ownerPubKey\":\"027c876ac77226d6f25d198983b1ae58baa39b136ff0e09386e064b40d646767a1\",\"passphrase\":\"\",\"publicKeyRing\":[{\"requestPubKey\":\"027d917aa4732ebffcb496a40cce2bf5b57237570c106b97b98fa5be433c6b743d\",\"xPubKey\":\"xpub6CWoR5hv1BestMgnyWLPR1RXdttXhFLK9vTjri9J79SgYdCnpjTCNF5JiwyZXsaW4pMonQ8gaHWv5xUi9DgBLMzdWE75EULLzU444PkpF7E\"}],\"readonly\":false,\"requestPrivKey\":\"HiFHrGoxzoY3yCbshyUtMtY1fIO2rFw5265BALZgv08Vuen9c1llzg==\",\"requestPubKey\":\"027d917aa4732ebffcb496a40cce2bf5b57237570c106b97b98fa5be433c6b743d\",\"singleAddress\":false,\"xPrivKey\":\"SK1ian/e9X2YQdBdioAjo/P11xkGlaXp9AFXcSIYUmPuQwz8aepAkk4hUG7KfqqEtzwb4kOTt0Tm+ZiYiRXtLmVkaVLMaQD1ab49rIHlRj9zw2fSft8=\",\"xPubKey\":\"xpub6CWoR5hv1BestMgnyWLPR1RXdttXhFLK9vTjri9J79SgYdCnpjTCNF5JiwyZXsaW4pMonQ8gaHWv5xUi9DgBLMzdWE75EULLzU444PkpF7E\"}"));
	ls.SaveTo(path);

	std::string iso = "ela1";
	DatabaseManager dm("Data/" + masterWalletId + "/IDChain.db");

	std::string xprv = ls.GetxPrivKey();
	bytes_t bytes = AES::DecryptCCM(xprv, payPassword);
	Key key = HDKeychain(bytes).getChild(keyPath);

	int txCount = 100;
	nlohmann::json didPayloadJSON = R"(
{"header":{"specification":"elastos/did/1.0","operation":"create"},"payload":"eyJpZCI6ImRpZDplbGFzdG9zOmlmVVE1OXdGcEhVS2U1Tlo2Z2pmZng0OHNXRUJ0OVlnUUUiLCJwdWJsaWNLZXkiOlt7ImlkIjoiI3ByaW1hcnkiLCJwdWJsaWNLZXlCYXNlNTgiOiJpSFVuRGZzWTh3RWY0ZnI4dm1mZ3NZN1dzOVJDTFJxd0paS0JWNVZCYzdqRyJ9XSwiYXV0aGVudGljYXRpb24iOlsiI3ByaW1hcnkiXSwiZXhwaXJlcyI6IjIwMjQtMDktMzBUMDQ6MDA6MDBaIn0","proof":{"verificationMethod":"#primary","signature":"c1ux5u6ZHGC5UkPI97ZhwYWUhwFgrIAV9AMTDl9/s07BLhZ9tZn6zTh4+VdiDA6R98HjvwzAuSIkISWTxz5N/A=="}}
)"_json;

	nlohmann::json inputJson = R"({"header":{"operation":"create","specification":"elastos/did/1.0"},"payload":"eyJleHBpcmVzIjoiMjAyNC0wMi0xMFQxNzowMDowMFoiLCJpZCI6ImRpZDplbGFzdG9zOmlaRnJoWkxldGQ2aTZxUHUyTXNZdkUyYUtyZ3c3QWY0V3ciLCJwdWJsaWNLZXkiOlt7ImNvbnRyb2xsZXIiOiIiLCJpZCI6IiNwcmltYXJ5IiwicHVibGljS2V5QmFzZTU4IjoiMjhyOEhQeXg3VWJadXdFcW5YQWpmNnJtVlJNc0p6dHRMNXpBUHZNU1pDWkJxIiwidHlwZSI6IkVDRFNBc2VjcDI1NnIxIn1dLCJ2ZXJpZmlhYmxlQ3JlZGVudGlhbCI6W3siY3JlZGVudGlhbFN1YmplY3QiOnsiYWxpcGF5IjoiYWxpcGF5QDIyMy5jb20iLCJhdmF0YXIiOiJpbWcuanBnIiwiYmlydGhkYXkiOiIyMDE5LjEwLjEyIiwiZGVzY3JpcHQiOiJ0aGlzIGlzIHNpbXBsZSBkZXNjcmlwdCIsImVtYWlsIjoidGVzdEB0ZXN0LmNvbSIsImZhY2Vib29rIjoiZmFjZWJvb2siLCJnZW5kZXIiOiJtYWxlIiwiZ29vZ2xlQWNjb3VudCI6Imdvb2dsZUBnb29nbGUuY29tIiwiaG9tZVBhZ2UiOiJob21lUGFnZSIsImlkIjoiZGlkOmVsYXN0b3M6aVpGcmhaTGV0ZDZpNnFQdTJNc1l2RTJhS3JndzdBZjRXdyIsIm1pY3Jvc29mdFBhc3Nwb3J0IjoiTWljcm9zb2Z0UGFzc3BvcnQiLCJuYW1lIjoiSDYwQ1oiLCJuYXRpb24iOiJjaGluYSIsIm5pY2tuYW1lIjoiakhvOEFCIiwicGhvbmUiOiIrODYxMzAzMjQ1NDUyMyIsInR3aXR0ZXIiOiJ0d2l0dGVyIiwid2VjaGF0Ijoid2VjaGF0MjMzMyIsIndlaWJvIjoidGVzdEBzaW5hLmNvbSJ9LCJleHBpcmF0aW9uRGF0ZSI6IiIsImlkIjoiZGlkOmVsYXN0b3M6aVpGcmhaTGV0ZDZpNnFQdTJNc1l2RTJhS3JndzdBZjRXdyIsImlzc3VhbmNlRGF0ZSI6IjIwMTktMTAtMjNUMTQ6NTU6NTVaIiwiaXNzdWVyIjoiIiwicHJvb2YiOnsic2lnbmF0dXJlIjoiIiwidHlwZSI6IiIsInZlcmlmaWNhdGlvbk1ldGhvZCI6IiJ9LCJ0eXBlcyI6WyJTZWxmUHJvY2xhaW1lZENyZWRlbnRpYWwiLCJCYXNpY1Byb2ZpbGVDcmVkZW50aWFsIiwiUGhvbmVDcmVkZW50aWFsIiwiSW50ZXJuZXRBY2NvdW50Q3JlZGVudGlhbCJdfV19","proof":{"signature":"qspTaV+B9HsBNeCvwROW4QD+7btZ5ueveTGyzx127rFRpZqz39OcSOzfir2paMRuwJaocYai3akkjLqTYm+b4w==","type":"ECDSAsecp256r1","verificationMethod":"#primary"}}
)"_json;

	std::vector<TransactionPtr> txlist;
	for (int i = 0; i < txCount; ++i) {
		TransactionPtr tx(new IDTransaction());
		tx->SetVersion(Transaction::TxVersion::Default);
		if (getRandUInt8() % 2 == 1) {
			PayloadPtr payload = PayloadPtr(new DIDInfo());

			if (getRandUInt8() % 2 == 0) {
				payload->FromJson(inputJson, 0);
			} else {
				payload->FromJson(didPayloadJSON, 0);
			}

			tx = TransactionPtr(new IDTransaction(IDTransaction::didTransaction, payload));
			tx->SetVersion(Transaction::TxVersion::Default);
		}

		tx->SetLockTime(getRandUInt32());
		tx->SetBlockHeight(i + 1);
		tx->SetTimestamp(getRandUInt32());
		tx->SetFee(getRandUInt64());

		for (size_t i = 0; i < 1; ++i) {
			InputPtr input(new TransactionInput());
			input->SetTxHash(getRanduint256());
			input->SetIndex(getRandUInt16());
			input->SetSequence(getRandUInt32());
			tx->AddInput(input);

			Address address(PrefixStandard, key.PubKey());
			ProgramPtr program(new Program(keyPath, address.RedeemScript(), bytes_t()));
			tx->AddUniqueProgram(program);
		}

		for (size_t i = 0; i < 2; ++i) {
			Address toAddress(PrefixStandard, key.PubKey());
			OutputPtr output(new TransactionOutput(10, toAddress));
			tx->AddOutput(output);
		}
		tx->FixIndex();

		uint256 md = tx->GetShaData();
		const std::vector<ProgramPtr> &programs = tx->GetPrograms();
		for (size_t i = 0; i < programs.size(); ++i) {
			bytes_t parameter = key.Sign(md);
			ByteStream stream;
			stream.WriteVarBytes(parameter);
			programs[i]->SetParameter(stream.GetBytes());
		}

		ByteStream stream;
		tx->Serialize(stream, true);
		bytes_t data = stream.GetBytes();
		std::string txHash = tx->GetHash().GetHex();

		dm.PutTransaction(iso, tx);

		txlist.push_back(tx);
	}
	REQUIRE(dm.GetAllTransactions(CHAINID_IDCHAIN).size() == txCount);

	// GetResolveDIDInfo
	TestMasterWalletManager manager("Data");
	std::vector<IMasterWallet *> masterWallets = manager.GetAllMasterWallets();
	REQUIRE(!masterWallets.empty());
	IMasterWallet *masterWallet = nullptr;
	for (size_t i = 0; i < masterWallets.size(); ++i) {
		if (masterWallets[i]->GetID() == masterWalletId) {
			masterWallet = masterWallets[i];
			break;
		}
	}
	REQUIRE(masterWallet != nullptr);
	std::vector<ISubWallet *> subWallets = masterWallet->GetAllSubWallets();
	REQUIRE(subWallets.size() == 1);
	IIDChainSubWallet *subWallet = dynamic_cast<IIDChainSubWallet *>(subWallets[0]);
	nlohmann::json didInfoList = subWallet->GetResolveDIDInfo(0, 2, "ifUQ59wFpHUKe5NZ6gjffx48sWEBt9YgQE");
	size_t count = didInfoList["DID"].size();
	REQUIRE(count == 1);

	didInfoList = subWallet->GetResolveDIDInfo(-1, 3, "");
	count = didInfoList["DID"].size();
	REQUIRE(count == 0);

	didInfoList = subWallet->GetResolveDIDInfo(0, -3, "iZFrhZLetd6i6qPu2MsYvE2aKrgw7Af4Ww");
	count = didInfoList["DID"].size();
	REQUIRE(count == 1);

	didInfoList = subWallet->GetResolveDIDInfo(-1, -2, "");
	count = didInfoList["DID"].size();
	REQUIRE(count == 0);

	didInfoList = subWallet->GetResolveDIDInfo(0, 0, "");
	count = didInfoList["DID"].size();
	REQUIRE(count == 0);

	didInfoList = subWallet->GetResolveDIDInfo(0, 300, "");
	count = didInfoList["DID"].size();
	REQUIRE(count == 2);

	manager.DestroyWallet(masterWalletId);
	REQUIRE(!boost::filesystem::exists(path));
}
