// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#define CATCH_CONFIG_MAIN

#include <fstream>

#include "TransactionDataStore.h"
#include "DatabaseManager.h"
#include "catch.hpp"
#include "SpvService/BackgroundExecutor.h"
#include "Utils.h"
#include "Log.h"

using namespace Elastos::ElaWallet;
#if 0
const int totalThreadCount = 200;
const int loopCount = 5;
int thread_call_count = 0;
#endif

#define ISO "els"
#define DBFILE "wallet.db"
#define TEST_ASCII_BEGIN 48

std::string getRandString(size_t length) {
	char buf[length];
	for (size_t i = 0; i < length; ++i) {
		buf[i] = static_cast<uint8_t>(TEST_ASCII_BEGIN + rand() % 75);
	}

	return std::string(buf);
}

CMBlock getRandCMBlock(size_t size) {
	CMBlock block(size);

	for (size_t i = 0; i < size; ++i) {
		block[i] = (uint8_t)rand();
	}

	return block;
}

TEST_CASE("DatabaseManager test", "[DatabaseManager]") {

	SECTION("Prepare to test") {
		srand(time(nullptr));

		if (boost::filesystem::exists(DBFILE) && boost::filesystem::is_regular_file(DBFILE)) {
			boost::filesystem::remove(DBFILE);
		}

		REQUIRE(!boost::filesystem::exists(DBFILE));
	}

	SECTION("Merkle Block test ") {
#define TEST_MERKLEBLOCK_RECORD_CNT uint64_t(20)

		static std::vector<MerkleBlockEntity> blocksToSave;

		SECTION("Merkle Block prepare for testing") {
			for (uint64_t i = 0; i < TEST_MERKLEBLOCK_RECORD_CNT; ++i) {
				MerkleBlockEntity block;

				block.blockBytes = getRandCMBlock(40);
				block.blockHeight = static_cast<uint32_t>(i);

				blocksToSave.push_back(block);
			}
		}

		SECTION("Merkle Block save test") {
			DatabaseManager *dbm = new DatabaseManager(DBFILE);
			REQUIRE(dbm->putMerkleBlocks(ISO, blocksToSave));
			delete dbm;
		}

		SECTION("Merkle Block read test") {
			DatabaseManager dbm(DBFILE);
			std::vector<MerkleBlockEntity> blocksRead = dbm.getAllMerkleBlocks(ISO);
			REQUIRE(blocksRead.size() == blocksToSave.size());
			for (int i = 0; i < blocksRead.size(); ++i) {
				REQUIRE(blocksToSave[i].blockBytes.GetSize() == blocksRead[i].blockBytes.GetSize());
				REQUIRE(blocksRead[i].blockHeight == blocksToSave[i].blockHeight);
				REQUIRE(0 == memcmp(blocksRead[i].blockBytes, blocksToSave[i].blockBytes, blocksRead[i].blockBytes.GetSize()));
			}
		}

		SECTION("Merkle Block delete test") {
			DatabaseManager dbm(DBFILE);

			REQUIRE(dbm.deleteAllBlocks(ISO));

			std::vector<MerkleBlockEntity> blocksAfterDelete = dbm.getAllMerkleBlocks(ISO);
			REQUIRE(0 == blocksAfterDelete.size());
		}

		SECTION("Merkle Block save one by one test") {
			DatabaseManager dbm(DBFILE);
			for (int i = 0; i < blocksToSave.size(); ++i) {
				REQUIRE(dbm.putMerkleBlock(ISO, blocksToSave[i]));
			}
		}

		SECTION("Merkle Block read test") {
			DatabaseManager dbm(DBFILE);
			std::vector<MerkleBlockEntity> blocksRead = dbm.getAllMerkleBlocks(ISO);
			REQUIRE(blocksRead.size() == blocksToSave.size());
			for (int i = 0; i < blocksRead.size(); ++i) {
				REQUIRE(blocksRead[i].blockHeight == blocksToSave[i].blockHeight);
				REQUIRE(blocksToSave[i].blockBytes.GetSize() == blocksRead[i].blockBytes.GetSize());
				REQUIRE(0 == memcmp(blocksRead[i].blockBytes, blocksToSave[i].blockBytes, blocksRead[i].blockBytes.GetSize()));
			}
		}

		SECTION("Merkle Block delete one by one test") {
			DatabaseManager dbm(DBFILE);

			std::vector<MerkleBlockEntity> blocksBeforeDelete = dbm.getAllMerkleBlocks(ISO);

			for (int i = 0; i < blocksBeforeDelete.size(); ++i) {
				REQUIRE(dbm.deleteMerkleBlock(ISO, blocksBeforeDelete[i]));
			}

			std::vector<MerkleBlockEntity> blocksAfterDelete = dbm.getAllMerkleBlocks(ISO);
			REQUIRE(0 == blocksAfterDelete.size());
		}
	}

#define TEST_PEER_RECORD_CNT 20
	SECTION("Peer test") {

		static std::vector<PeerEntity> peerToSave;

		SECTION("Peer Prepare for test") {
			for (int i = 0; i < TEST_PEER_RECORD_CNT; i++) {
				PeerEntity peer;
				CMBlock addr = getRandCMBlock(sizeof(UInt128));
				memcpy(peer.address.u8, addr, addr.GetSize());
				peer.port = (uint16_t)rand();
				peer.timeStamp = (uint64_t)rand();
				peerToSave.push_back(peer);
			}

			REQUIRE(TEST_PEER_RECORD_CNT == peerToSave.size());
		}

		SECTION("Peer save test") {
			DatabaseManager dbm(DBFILE);
			REQUIRE(dbm.putPeers(ISO, peerToSave));
		}

		SECTION("Peer read test") {
			DatabaseManager dbm(DBFILE);
			std::vector<PeerEntity> peers = dbm.getAllPeers(ISO);
			REQUIRE(peers.size() == peerToSave.size());
			for (int i = 0; i < peers.size(); i++) {
				REQUIRE(UInt128Eq(&peers[i].address, &peerToSave[i].address));
				REQUIRE(peers[i].port == peerToSave[i].port);
				REQUIRE(peers[i].timeStamp == peerToSave[i].timeStamp);
			}
		}

		SECTION("Peer delete test") {
			DatabaseManager *dbm = new DatabaseManager(DBFILE);
			REQUIRE(dbm->deleteAllPeers(ISO));
			std::vector<PeerEntity> peers = dbm->getAllPeers(ISO);
			REQUIRE(peers.size() == 0);
			delete dbm;
		}

		SECTION("Peer save one by one test") {
			DatabaseManager dbm(DBFILE);
			for (int i = 0; i < peerToSave.size(); ++i) {
				REQUIRE(dbm.putPeer(ISO, peerToSave[i]));
			}
		}

		SECTION("Peer read test") {
			DatabaseManager dbm(DBFILE);
			std::vector<PeerEntity> peers = dbm.getAllPeers(ISO);
			REQUIRE(peers.size() == peerToSave.size());
			for (int i = 0; i < peers.size(); i++) {
				REQUIRE(UInt128Eq(&peers[i].address, &peerToSave[i].address));
				REQUIRE(peers[i].port == peerToSave[i].port);
				REQUIRE(peers[i].timeStamp == peerToSave[i].timeStamp);
			}
		}

		SECTION("Peer delete one by one test") {
			DatabaseManager dbm(DBFILE);

			std::vector<PeerEntity> PeersBeforeDelete = dbm.getAllPeers(ISO);
			REQUIRE(PeersBeforeDelete.size() == peerToSave.size());

			for (int i = 0; i < PeersBeforeDelete.size(); ++i) {
				REQUIRE(dbm.deletePeer(ISO, PeersBeforeDelete[i]));
			}

			std::vector<PeerEntity> PeersAfterDelete = dbm.getAllPeers(ISO);
			REQUIRE(0 == PeersAfterDelete.size());
		}

	}

#define TEST_TX_RECORD_CNT 20
	SECTION("Transaction test") {
		static std::vector<TransactionEntity> txToSave;
		static std::vector<TransactionEntity> txToUpdate;

		SECTION("Transaction prepare for testing") {
			for (uint64_t i = 0; i < TEST_TX_RECORD_CNT; ++i) {
				TransactionEntity tx;

				tx.buff = getRandCMBlock(35);
				tx.blockHeight = (uint32_t)rand();
				tx.timeStamp = (uint32_t)rand();
				tx.txHash = getRandString(25);
				txToSave.push_back(tx);
			}

			for (uint64_t i = 0; i < TEST_TX_RECORD_CNT; ++i) {
				TransactionEntity tx;

				tx.buff = getRandCMBlock(49);
				tx.blockHeight = (uint32_t)rand();
				tx.timeStamp = (uint32_t)rand();
				tx.txHash = txToSave[i].txHash;
				txToUpdate.push_back(tx);
			}
		}

		SECTION("Transaction save test") {
			DatabaseManager dbm(DBFILE);
			for (int i = 0; i < txToSave.size(); ++i) {
				REQUIRE(dbm.putTransaction(ISO, txToSave[i]));
			}
		}

		SECTION("Transaction read test") {
			DatabaseManager dbm(DBFILE);
			std::vector<TransactionEntity> readTx = dbm.getAllTransactions(ISO);
			REQUIRE(txToSave.size() == readTx.size());

			for (int i = 0; i < readTx.size(); ++i) {
				REQUIRE(txToSave[i].buff.GetSize() == readTx[i].buff.GetSize());
				REQUIRE(0 == memcmp(readTx[i].buff, txToSave[i].buff, txToSave[i].buff.GetSize()));
				REQUIRE(readTx[i].txHash == txToSave[i].txHash);
				REQUIRE(readTx[i].timeStamp == txToSave[i].timeStamp);
				REQUIRE(readTx[i].blockHeight == txToSave[i].blockHeight);
			}
		}

		SECTION("Transaction udpate test") {
			DatabaseManager dbm(DBFILE);

			for (int i = 0; i < txToUpdate.size(); ++i) {
				REQUIRE(dbm.updateTransaction(ISO, txToUpdate[i]));
			}
		}

		SECTION("Transaction read after update test") {
			DatabaseManager dbm(DBFILE);
			std::vector<TransactionEntity> readTx = dbm.getAllTransactions(ISO);
			REQUIRE(TEST_TX_RECORD_CNT == readTx.size());

			for (int i = 0; i < readTx.size(); ++i) {
				REQUIRE(txToSave[i].buff.GetSize() == readTx[i].buff.GetSize());
				REQUIRE(0 == memcmp(readTx[i].buff, txToSave[i].buff, txToSave[i].buff.GetSize()));
				REQUIRE(readTx[i].txHash == txToUpdate[i].txHash);
				REQUIRE(readTx[i].timeStamp == txToUpdate[i].timeStamp);
				REQUIRE(readTx[i].blockHeight == txToUpdate[i].blockHeight);
			}
		}

		SECTION("Transaction delete by txHash test") {
			DatabaseManager dbm(DBFILE);

			for (int i = 0; i < txToUpdate.size(); ++i) {
				REQUIRE(dbm.deleteTxByHash(ISO, txToUpdate[i].txHash));
			}

			std::vector<TransactionEntity> readTx = dbm.getAllTransactions(ISO);
			REQUIRE(0 == readTx.size());
		}

	}

	SECTION("InternalAddresses test") {
		DatabaseManager dbm(DBFILE);

		size_t count = 111111;
		REQUIRE(dbm.clearInternalAddresses());
		count = dbm.getInternalAvailableAddresses(0);
		REQUIRE(0 == count);

		std::string addr0 = getRandString(40);
		REQUIRE(dbm.putInternalAddress(0, addr0));
		std::string addr1 = getRandString(40);
		REQUIRE(dbm.putInternalAddress(1, addr1));
		std::string addr2 = getRandString(40);
		REQUIRE(dbm.putInternalAddress(2, addr2));
		std::string addr3 = getRandString(40);
		REQUIRE(dbm.putInternalAddress(3, addr3));

		std::vector<std::string> gotAddresses = dbm.getInternalAddresses(0, 2);
		REQUIRE(gotAddresses.size() == 2);
		REQUIRE(gotAddresses[0] == addr0);
		REQUIRE(gotAddresses[1] == addr1);

		gotAddresses = dbm.getInternalAddresses(0, 4);
		REQUIRE(gotAddresses.size() == 4);
		REQUIRE(gotAddresses[0] == addr0);
		REQUIRE(gotAddresses[1] == addr1);
		REQUIRE(gotAddresses[2] == addr2);
		REQUIRE(gotAddresses[3] == addr3);

		gotAddresses = dbm.getInternalAddresses(2, 1000);
		REQUIRE(gotAddresses.size() == 2);
		REQUIRE(gotAddresses[0] == addr2);
		REQUIRE(gotAddresses[1] == addr3);

		gotAddresses = dbm.getInternalAddresses(1, 1);
		REQUIRE(gotAddresses.size() == 1);
		REQUIRE(gotAddresses[0] == addr1);

		gotAddresses = dbm.getInternalAddresses(0, 0);
		REQUIRE(gotAddresses.size() == 0);

		gotAddresses = dbm.getInternalAddresses(2, 0);
		REQUIRE(gotAddresses.size() == 0);

		count = dbm.getInternalAvailableAddresses(0);
		REQUIRE(count == 4);
		count = dbm.getInternalAvailableAddresses(2);
		REQUIRE(count == 2);
		count = dbm.getInternalAvailableAddresses(4);
		REQUIRE(count == 0);

		std::vector<std::string> addresses(10);
		for (size_t i = 0; i < addresses.size(); ++i) {
			addresses[i] = getRandString(40);
		}
		REQUIRE(dbm.putInternalAddresses(4, addresses));

		count = dbm.getInternalAvailableAddresses(20);
		REQUIRE(count == 0);

		count = dbm.getInternalAvailableAddresses(0);
		REQUIRE(count == 14);

		gotAddresses = dbm.getInternalAddresses(0, count);
		REQUIRE(gotAddresses.size() == count);
		REQUIRE(gotAddresses[4] == addresses[0]);
		REQUIRE(gotAddresses[6] == addresses[2]);
		REQUIRE(gotAddresses[13] == addresses[9]);

		REQUIRE(dbm.clearInternalAddresses());
		count = dbm.getInternalAvailableAddresses(0);
		REQUIRE(0 == count);
	}

	SECTION("ExternalAddresses test") {
		DatabaseManager dbm(DBFILE);

		size_t count = 111111;
		REQUIRE(dbm.clearExternalAddresses());
		count = dbm.getExternalAvailableAddresses(0);
		REQUIRE(0 == count);

		std::string addr0 = getRandString(40);
		REQUIRE(dbm.putExternalAddress(0, addr0));
		std::string addr1 = getRandString(40);
		REQUIRE(dbm.putExternalAddress(1, addr1));
		std::string addr2 = getRandString(40);
		REQUIRE(dbm.putExternalAddress(2, addr2));
		std::string addr3 = getRandString(40);
		REQUIRE(dbm.putExternalAddress(3, addr3));

		std::vector<std::string> gotAddresses = dbm.getExternalAddresses(0, 2);
		REQUIRE(gotAddresses.size() == 2);
		REQUIRE(gotAddresses[0] == addr0);
		REQUIRE(gotAddresses[1] == addr1);

		gotAddresses = dbm.getExternalAddresses(0, 4);
		REQUIRE(gotAddresses.size() == 4);
		REQUIRE(gotAddresses[0] == addr0);
		REQUIRE(gotAddresses[1] == addr1);
		REQUIRE(gotAddresses[2] == addr2);
		REQUIRE(gotAddresses[3] == addr3);

		gotAddresses = dbm.getExternalAddresses(2, 1000);
		REQUIRE(gotAddresses.size() == 2);
		REQUIRE(gotAddresses[0] == addr2);
		REQUIRE(gotAddresses[1] == addr3);

		gotAddresses = dbm.getExternalAddresses(1, 1);
		REQUIRE(gotAddresses.size() == 1);
		REQUIRE(gotAddresses[0] == addr1);

		gotAddresses = dbm.getExternalAddresses(0, 0);
		REQUIRE(gotAddresses.size() == 0);

		gotAddresses = dbm.getExternalAddresses(2, 0);
		REQUIRE(gotAddresses.size() == 0);

		count = dbm.getExternalAvailableAddresses(0);
		REQUIRE(count == 4);
		count = dbm.getExternalAvailableAddresses(2);
		REQUIRE(count == 2);
		count = dbm.getExternalAvailableAddresses(4);
		REQUIRE(count == 0);

		std::vector<std::string> addresses(10);
		for (size_t i = 0; i < addresses.size(); ++i) {
			addresses[i] = getRandString(40);
		}
		REQUIRE(dbm.putExternalAddresses(4, addresses));

		count = dbm.getExternalAvailableAddresses(20);
		REQUIRE(count == 0);

		count = dbm.getExternalAvailableAddresses(0);
		REQUIRE(count == 14);

		gotAddresses = dbm.getExternalAddresses(0, count);
		REQUIRE(gotAddresses.size() == count);
		REQUIRE(gotAddresses[4] == addresses[0]);
		REQUIRE(gotAddresses[6] == addresses[2]);
		REQUIRE(gotAddresses[13] == addresses[9]);

		REQUIRE(dbm.clearExternalAddresses());
		count = dbm.getExternalAvailableAddresses(0);
		REQUIRE(0 == count);
	}

}
