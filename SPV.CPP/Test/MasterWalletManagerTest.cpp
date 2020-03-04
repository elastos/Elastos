// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#define CATCH_CONFIG_MAIN

#include <catch.hpp>

#include <Common/ErrorChecker.h>
#include <Common/Log.h>
#include <Implement/MainchainSubWallet.h>
#include <Implement/IDChainSubWallet.h>
#include <Implement/MasterWallet.h>
#include <Database/DatabaseManager.h>
#include <WalletCore/AES.h>
#include <WalletCore/HDKeychain.h>
#include <WalletCore/Key.h>
#include <Wallet/UTXO.h>

#include <MasterWalletManager.h>

#include <climits>
#include <boost/scoped_ptr.hpp>
#include <boost/filesystem.hpp>
#include <Plugin/Transaction/Payload/CoinBase.h>

#include "TestHelper.h"

using namespace Elastos::ElaWallet;

static const std::string __rootPath = "./";
class TestMasterWalletManager : public MasterWalletManager {
public:
	TestMasterWalletManager() :
		MasterWalletManager(MasterWalletMap(), __rootPath, __rootPath) {
		_p2pEnable = false;
	}

	TestMasterWalletManager(const std::string &rootPath) :
		MasterWalletManager(rootPath) {
		_p2pEnable = false;
	}
};

static const std::string masterWalletId = "masterWalletId";
static const std::string masterWalletId2 = "masterWalletId2";
static const std::string phrasePassword = "phrasePassword";
static const std::string payPassword = "payPassword";

TEST_CASE("Master wallet manager CreateMasterWallet test", "[CreateMasterWallet]") {
	Log::registerMultiLogger();

	std::string mnemonic = "abandon abandon abandon abandon abandon abandon abandon abandon abandon abandon abandon about";
	bool singleAddress = false;

	boost::filesystem::remove_all(boost::filesystem::path(std::string(__rootPath) + masterWalletId));
	boost::filesystem::remove_all(boost::filesystem::path(std::string(__rootPath) + masterWalletId2));
	boost::filesystem::remove_all(boost::filesystem::path(std::string(__rootPath) + "MasterWalletId"));
	boost::scoped_ptr<TestMasterWalletManager> manager(new TestMasterWalletManager());

	SECTION("Normal creation") {
		IMasterWallet *masterWallet = manager->CreateMasterWallet(masterWalletId, mnemonic, phrasePassword,
																  payPassword, singleAddress);
		MasterWallet *masterWallet1 = dynamic_cast<MasterWallet *>(masterWallet);
		REQUIRE(masterWallet1 != nullptr);

		manager->DestroyWallet(masterWalletId);
		REQUIRE(!boost::filesystem::exists(std::string(__rootPath) + masterWalletId));
	}

	SECTION("Create with phrase password can be empty") {
		IMasterWallet *masterWallet = manager->CreateMasterWallet(masterWalletId, mnemonic, "",
																  payPassword, singleAddress);
		manager->DestroyWallet(masterWallet->GetID());
		REQUIRE(!boost::filesystem::exists(std::string(__rootPath) + masterWalletId));
	}

	REQUIRE_THROWS(manager->CreateMasterWallet("", mnemonic, phrasePassword, payPassword, singleAddress));
	REQUIRE_THROWS(manager->CreateMasterWallet(masterWalletId, "", phrasePassword, payPassword, singleAddress));

	REQUIRE_THROWS(manager->CreateMasterWallet(masterWalletId, mnemonic, "ilegal", payPassword, singleAddress));

	REQUIRE_THROWS(manager->CreateMasterWallet(masterWalletId, mnemonic,
											   "1234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890",
											   payPassword, singleAddress));

	REQUIRE_THROWS(manager->CreateMasterWallet(masterWalletId, mnemonic, phrasePassword, "", singleAddress));
	REQUIRE_THROWS(manager->CreateMasterWallet(masterWalletId, mnemonic, phrasePassword, "ilegal", singleAddress));

	REQUIRE_THROWS(manager->CreateMasterWallet(masterWalletId, mnemonic, phrasePassword,
											   "1234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890",
											   singleAddress));
}


