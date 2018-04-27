// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#define CATCH_CONFIG_MAIN

#include <fstream>

#include "TransactionDataStore.h"
#include "DatabaseManager.h"
#include "catch.hpp"
#include "Manager/BackgroundExecutor.h"
#include "Utils.h"
#include "Log.h"

using namespace Elastos::SDK;
const int totalThreadCount = 200;
const int loopCount = 5;
int thread_call_count = 0;

TEST_CASE("DatabaseManager Contructor test", "[DatabaseManager]") {

	SECTION("Contruct default test ") {
		DatabaseManager *databaseManager = new DatabaseManager();
		REQUIRE(databaseManager != nullptr);

		std::fstream file;
		file.open("wallet.db", std::ios::in);
		REQUIRE(file.is_open() == true);
		file.close();
	}

	SECTION("Construct with database path test") {
		std::fstream file;
		const char *testDb = "testData.db";
		file.open(testDb, std::ios::in);
		if (file) {
			remove(testDb);
		}
		file.close();

		DatabaseManager *databaseManager = new DatabaseManager(testDb);
		REQUIRE(databaseManager != nullptr);

		file.open(testDb, std::ios::in);
		REQUIRE(file.is_open() == true);
		file.close();
	}
}

TEST_CASE("DatabaseManager database transaction test", "[DatabaseManager]") {

	DatabaseManager databaseManager;
	std::string iso = "ELA";

	SECTION("DatabaseManager putTransaction test") {
		TransactionEntity transactionEntity;
		uint8_t s[21] = {33, 110, 179, 17, 41, 134, 242, 38, 145, 166, 17, 187, 37, 147, 24,
		                 60, 75, 8, 182, 57, 98};
		transactionEntity.buff = ByteData(s, 21);
		transactionEntity.blockHeight = 9000;
		transactionEntity.timeStamp = 1523935043;
		transactionEntity.txHash = "000000000019d6689c085ae165831e934ff763ae46a2a6c172b3f1b60a8ce26f";
		bool result = databaseManager.putTransaction(iso, transactionEntity);
		REQUIRE(result == true);

		std::vector<TransactionEntity> list = databaseManager.getAllTransactions(iso);
		ssize_t len = list.size();
		REQUIRE(len > 0);
		REQUIRE(list[len - 1].blockHeight == transactionEntity.blockHeight);
		REQUIRE(list[len - 1].timeStamp == transactionEntity.timeStamp);
		REQUIRE(list[len - 1].txHash == transactionEntity.txHash);
		REQUIRE(list[len - 1].buff.length == transactionEntity.buff.length);
		for (int i = 0; i < transactionEntity.buff.length; i++) {
			REQUIRE(list[len - 1].buff.data[i] == transactionEntity.buff.data[i]);
		}
	}

	SECTION("DatabaseManager updateTransaction test") {
		TransactionEntity transactionEntity;
		uint8_t s[20] = {43, 34, 80, 17, 41, 30, 242, 38, 145, 166, 17, 187, 37, 147, 24,
		                 60, 75, 8, 182, 76};
		transactionEntity.buff = ByteData(s, 20);
		transactionEntity.blockHeight = 8000;
		transactionEntity.timeStamp = 1523935888;
		transactionEntity.txHash = "000000000019d6689c085ae165831e934ff763ae46a2a6c172b3f1b60a8ce26f";

		bool result = databaseManager.updateTransaction(iso, transactionEntity);
		REQUIRE(result == true);

		std::vector<TransactionEntity> list = databaseManager.getAllTransactions(iso);
		ssize_t len = list.size();
		REQUIRE(len > 0);
		REQUIRE(list[len - 1].blockHeight == transactionEntity.blockHeight);
		REQUIRE(list[len - 1].timeStamp == transactionEntity.timeStamp);
		REQUIRE(list[len - 1].txHash == transactionEntity.txHash);
		REQUIRE(list[len - 1].buff.length == 20);
		for (int i = 0; i < transactionEntity.buff.length; i++) {
			REQUIRE(list[len - 1].buff.data[i] == transactionEntity.buff.data[i]);
		}
	}

	SECTION("DatabaseManager deleteTxByHash test") {
		std::vector<TransactionEntity> list = databaseManager.getAllTransactions(iso);
		ssize_t len = list.size();

		bool result = databaseManager.deleteTxByHash(iso, "aa34r");
		REQUIRE(result == true);

		list = databaseManager.getAllTransactions(iso);
		REQUIRE(list.size() == len);

		result = databaseManager.deleteTxByHash(iso,
		                                        "000000000019d6689c085ae165831e934ff763ae46a2a6c172b3f1b60a8ce26f");
		REQUIRE(result == true);

		list = databaseManager.getAllTransactions(iso);
		REQUIRE(list.size() < len);

		result = databaseManager.deleteAllTransactions(iso);
		REQUIRE(result == true);

		list = databaseManager.getAllTransactions(iso);
		REQUIRE(list.size() == 0);
	}
}

