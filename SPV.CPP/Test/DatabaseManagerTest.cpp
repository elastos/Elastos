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
#define TEST_MERKLEBLOCK_DATALEN uint64_t(75)

		static std::vector<MerkleBlockEntity> blocksToSave;

		SECTION("Merkle Block prepare for testing") {
			for (uint64_t i = 0; i < TEST_MERKLEBLOCK_RECORD_CNT; ++i) {
				MerkleBlockEntity block;

				CMBlock buff;
				buff.Resize(TEST_MERKLEBLOCK_DATALEN);
				for (uint64_t j = 0; j < TEST_MERKLEBLOCK_DATALEN; ++j) {
					buff[j] = static_cast<uint8_t>(TEST_ASCII_BEGIN + rand() % TEST_MERKLEBLOCK_DATALEN);
				}
				buff[TEST_MERKLEBLOCK_DATALEN - 1] = 0;

				block.blockBytes = buff;
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
				REQUIRE(TEST_MERKLEBLOCK_DATALEN == blocksRead[i].blockBytes.GetSize());
				REQUIRE(blocksRead[i].blockHeight == blocksToSave[i].blockHeight);
				REQUIRE(0 == memcmp(blocksRead[i].blockBytes, blocksToSave[i].blockBytes, TEST_MERKLEBLOCK_DATALEN));
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
				REQUIRE(TEST_MERKLEBLOCK_DATALEN == blocksRead[i].blockBytes.GetSize());
				REQUIRE(0 == memcmp(blocksRead[i].blockBytes, blocksToSave[i].blockBytes, TEST_MERKLEBLOCK_DATALEN));
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
				peer.address = ((UInt128) {'1', '2', '7', '.', '0', '.', '0', '.', (uint8_t)('0' + i), ':', '8', '0', '8', '0', '.', 0});
				peer.port = static_cast<uint16_t>(8000 + i);
				peer.timeStamp = static_cast<uint64_t>(1000 + i);
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
			REQUIRE(peers.size() == TEST_PEER_RECORD_CNT);
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
			REQUIRE(peers.size() == TEST_PEER_RECORD_CNT);
			for (int i = 0; i < peers.size(); i++) {
				REQUIRE(UInt128Eq(&peers[i].address, &peerToSave[i].address));
				REQUIRE(peers[i].port == peerToSave[i].port);
				REQUIRE(peers[i].timeStamp == peerToSave[i].timeStamp);
			}
		}

		SECTION("Peer delete one by one test") {
			DatabaseManager dbm(DBFILE);

			std::vector<PeerEntity> PeersBeforeDelete = dbm.getAllPeers(ISO);
			REQUIRE(PeersBeforeDelete.size() == TEST_PEER_RECORD_CNT);

			for (int i = 0; i < PeersBeforeDelete.size(); ++i) {
				REQUIRE(dbm.deletePeer(ISO, PeersBeforeDelete[i]));
			}

			std::vector<PeerEntity> PeersAfterDelete = dbm.getAllPeers(ISO);
			REQUIRE(0 == PeersAfterDelete.size());
		}

	}

#define TEST_TX_RECORD_CNT 20
#define TEST_TX_DATALEN    uint64_t(75)
#define TEST_TX_HASH_LEN   256
	SECTION("Transaction test") {
		static std::vector<TransactionEntity> txToSave;
		static std::vector<TransactionEntity> txToUpdate;

		SECTION("Transaction prepare for testing") {
			char buf[TEST_TX_HASH_LEN + 1];

			for (uint64_t i = 0; i < TEST_TX_RECORD_CNT; ++i) {
				TransactionEntity tx;

				CMBlock buff;
				buff.Resize(TEST_TX_DATALEN);
				for (uint64_t j = 0; j < TEST_TX_DATALEN; ++j) {
					buff[j] = static_cast<uint8_t>(TEST_ASCII_BEGIN + rand() % TEST_TX_DATALEN);
				}
				buff[TEST_TX_DATALEN - 1] = 0;

				tx.buff = buff;
				tx.blockHeight = static_cast<uint32_t>(i + 100);
				tx.timeStamp = static_cast<uint32_t>(i + 200);
				for (int j = 0; j < TEST_TX_HASH_LEN; j++) {
					buf[j] = static_cast<char>(rand() % TEST_TX_DATALEN + TEST_ASCII_BEGIN);
				}
				tx.txHash = buf;
				txToSave.push_back(tx);
			}

			for (uint64_t i = 0; i < TEST_TX_RECORD_CNT; ++i) {
				TransactionEntity tx;

				CMBlock buff;
				buff.Resize(TEST_TX_DATALEN);
				for (uint64_t j = 0; j < TEST_TX_DATALEN; ++j) {
					buff[j] = static_cast<uint8_t>(TEST_ASCII_BEGIN + rand() % TEST_TX_DATALEN);
				}
				buff[TEST_TX_DATALEN - 1] = 0;

				tx.buff = buff;
				tx.blockHeight = static_cast<uint32_t>(i + 300);
				tx.timeStamp = static_cast<uint32_t>(i + 400);
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
			REQUIRE(TEST_TX_RECORD_CNT == readTx.size());

			for (int i = 0; i < readTx.size(); ++i) {
				REQUIRE(TEST_TX_DATALEN == readTx[i].buff.GetSize());
				REQUIRE(0 == memcmp(readTx[i].buff, txToSave[i].buff, TEST_TX_DATALEN));
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
				REQUIRE(TEST_TX_DATALEN == readTx[i].buff.GetSize());
				//do not need to compare buff, because updateTransaction() do not update this member.
				//REQUIRE(0 == memcmp(readTx[i].buff, txToUpdate[i].buff, TEST_TX_DATALEN));
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

}