TEST_CASE("GetAllMasterWallets", "[MasterWalletManager]") {
	boost::scoped_ptr<TestMasterWalletManager> manager(new TestMasterWalletManager());

	std::string mnemonic = "";
	std::string mnemonic2 = "";
	bool singleAddress = false;
	//boost::scoped_ptr<IMasterWallet> masterWallet;
	IMasterWallet *masterWallet = nullptr;
	IMasterWallet *masterWallet2 = nullptr;

	std::vector<IMasterWallet *> masterWallets;
	masterWallets = manager->GetAllMasterWallets();
	REQUIRE(masterWallets.empty());

	mnemonic = MasterWallet::GenerateMnemonic("english", __rootPath);
	masterWallet = manager->CreateMasterWallet(masterWalletId, mnemonic, phrasePassword, payPassword, singleAddress);
	REQUIRE(masterWallet != nullptr);

	masterWallets = manager->GetAllMasterWallets();
	REQUIRE(masterWallets.size() == 1);
	REQUIRE(masterWallets[0] == masterWallet);
	REQUIRE(masterWallet->GetAllSubWallets().size() == 0);

	mnemonic2 = MasterWallet::GenerateMnemonic("english", __rootPath);
	masterWallet2 = manager->CreateMasterWallet(masterWalletId2, mnemonic2, phrasePassword, payPassword, singleAddress);
	REQUIRE(masterWallet2 != nullptr);

	masterWallets = manager->GetAllMasterWallets();
	REQUIRE(masterWallets.size() == 2);

	REQUIRE_NOTHROW(manager->DestroyWallet(masterWalletId));
	REQUIRE_NOTHROW(manager->DestroyWallet(masterWalletId2));
	REQUIRE(!boost::filesystem::exists(std::string(__rootPath) + masterWalletId));
	REQUIRE(!boost::filesystem::exists(std::string(__rootPath) + masterWalletId2));
}

TEST_CASE("Wallet Import/Export method", "[Import]") {
	boost::scoped_ptr<TestMasterWalletManager> manager(new TestMasterWalletManager());
	std::string phrasePassword = "phrasePassword";
	std::string payPassword = "payPassword";
	std::string backupPassword = "backupPassword";
	bool singleAddress = false;

	std::string mnemonic = "separar sopa resto fraude tinta ánimo diseño misa nube sardina tóxico turbina";

	IMasterWallet *masterWallet = manager->ImportWalletWithMnemonic(
		masterWalletId, mnemonic, "", payPassword, singleAddress);
	REQUIRE(masterWallet != nullptr);
	REQUIRE_NOTHROW(manager->DestroyWallet(masterWalletId));
	masterWallet = nullptr;

	// Import mnemonic
	REQUIRE_THROWS(manager->ImportWalletWithMnemonic(masterWalletId, mnemonic, phrasePassword,
													 "1234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890",
													 singleAddress));
	REQUIRE_THROWS(manager->ImportWalletWithMnemonic(masterWalletId, mnemonic, phrasePassword, "il", singleAddress));
	REQUIRE_THROWS(manager->ImportWalletWithMnemonic(masterWalletId, mnemonic, phrasePassword, "", singleAddress));
	REQUIRE_THROWS(manager->ImportWalletWithMnemonic(masterWalletId, mnemonic,
													 "1234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890",
													 payPassword, singleAddress));
	REQUIRE_THROWS(manager->ImportWalletWithMnemonic(masterWalletId, mnemonic, "il", payPassword, singleAddress));
	REQUIRE_THROWS(manager->ImportWalletWithMnemonic("", mnemonic, phrasePassword, payPassword, singleAddress));
	masterWallet = manager->ImportWalletWithMnemonic(masterWalletId, mnemonic, phrasePassword, payPassword, singleAddress);

	REQUIRE(masterWallet != nullptr);
	REQUIRE(nullptr != masterWallet->CreateSubWallet("ELA"));

	// Export keystore
	REQUIRE_THROWS(masterWallet->ExportKeystore(backupPassword, "WrongPayPassword"));
	REQUIRE_THROWS(masterWallet->ExportKeystore(backupPassword, ""));
	REQUIRE_THROWS(masterWallet->ExportKeystore("", payPassword));
	nlohmann::json keystoreContent = masterWallet->ExportKeystore(backupPassword, payPassword);

	// Export mnemonic
	REQUIRE_THROWS(masterWallet->ExportMnemonic(""));
	REQUIRE_THROWS(masterWallet->ExportMnemonic("ilegal"));
	REQUIRE_THROWS(masterWallet->ExportMnemonic(
		"1234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890"));
	REQUIRE(mnemonic == masterWallet->ExportMnemonic(payPassword));

	// Import keystore
	REQUIRE_THROWS(manager->ImportWalletWithKeystore(masterWalletId2, keystoreContent, "", payPassword));
	REQUIRE_THROWS(manager->ImportWalletWithKeystore(masterWalletId2, keystoreContent, backupPassword, ""));
	REQUIRE_THROWS(manager->ImportWalletWithKeystore(masterWalletId2, keystoreContent, backupPassword, payPassword));

	manager->DestroyWallet(masterWalletId);
	REQUIRE(!boost::filesystem::exists(std::string(__rootPath) + masterWalletId));
}