TEST_CASE("DatabaseManager peer interface test", "[DatabaseManager]") {

	DatabaseManager databaseManager;
	std::string iso = "ELA";

	SECTION("DatabaseManager putPeer test") {
		PeerEntity peerEntity;
		peerEntity.id = 1;
		peerEntity.timeStamp = 1523935888;
		peerEntity.address = ((UInt128) {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0xff, 0xff, 0x7f, 0x00, 0x00, 0x01});
		peerEntity.port = 9090;

		bool result = databaseManager.putPeer(iso, peerEntity);
		REQUIRE(result == true);

		std::vector<PeerEntity> list = databaseManager.getAllPeers(iso);
		ssize_t len = list.size();
		REQUIRE(len > 0);
		REQUIRE(list[len - 1].id >= 1);
		REQUIRE(list[len - 1].timeStamp == peerEntity.timeStamp);
		REQUIRE(list[len - 1].port == peerEntity.port);
		int res = UInt128Eq(&list[len - 1].address, &peerEntity.address);
		REQUIRE(res == 1);
	}

	SECTION("DatabaseManager deleteAllPeers and putPeers test") {
		bool result = databaseManager.deleteAllPeers(iso);
		REQUIRE(result == true);

		std::vector<PeerEntity> list = databaseManager.getAllPeers(iso);
		ssize_t len = list.size();
		REQUIRE(len == 0);

		list.clear();
		PeerEntity peerEntity;
		for (int i = 0; i < 10; i++) {
			peerEntity.timeStamp = uint64_t(1523935888 + i);
			peerEntity.address = ((UInt128) {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0xff, 0xff, 0x7f, 0x00, 0x00, 0x01});
			peerEntity.port = uint16_t(8080 + i);
			list.push_back(peerEntity);
		}

		result = databaseManager.putPeers(iso, list);
		REQUIRE(result == true);

		std::vector<PeerEntity> peerList = databaseManager.getAllPeers(iso);
		REQUIRE(peerList.size() == list.size());

		for (int i = 0; i < peerList.size(); i++) {
			REQUIRE(peerList[i].id > 0);
			REQUIRE(peerList[i].timeStamp == list[i].timeStamp);
			REQUIRE(peerList[i].port == list[i].port);
			int res = UInt128Eq(&peerList[i].address, &list[i].address);
			REQUIRE(res == 1);
		}
	}

	SECTION("DatabaseManager deletePeer test") {
		std::vector<PeerEntity> list = databaseManager.getAllPeers(iso);
		ssize_t len = list.size();
		REQUIRE(len > 0);

		PeerEntity peerEntity;
		bool result = databaseManager.deletePeer(iso, peerEntity);
		REQUIRE(result == true);

		list = databaseManager.getAllPeers(iso);
		REQUIRE(list.size() == len);

		peerEntity.id = list[len - 1].id;
		result = databaseManager.deletePeer(iso, peerEntity);
		REQUIRE(result == true);

		list = databaseManager.getAllPeers(iso);
		REQUIRE(list.size() == len - 1);
	}
}

TEST_CASE("DatabaseManager merkleBlock test", "[DatabaseManager]") {

	DatabaseManager databaseManager;
	std::string iso = "ELA";

	SECTION("DatabaseManager putMerkleBlock test") {
		MerkleBlockEntity blockEntity;
		blockEntity.blockHeight = 8888;
		blockEntity.blockBytes.length = 21;
		uint8_t s[21] = {33, 110, 179, 17, 41, 134, 242, 38, 145, 166, 17, 187, 37, 147, 24,
		                 60, 75, 8, 182, 57, 98};
		blockEntity.blockBytes.data = s;

		bool result = databaseManager.putMerkleBlock(iso, blockEntity);
		REQUIRE(result == true);

		std::vector<MerkleBlockEntity> list = databaseManager.getAllMerkleBlocks(iso);
		ssize_t len = list.size();
		REQUIRE(len > 0);
		REQUIRE(list[len - 1].id > 0);
		REQUIRE(list[len - 1].blockHeight == blockEntity.blockHeight);
		REQUIRE(list[len - 1].blockBytes.length == blockEntity.blockBytes.length);
		for (int i = 0; i < list[len - 1].blockBytes.length; i++) {
			REQUIRE(list[len - 1].blockBytes.data[i] == blockEntity.blockBytes.data[i]);
		}
	}

	SECTION("DatabaseManager deleteAllBlocks and putMerkleBlocks test") {
		bool result = databaseManager.deleteAllBlocks(iso);
		REQUIRE(result == true);

		std::vector<MerkleBlockEntity> list = databaseManager.getAllMerkleBlocks(iso);
		ssize_t len = list.size();
		REQUIRE(len == 0);

		list.clear();
		uint8_t s[21] = {33, 110, 179, 17, 41, 134, 242, 38, 145, 166, 17, 187, 37, 147, 24,
		                 60, 75, 8, 182, 57, 98};
		for (int i = 0; i < 10; i++) {
			MerkleBlockEntity blockEntity;
			blockEntity.blockBytes.length = 21;
			blockEntity.blockBytes.data = s;
			blockEntity.blockHeight = uint32_t(i + 1);
			list.push_back(blockEntity);
		}

		result = databaseManager.putMerkleBlocks(iso, list);
		REQUIRE(result == true);

		std::vector<MerkleBlockEntity> blockList = databaseManager.getAllMerkleBlocks(iso);
		REQUIRE(blockList.size() == list.size());
		for (int i = 0; i < list.size(); i++) {
			REQUIRE(blockList[i].id > 0);
			REQUIRE(blockList[i].blockHeight == list[i].blockHeight);
			REQUIRE(blockList[i].blockBytes.length == list[i].blockBytes.length);
			for (int j = 0; j < blockList[i].blockBytes.length; j++) {
				REQUIRE(blockList[i].blockBytes.data[j] == list[i].blockBytes.data[j]);
			}
		}
	}

	SECTION("DatabaseManager deleteMerkleBlock test") {
		std::vector<MerkleBlockEntity> list = databaseManager.getAllMerkleBlocks(iso);
		ssize_t len = list.size();
		REQUIRE(len > 0);

		MerkleBlockEntity blockEntity;
		bool result = databaseManager.deleteMerkleBlock(iso, blockEntity);
		REQUIRE(result == true);

		list = databaseManager.getAllMerkleBlocks(iso);
		REQUIRE(list.size() == len);

		blockEntity.id = list[len - 1].id;
		result = databaseManager.deleteMerkleBlock(iso, blockEntity);
		REQUIRE(result == true);

		list = databaseManager.getAllMerkleBlocks(iso);
		REQUIRE(list.size() == len - 1);
	}
}