TEST_CASE("Wallet GetBalance test", "[GetBalance]") {
	std::string path = __rootPath + masterWalletId + "/";
	boost::filesystem::remove_all(path);
	boost::filesystem::create_directories(path);

	// prepare wallet data
	std::string keyPath = "44'/0'/0'/0/0";
	LocalStore ls(nlohmann::json::parse(
		"{\"account\":0,\"coinInfo\":[{\"ChainID\":\"ELA\",\"EarliestPeerTime\":1513936800,\"FeePerKB\":10000,\"VisibleAssets\":[\"a3d0eaa466df74983b5d7c543de6904f4c9418ead5ffd6d25814234a96db37b0\"]}],\"derivationStrategy\":\"BIP44\",\"m\":1,\"mnemonic\":\"P0C/7w2/h13rLqA8qgI+BwwDZrcK9g8ixjdPFFIEC6+G62Qsm4WsmoNbxJE+shQ2jy7tTsPsDYLKCow9hsGrWchJuBV5ULwwcnRhYimP9TlAYA6uTdk3aQgolw==\",\"mnemonicHasPassphrase\":true,\"n\":1,\"ownerPubKey\":\"027c876ac77226d6f25d198983b1ae58baa39b136ff0e09386e064b40d646767a1\",\"passphrase\":\"\",\"publicKeyRing\":[{\"requestPubKey\":\"027d917aa4732ebffcb496a40cce2bf5b57237570c106b97b98fa5be433c6b743d\",\"xPubKey\":\"xpub6CWoR5hv1BestMgnyWLPR1RXdttXhFLK9vTjri9J79SgYdCnpjTCNF5JiwyZXsaW4pMonQ8gaHWv5xUi9DgBLMzdWE75EULLzU444PkpF7E\"}],\"readonly\":false,\"requestPrivKey\":\"HiFHrGoxzoY3yCbshyUtMtY1fIO2rFw5265BALZgv08Vuen9c1llzg==\",\"requestPubKey\":\"027d917aa4732ebffcb496a40cce2bf5b57237570c106b97b98fa5be433c6b743d\",\"singleAddress\":false,\"xPrivKey\":\"SK1ian/e9X2YQdBdioAjo/P11xkGlaXp9AFXcSIYUmPuQwz8aepAkk4hUG7KfqqEtzwb4kOTt0Tm+ZiYiRXtLmVkaVLMaQD1ab49rIHlRj9zw2fSft8=\",\"xPubKey\":\"xpub6CWoR5hv1BestMgnyWLPR1RXdttXhFLK9vTjri9J79SgYdCnpjTCNF5JiwyZXsaW4pMonQ8gaHWv5xUi9DgBLMzdWE75EULLzU444PkpF7E\"}"));
	ls.SaveTo(path);

	DatabaseManager dm(__rootPath + masterWalletId + "/ELA.db");

	std::string xprv = ls.GetxPrivKey();
	bytes_t bytes = AES::DecryptCCM(xprv, payPassword);
	Key key = HDKeychain(bytes).getChild(keyPath);
	Address addr(PrefixStandard, key.PubKey());

	std::vector<UTXOEntity> utxoEntities;
	int txCount = 20;
	std::vector<TransactionPtr> txlist;
	for (int i = 0; i < txCount; ++i) {
		TransactionPtr tx(new Transaction());
		tx->SetVersion(Transaction::TxVersion::Default);
		tx->SetLockTime(getRandUInt32());
		tx->SetBlockHeight(i + 100);
		tx->SetTimestamp(getRandUInt32());
		tx->SetPayloadVersion(getRandUInt8());
		tx->SetFee(getRandUInt64());

		InputPtr input(new TransactionInput());
		input->SetTxHash(getRanduint256());
		input->SetIndex(getRandUInt16());
		input->SetSequence(getRandUInt32());
		tx->AddInput(input);
		ProgramPtr program(new Program(keyPath, addr.RedeemScript(), bytes_t()));
		tx->AddUniqueProgram(program);

		for (size_t j = 0; j < 20; ++j) {
			OutputPtr output(new TransactionOutput(10, addr));
			tx->AddOutput(output);
		}
		tx->FixIndex();

		uint256 md = tx->GetShaData();
		for (const ProgramPtr &p : tx->GetPrograms()) {
			bytes_t parameter = key.Sign(md);
			ByteStream stream;
			stream.WriteVarBytes(parameter);
			p->SetParameter(stream.GetBytes());
		}

		ByteStream stream;
		tx->Serialize(stream, true);
		bytes_t data = stream.GetBytes();
		std::string txHash = tx->GetHash().GetHex();
		utxoEntities.clear();
		for (const OutputPtr &o : tx->GetOutputs())
			utxoEntities.emplace_back(txHash, o->FixedIndex());

		dm.PutNormalTxn(tx);
		dm.PutUTXOs(utxoEntities);

		txlist.push_back(tx);
	}

	REQUIRE(dm.GetNormalTxns(CHAINID_MAINCHAIN).size() == txCount);
	REQUIRE(dm.GetUTXOs().size() == 20 * txCount);

	//transfer to another address
	BigInt transferAmount(2005);
	BigInt totalInput(0);
	TransactionPtr tx(new Transaction());
	tx->SetVersion(Transaction::TxVersion::Default);
	tx->SetLockTime(getRandUInt32());
	tx->SetBlockHeight(1000);
	tx->SetTimestamp(getRandUInt32());
	tx->SetPayloadVersion(getRandUInt8());
	tx->SetFee(getRandUInt64());

	std::vector<UTXOEntity> utxoRemoved;
	for (size_t i = 0; i < txCount && totalInput < transferAmount; ++i) {
		for (size_t j = 0; j < txlist[i]->GetOutputs().size() && totalInput < transferAmount; ++j) {
			InputPtr input(new TransactionInput());
			input->SetTxHash(txlist[i]->GetHash());
			input->SetIndex(j);
			input->SetSequence(getRandUInt32());
			tx->AddInput(input);
			utxoRemoved.emplace_back(txlist[i]->GetHash().GetHex(), txlist[i]->GetOutputs()[j]->FixedIndex());

			totalInput += txlist[i]->GetOutputs()[j]->Amount();

			Address address(PrefixStandard, key.PubKey());
			ProgramPtr program(new Program(keyPath, address.RedeemScript(), bytes_t()));
			tx->AddUniqueProgram(program);
		}
	}
	Address toAddress("Ed8ZSxSB98roeyuRZwwekrnRqcgnfiUDeQ");
	tx->AddOutput(OutputPtr(new TransactionOutput(transferAmount, toAddress)));
	int fee = 100;
	BigInt change = totalInput - transferAmount - fee;
	if (change > 0) {
		tx->AddOutput(OutputPtr(new TransactionOutput(change, addr)));
	}
	tx->FixIndex();

	uint256 md = tx->GetShaData();
	for (const ProgramPtr &p : tx->GetPrograms()) {
		bytes_t parameter = key.Sign(md);
		ByteStream stream;
		stream.WriteVarBytes(parameter);
		p->SetParameter(stream.GetBytes());
	}

	ByteStream stream;
	tx->Serialize(stream, true);
	bytes_t data = stream.GetBytes();
	std::string txHash = tx->GetHash().GetHex();

	if (tx->GetOutputs().size() > 1) {
		utxoEntities.clear();
		utxoEntities.emplace_back(txHash, 1);
		REQUIRE(dm.PutUTXOs(utxoEntities));
	}

	REQUIRE(dm.DeleteUTXOs(utxoRemoved));
	REQUIRE(dm.PutNormalTxn(tx));

	REQUIRE(dm.GetNormalTxns(CHAINID_MAINCHAIN).size() == txCount + 1);

	//put coinbase tx
	utxoEntities.clear();
	for (int i = 0; i < txCount; ++i) {
		TransactionPtr txn(new Transaction(Transaction::coinBase, PayloadPtr(new CoinBase())));
		OutputPtr o(new TransactionOutput(10, addr));
		txn->AddOutput(o);
		txn->SetBlockHeight(4000 + i);
		std::string nonce = std::to_string((std::rand() & 0xFFFFFFFF));
		txn->AddAttribute(AttributePtr(new Attribute(Attribute::Nonce, bytes_t(nonce.c_str(), nonce.size()))));
		txHash = txn->GetHash().GetHex();
		utxoEntities.emplace_back(txHash, 0);
		REQUIRE(dm.PutCoinbaseTxn(txn));
	}
	REQUIRE(dm.PutUTXOs(utxoEntities));
	REQUIRE(dm.GetCoinbaseTotalCount() == txCount);
	utxoEntities.clear();
	for (int i = 0; i < txCount; ++i) {
		TransactionPtr txn(new Transaction(Transaction::coinBase, PayloadPtr(new CoinBase())));
		OutputPtr o(new TransactionOutput(10, addr));
		txn->AddOutput(o);
		txn->SetBlockHeight(4032 - 101 - i);
		std::string nonce = std::to_string((std::rand() & 0xFFFFFFFF));
		txn->AddAttribute(AttributePtr(new Attribute(Attribute::Nonce, bytes_t(nonce.c_str(), nonce.size()))));
		txHash = txn->GetHash().GetHex();
		utxoEntities.emplace_back(txHash, 0);
		dm.PutCoinbaseTxn(txn);
	}
	REQUIRE(dm.PutUTXOs(utxoEntities));
	REQUIRE(dm.GetCoinbaseTotalCount() == 2 * txCount);

	// put merkle block
	MerkleBlockPtr block = Registry::Instance()->CreateMerkleBlock("ELA");
	block->SetHeight(4032);
	block->SetHash(getRanduint256());
	block->SetTimestamp(getRandUInt32());
	block->SetTarget(getRandUInt32());
	dm.PutMerkleBlock(block);
	dm.flush();

	// verify wallet balance
	TestMasterWalletManager manager(__rootPath);
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
	ISubWallet *subWallet = subWallets[0];

	std::string balance = subWallet->GetBalance();
	REQUIRE(balance == "2190");

	nlohmann::json balanceInfo = subWallet->GetBalanceInfo();
	REQUIRE(balanceInfo.size() == 1);
	balanceInfo = balanceInfo[0]["Summary"];
	REQUIRE(balanceInfo["LockedBalance"].get<std::string>() == "200");
	REQUIRE(balanceInfo["Balance"].get<std::string>() == "2190");
	REQUIRE(balanceInfo["DepositBalance"].get<std::string>() == "0");
	REQUIRE(balanceInfo["PendingBalance"].get<std::string>() == "0");
	REQUIRE(balanceInfo["SpendingBalance"].get<std::string>() == "0");
	REQUIRE(balanceInfo["VotedBalance"].get<std::string>() == "0");

	manager.DestroyWallet(masterWalletId);
	boost::filesystem::path masterWalletPath = path;
	REQUIRE(!boost::filesystem::exists(masterWalletPath));
}