void onPutTransaction(void *arg, int index) {
	DatabaseManager *databaseManager = (DatabaseManager *) arg;

	TransactionEntity transactionEntity;
	uint8_t s[21] = {33, 110, 179, 17, 41, 134, 242, 38, 145, 166, 17, 187, 37, 147, 24,
	                 60, 75, 8, 182, 57, 98};
	for (int i = 0; i < loopCount; i++) {
		transactionEntity.buff = ByteData(s, 21);
		transactionEntity.blockHeight = uint32_t(9000 + index * loopCount + i);
		transactionEntity.timeStamp = uint32_t(15000000 + index * loopCount + i);
		transactionEntity.txHash = std::to_string(index * loopCount + i + 1);
		bool result = databaseManager->putTransaction("ELA", transactionEntity);
		REQUIRE(result == true);
	}
	thread_call_count++;
}

void onDeleteTransaction(void *arg, int index) {
	DatabaseManager *databaseManager = (DatabaseManager *) arg;
	for (int i = 0; i < loopCount; i++) {
		bool result = databaseManager->deleteTxByHash("ELA", std::to_string(index * loopCount + i + 1));
		REQUIRE(result == true);
	}

	thread_call_count++;
}

void onUpdateTransaction(void *arg, int index) {
	DatabaseManager *databaseManager = (DatabaseManager *) arg;

	TransactionEntity transactionEntity;
	uint8_t s[20] = {43, 34, 80, 17, 41, 30, 242, 38, 145, 166, 17, 187, 37, 147, 24,
	                 60, 75, 8, 182, 76};
	for (int i = 0; i < loopCount; i++) {
		transactionEntity.buff = ByteData(s, 20);
		transactionEntity.blockHeight = 8000 + index * loopCount + i;
		transactionEntity.timeStamp = 1523935000 + index * loopCount + i;
		transactionEntity.txHash = std::to_string(index * loopCount + i + 1);
		bool result = databaseManager->updateTransaction("ELA", transactionEntity);
		REQUIRE(result == true);
	}

	thread_call_count++;
}

void onDeleteAllTransactions(void *arg) {
	DatabaseManager *databaseManager = (DatabaseManager *) arg;
	bool result = databaseManager->deleteAllTransactions("ELA");
	REQUIRE(result == true);

	std::vector<TransactionEntity> list = databaseManager->getAllTransactions("ELA");
	REQUIRE(list.size() == 0);
}

bool sortTransactionList(TransactionEntity &a, TransactionEntity &b) {
	return a.blockHeight < b.blockHeight;
}

TEST_CASE("DatabaseManager mulity thread transaction", "[DatabaseManager]") {
	DatabaseManager databaseManager;

	SECTION("DatabaseManager deleteAllTransactions test") {
		BackgroundExecutor executor((uint8_t) totalThreadCount);
		for (int i = 0; i < totalThreadCount; i++) {
			executor.execute(Runnable(std::bind(&onDeleteAllTransactions, &databaseManager)));
		}
	}

	SECTION("DatabaseManager putTransaction thread test") {
		bool result = databaseManager.deleteAllTransactions("ELA");
		REQUIRE(result == true);

		BackgroundExecutor executor((uint8_t) totalThreadCount);

		for (int i = 0; i < totalThreadCount; i++) {
			executor.execute(Runnable(std::bind(&onPutTransaction, &databaseManager, i)));
		}

		while (true) {
			if (thread_call_count >= totalThreadCount) {
				break;
			}
		}

		std::vector<TransactionEntity> list = databaseManager.getAllTransactions("ELA");
		std::sort(list.begin(), list.end(), sortTransactionList);
		uint8_t s[21] = {33, 110, 179, 17, 41, 134, 242, 38, 145, 166, 17, 187, 37, 147, 24,
		                 60, 75, 8, 182, 57, 98};
		for (int i = 0; i < totalThreadCount * loopCount; i++) {
			REQUIRE(list[i].buff.length == 21);
			for (int j = 0; j < 21; j++) {
				REQUIRE(list[i].buff.data[j] == s[j]);
			}
			REQUIRE(list[i].blockHeight == 9000 + i);
			REQUIRE(list[i].timeStamp == 15000000 + i);
			REQUIRE(list[i].txHash == std::to_string(i + 1));
		}
	}

	SECTION("DatabaseManager updateTransaction thread test") {
		BackgroundExecutor executor((uint8_t) totalThreadCount);
		thread_call_count = 0;
		for (int i = 0; i < totalThreadCount; i++) {
			executor.execute(Runnable(std::bind(&onUpdateTransaction, &databaseManager, i)));
		}

		while (true) {
			if (thread_call_count >= totalThreadCount) {
				break;
			}
		}

		std::vector<TransactionEntity> list = databaseManager.getAllTransactions("ELA");
		std::sort(list.begin(), list.end(), sortTransactionList);
		uint8_t s[20] = {43, 34, 80, 17, 41, 30, 242, 38, 145, 166, 17, 187, 37, 147, 24,
		                 60, 75, 8, 182, 76};
		for (int i = 0; i < totalThreadCount * loopCount; i++) {
			REQUIRE(list[i].buff.length == 20);
			for (int j = 0; j < 20; j++) {
				REQUIRE(list[i].buff.data[j] == s[j]);
			}
			REQUIRE(list[i].blockHeight == 8000 + i);
			REQUIRE(list[i].timeStamp == 1523935000 + i);
			REQUIRE(list[i].txHash == std::to_string(i + 1));
		}
	}

	SECTION("DatabaseManager deleteTxByHash thread test") {
		BackgroundExecutor executor((uint8_t) totalThreadCount);
		thread_call_count = 0;
		for (int i = 0; i < totalThreadCount; i++) {
			executor.execute(Runnable(std::bind(&onDeleteTransaction, &databaseManager, i)));
		}

		while (true) {
			if (thread_call_count >= totalThreadCount) {
				break;
			}
		}

		std::vector<TransactionEntity> list = databaseManager.getAllTransactions("ELA");
		REQUIRE(list.size() == 0);
	}

}

void onPutPeers(void *arg, int index) {
	DatabaseManager *databaseManager = (DatabaseManager *) arg;

	std::vector<PeerEntity> list;
	PeerEntity peerEntity;
	for (int i = 0; i < loopCount; i++) {
		peerEntity.timeStamp = uint64_t(index * loopCount + i);
		peerEntity.address = ((UInt128) {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0xff, 0xff, 0x7f, 0x00, 0x00, 0x01});
		peerEntity.port = uint16_t(index * loopCount + i);
		list.push_back(peerEntity);
	}
	for (int i = 0; i < loopCount; i++) {
		bool result = databaseManager->putPeers("elas", list);
		REQUIRE(result == true);
	}
	thread_call_count++;
}

void onPutPeer(void *arg, int index) {
	DatabaseManager *databaseManager = (DatabaseManager *) arg;
	PeerEntity peerEntity;
	for (int i = 0; i < loopCount; i++) {
		peerEntity.timeStamp = uint64_t(index * loopCount + i);
		peerEntity.address = ((UInt128) {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0xff, 0xff, 0x7f, 0x00, 0x00, 0x01});
		peerEntity.port = uint16_t(index * loopCount + i);;
		bool result = databaseManager->putPeer("ela", peerEntity);
		REQUIRE(result == true);
	}
	thread_call_count++;
}

void onDeletePeer(void *arg, int index) {
	DatabaseManager *databaseManager = (DatabaseManager *) arg;
	PeerEntity peerEntity;
	peerEntity.id = index;
	bool result = databaseManager->deletePeer("ela", peerEntity);
	REQUIRE(result == true);
	thread_call_count++;
}

void onDeletePeers(void *arg, int index) {
	DatabaseManager *databaseManager = (DatabaseManager *) arg;
	PeerEntity peerEntity;
	peerEntity.id = index;
	bool result = databaseManager->deletePeer("elas", peerEntity);
	REQUIRE(result == true);
	thread_call_count++;
}

void onDeleteAllPeers(void *arg) {
	DatabaseManager *databaseManager = (DatabaseManager *) arg;
	databaseManager->deleteAllPeers("ela");
	databaseManager->deleteAllPeers("elas");
	thread_call_count++;
}

bool sortPeers(PeerEntity &a, PeerEntity &b) {
	return a.timeStamp < b.timeStamp;
}

TEST_CASE("DatabaseManager mulity thread peer", "[DatabaseManager]") {
	DatabaseManager databaseManager;

	SECTION("DatabaseManager deleteAllTransactions test") {
		BackgroundExecutor executor((uint8_t) totalThreadCount);
		thread_call_count = 0;
		for (int i = 0; i < totalThreadCount; i++) {
			executor.execute(Runnable(std::bind(&onDeleteAllPeers, &databaseManager)));
		}

		while (1) {
			if (thread_call_count >= totalThreadCount) {
				break;
			}
		}

		std::vector<PeerEntity> list = databaseManager.getAllPeers("ela");
		REQUIRE(list.size() == 0);
		list = databaseManager.getAllPeers("elas");
		REQUIRE(list.size() == 0);
	}

	SECTION("DatabaseManager putPeer thread test") {
		bool result = databaseManager.deleteAllPeers("ela");
		REQUIRE(result == true);
		thread_call_count = 0;
		BackgroundExecutor executor((uint8_t) totalThreadCount);
		for (int i = 0; i < totalThreadCount; i++) {
			executor.execute(Runnable(std::bind(&onPutPeer, &databaseManager, i)));
		}

		while (1) {
			if (thread_call_count >= totalThreadCount) {
				break;
			}
		}

		std::vector<PeerEntity> list = databaseManager.getAllPeers("ela");
		std::sort(list.begin(), list.end(), sortPeers);
		UInt128 addr = ((UInt128) {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0xff, 0xff, 0x7f, 0x00, 0x00, 0x01});
		for (int i = 0; i < totalThreadCount * loopCount; i++) {
			REQUIRE(list[i].timeStamp == i);
			REQUIRE(list[i].port == i);
			int res = UInt128Eq(&list[i].address, &addr);
			REQUIRE(res == 1);
		}
	}

	SECTION("DatabaseManager putPeers thread test") {
		bool result = databaseManager.deleteAllPeers("elas");
		REQUIRE(result == true);
		thread_call_count = 0;
		BackgroundExecutor executor((uint8_t) totalThreadCount);
		for (int i = 0; i < totalThreadCount; i++) {
			executor.execute(Runnable(std::bind(&onPutPeers, &databaseManager, i)));
		}


		while (1) {
			if (thread_call_count >= totalThreadCount) {
				break;
			}
		}

		std::vector<PeerEntity> list = databaseManager.getAllPeers("elas");
		std::sort(list.begin(), list.end(), sortPeers);
		UInt128 addr = ((UInt128) {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0xff, 0xff, 0x7f, 0x00, 0x00, 0x01});
		for (int i = 0; i < totalThreadCount * loopCount; i++) {
			for (int j = 0; j < loopCount; j++) {
				REQUIRE(list[i * loopCount + j].timeStamp == i);
				REQUIRE(list[i * loopCount + j].port == i);
				int res = UInt128Eq(&list[i * loopCount + j].address, &addr);
				REQUIRE(res == 1);
			}
		}
	}

	SECTION("DatabaseManager deletePeer thread test") {
		BackgroundExecutor executor((uint8_t) totalThreadCount);
		std::vector<PeerEntity> list = databaseManager.getAllPeers("ela");
		thread_call_count = 0;
		for (int i = 0; i < list.size(); i++) {
			executor.execute(Runnable(std::bind(&onDeletePeer, &databaseManager, list[i].id)));
		}

		while (1) {
			if (thread_call_count >= list.size()) {
				break;
			}
		}

		list = databaseManager.getAllPeers("ela");
		REQUIRE(list.size() == 0);

		list = databaseManager.getAllPeers("elas");
		BackgroundExecutor executor1(uint8_t(list.size()));
		thread_call_count = 0;
		for (int i = 0; i < list.size(); i++) {
			executor1.execute(Runnable(std::bind(&onDeletePeers, &databaseManager, list[i].id)));
		}

		while (1) {
			if (thread_call_count >= list.size()) {
				break;
			}
		}
		list = databaseManager.getAllPeers("elas");
		REQUIRE(list.size() == 0);
	}
}

void onPutMerkleBlock(void *arg, int index) {
	DatabaseManager *databaseManager = (DatabaseManager *) arg;

	MerkleBlockEntity blockEntity;
	for (int i = 0; i < loopCount; i++) {
		blockEntity.blockHeight = uint32_t(index * loopCount + i);
		blockEntity.blockBytes.length = 21;
		uint8_t s[21] = {33, 110, 179, 17, 41, 134, 242, 38, 145, 166, 17, 187, 37, 147, 24,
		                 60, 75, 8, 182, 57, 98};
		blockEntity.blockBytes.data = s;

		bool result = databaseManager->putMerkleBlock("ela", blockEntity);
		REQUIRE(result == true);
	}
	thread_call_count++;
}

void onPutMerkleBlocks(void *arg, int index) {
	DatabaseManager *databaseManager = (DatabaseManager *) arg;

	std::vector<MerkleBlockEntity> list;
	uint8_t s[21] = {33, 110, 179, 17, 41, 134, 242, 38, 145, 166, 17, 187, 37, 147, 24,
	                 60, 75, 8, 182, 57, 98};
	for (int i = 0; i < loopCount; i++) {
		MerkleBlockEntity blockEntity;
		blockEntity.blockBytes.length = 21;
		blockEntity.blockBytes.data = s;
		blockEntity.blockHeight = uint32_t(index * loopCount + i);
		list.push_back(blockEntity);
	}
	for (int i = 0; i < loopCount; i++) {
		bool result = databaseManager->putMerkleBlocks("elas", list);
		REQUIRE(result == true);
	}

	thread_call_count++;
}

void onDeleteMerkleBlocksByElas(void *arg, uint32_t blockID) {
	DatabaseManager *databaseManager = (DatabaseManager *) arg;

	MerkleBlockEntity blockEntity;
	blockEntity.id = blockID;
	bool result = databaseManager->deleteMerkleBlock("elas", blockEntity);
	REQUIRE(result == true);
	thread_call_count++;
}

void onDeleteMerkleBlocksByEla(void *arg, uint32_t blockID) {
	DatabaseManager *databaseManager = (DatabaseManager *) arg;

	MerkleBlockEntity blockEntity;
	blockEntity.id = blockID;
	bool result = databaseManager->deleteMerkleBlock("ela", blockEntity);
	REQUIRE(result == true);
	thread_call_count++;
}

void onDeleteAllMerkleBlocks(void *arg) {
	DatabaseManager *databaseManager = (DatabaseManager *) arg;
	bool result = databaseManager->deleteAllBlocks("ela");
	REQUIRE(result == true);
	result = databaseManager->deleteAllBlocks("elas");
	REQUIRE(result == true);
	result = databaseManager->deleteAllBlocks("ELA");
	REQUIRE(result == true);
	thread_call_count++;
}

bool sortMerkleBlocks(MerkleBlockEntity &a, MerkleBlockEntity &b) {
	return a.blockHeight < b.blockHeight;
}

TEST_CASE("DatabaseManager mulity thread merkleBlocks", "[DatabaseManager]") {
	DatabaseManager databaseManager;
	onDeleteAllMerkleBlocks(&databaseManager);

	SECTION("DatabaseManager deleteAllBlocks thread test") {
		BackgroundExecutor executor(totalThreadCount);
		thread_call_count = 0;
		for (int i = 0; i < totalThreadCount; ++i) {
			executor.execute(Runnable(std::bind(&onDeleteAllMerkleBlocks, &databaseManager)));
		}

		while (1) {
			if (thread_call_count >= totalThreadCount) {
				break;
			}
		}

		std::vector<MerkleBlockEntity> list = databaseManager.getAllMerkleBlocks("ela");
		REQUIRE(list.size() == 0);
		list = databaseManager.getAllMerkleBlocks("elas");
		REQUIRE(list.size() == 0);
		list = databaseManager.getAllMerkleBlocks("ELA");
		REQUIRE(list.size() == 0);
	}

	SECTION("DatabaseManager putMerkleBlock thread test") {
		bool result = databaseManager.deleteAllBlocks("ela");
		REQUIRE(result == true);
		thread_call_count = 0;
		BackgroundExecutor executor((uint8_t) totalThreadCount);
		for (int i = 0; i < totalThreadCount; i++) {
			executor.execute(Runnable(std::bind(&onPutMerkleBlock, &databaseManager, i)));
		}

		while (1) {
			if (thread_call_count >= totalThreadCount) {
				break;
			}
		}

		uint8_t s[21] = {33, 110, 179, 17, 41, 134, 242, 38, 145, 166, 17, 187, 37, 147, 24,
		                 60, 75, 8, 182, 57, 98};
		std::vector<MerkleBlockEntity> list = databaseManager.getAllMerkleBlocks("ela");
		sort(list.begin(), list.end(), sortMerkleBlocks);
		for (int i = 0; i < totalThreadCount * loopCount; i++) {
			REQUIRE(list[i].blockHeight == i);
			REQUIRE(list[i].blockBytes.length == 21);
			for (int j = 0; j < 21; j++) {
				REQUIRE(list[i].blockBytes.data[j] == s[j]);
			}
		}
	}

	SECTION("DatabaseManager putMerkleBlocks thread test") {
		bool result = databaseManager.deleteAllBlocks("elas");
		REQUIRE(result == true);
		thread_call_count = 0;
		BackgroundExecutor executor((uint8_t) totalThreadCount);
		for (int i = 0; i < totalThreadCount; i++) {
			executor.execute(Runnable(std::bind(&onPutMerkleBlocks, &databaseManager, i)));
		}

		while (1) {
			if (thread_call_count >= totalThreadCount) {
				break;
			}
		}

		std::vector<MerkleBlockEntity> list = databaseManager.getAllMerkleBlocks("elas");
		std::sort(list.begin(), list.end(), sortMerkleBlocks);
		uint8_t s[21] = {33, 110, 179, 17, 41, 134, 242, 38, 145, 166, 17, 187, 37, 147, 24,
		                 60, 75, 8, 182, 57, 98};
		for (int i = 0; i < totalThreadCount * loopCount; i++) {
			for (int j = 0; j < loopCount; j++) {
				REQUIRE(list[i * loopCount + j].blockHeight == i);
				REQUIRE(list[i * loopCount + j].blockBytes.length == 21);
				for (int k = 0; k < 21; k++) {
					REQUIRE(list[i * loopCount + j].blockBytes.data[k] == s[k]);
				}
			}
		}
	}

	SECTION("DatabaseManager deleteMerkleBlock thread test") {

		std::vector<MerkleBlockEntity> list = databaseManager.getAllMerkleBlocks("ela");
		BackgroundExecutor executor((uint8_t) list.size());
		thread_call_count = 0;
		for (int i = 0; i < list.size(); i++) {
			executor.execute(Runnable(std::bind(&onDeleteMerkleBlocksByEla, &databaseManager, list[i].id)));
		}

		while (1) {
			if (thread_call_count >= list.size()) {
				break;
			}
		}

		list = databaseManager.getAllMerkleBlocks("ela");
		REQUIRE(list.size() == 0);

		list = databaseManager.getAllMerkleBlocks("elas");
		BackgroundExecutor executor1((uint8_t) list.size());
		thread_call_count = 0;
		for (int i = 0; i < list.size(); i++) {
			executor.execute(Runnable(std::bind(&onDeleteMerkleBlocksByElas, &databaseManager, list[i].id)));
		}

		while (1) {
			if (thread_call_count >= list.size()) {
				break;
			}
		}

		list = databaseManager.getAllMerkleBlocks("elas");
		REQUIRE(list.size() == 0);
	}
}