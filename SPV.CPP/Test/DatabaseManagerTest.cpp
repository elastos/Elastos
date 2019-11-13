// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#define CATCH_CONFIG_MAIN

#include <catch.hpp>
#include "TestHelper.h"

#include <Database/TransactionDataStore.h>
#include <Database/DatabaseManager.h>
#include <SpvService/BackgroundExecutor.h>
#include <Common/Utils.h>
#include <Common/Log.h>
#include <Wallet/UTXO.h>
#include <Plugin/Registry.h>
#include <Plugin/Block/MerkleBlock.h>
#include <Plugin/ELAPlugin.h>

#include <fstream>

using namespace Elastos::ElaWallet;

#define ISO "ela1"
#define DBFILE "wallet.db"

TEST_CASE("DatabaseManager test", "[DatabaseManager]") {
	Log::registerMultiLogger();
	std::string pluginType = "ELA";
#define DEFAULT_RECORD_CNT 20

	SECTION("Prepare to test") {
		srand(time(nullptr));

		if (boost::filesystem::exists(DBFILE) && boost::filesystem::is_regular_file(DBFILE)) {
			boost::filesystem::remove(DBFILE);
		}

		REQUIRE(!boost::filesystem::exists(DBFILE));
	}

	SECTION("Asset test") {
#define TEST_ASSET_RECORD_CNT DEFAULT_RECORD_CNT
		DatabaseManager dm(DBFILE);

		// save
		std::vector<AssetEntity> assets;
		for (int i = 0; i < TEST_ASSET_RECORD_CNT; ++i) {
			AssetEntity asset;
			asset.Asset = getRandBytes(100);
			asset.AssetID = getRandString(64);
			asset.Amount = rand();
			assets.push_back(asset);
			REQUIRE(dm.PutAsset(ISO, asset));
		}

		// verify save
		std::vector<AssetEntity> assetsVerify = dm.GetAllAssets();
		REQUIRE(assetsVerify.size() == TEST_ASSET_RECORD_CNT);
		REQUIRE(assetsVerify.size() == assets.size());
		for (size_t i = 0; i < assets.size(); ++i) {
			REQUIRE(assets[i].Asset == assetsVerify[i].Asset);
			REQUIRE(assets[i].AssetID == assetsVerify[i].AssetID);
			REQUIRE(assets[i].Amount == assetsVerify[i].Amount);
		}

		// delete random one
		int idx = rand() % assetsVerify.size();
		REQUIRE(dm.DeleteAsset(assets[idx].AssetID));

		// verify deleted
		AssetEntity assetGot;
		REQUIRE(!dm.GetAssetDetails(assets[idx].AssetID, assetGot));

		// verify count after delete
		assetsVerify = dm.GetAllAssets();
		REQUIRE(assetsVerify.size() == assets.size() - 1);

		// update already exist assetID
		idx = rand() % assetsVerify.size();
		AssetEntity assetsUpdate;
		assetsVerify[idx].Amount = rand();
		assetsVerify[idx].Asset = getRandBytes(200);
		assetsUpdate = assetsVerify[idx];
		REQUIRE(dm.PutAsset("Test", assetsUpdate));

		REQUIRE(dm.GetAssetDetails(assetsVerify[idx].AssetID, assetGot));
		REQUIRE(assetsVerify[idx].Amount == assetGot.Amount);
		REQUIRE(assetsVerify[idx].Asset == assetGot.Asset);

		// delete all
		REQUIRE(dm.DeleteAllAssets());
		assetsVerify = dm.GetAllAssets();
		REQUIRE(assetsVerify.size() == 0);
	}

	SECTION("Merkle Block test ") {
#define TEST_MERKLEBLOCK_RECORD_CNT DEFAULT_RECORD_CNT
#ifdef SPV_ENABLE_STATIC
		REGISTER_MERKLEBLOCKPLUGIN(ELA, getELAPluginComponent);
#endif
		static std::vector<MerkleBlockPtr> blocksToSave;

		SECTION("Merkle Block prepare for testing") {
			for (uint64_t i = 0; i < TEST_MERKLEBLOCK_RECORD_CNT; ++i) {
				MerkleBlockPtr merkleBlock(Registry::Instance()->CreateMerkleBlock(pluginType));

				merkleBlock->SetHeight(static_cast<uint32_t>(i + 1));
				merkleBlock->SetTimestamp(getRandUInt32());
				merkleBlock->SetPrevBlockHash(uint256(getRandBytes(32)));
				merkleBlock->SetTarget(getRandUInt32());
				merkleBlock->SetNonce(getRandUInt32());

				blocksToSave.push_back(merkleBlock);
			}
		}

		SECTION("Merkle Block save test") {
			DatabaseManager *dbm = new DatabaseManager(DBFILE);
			REQUIRE(dbm->PutMerkleBlocks(ISO, blocksToSave));
			delete dbm;
		}

		SECTION("Merkle Block read test") {
			DatabaseManager dbm(DBFILE);
			std::vector<MerkleBlockPtr> blocksRead = dbm.GetAllMerkleBlocks(ISO, pluginType);
			REQUIRE(blocksRead.size() == blocksToSave.size());
			for (int i = 0; i < blocksRead.size(); ++i) {
				REQUIRE(blocksToSave[i]->GetHeight() == blocksRead[i]->GetHeight());
				REQUIRE(blocksToSave[i]->GetTimestamp() == blocksRead[i]->GetTimestamp());
				REQUIRE(blocksToSave[i]->GetPrevBlockHash() == blocksRead[i]->GetPrevBlockHash());
				REQUIRE(blocksToSave[i]->GetHash() == blocksRead[i]->GetHash());
				REQUIRE(blocksToSave[i]->GetTarget() == blocksRead[i]->GetTarget());
				REQUIRE(blocksToSave[i]->GetNonce() == blocksRead[i]->GetNonce());
			}
		}

		SECTION("Merkle Block delete test") {
			DatabaseManager dbm(DBFILE);

			REQUIRE(dbm.DeleteAllBlocks(ISO));

			std::vector<MerkleBlockPtr> blocksAfterDelete = dbm.GetAllMerkleBlocks(ISO, pluginType);
			REQUIRE(0 == blocksAfterDelete.size());
		}

		SECTION("Merkle Block save one by one test") {
			DatabaseManager dbm(DBFILE);
			for (int i = 0; i < blocksToSave.size(); ++i) {
				REQUIRE(dbm.PutMerkleBlock(ISO, blocksToSave[i]));
			}
		}

		SECTION("Merkle Block read test") {
			DatabaseManager dbm(DBFILE);
			std::vector<MerkleBlockPtr> blocksRead = dbm.GetAllMerkleBlocks(ISO, pluginType);
			REQUIRE(blocksRead.size() == blocksToSave.size());
			for (int i = 0; i < blocksRead.size(); ++i) {
				REQUIRE(blocksToSave[i]->GetHeight() == blocksRead[i]->GetHeight());
				REQUIRE(blocksToSave[i]->GetTimestamp() == blocksRead[i]->GetTimestamp());
				REQUIRE(blocksToSave[i]->GetPrevBlockHash() == blocksRead[i]->GetPrevBlockHash());
				REQUIRE(blocksToSave[i]->GetHash() == blocksRead[i]->GetHash());
				REQUIRE(blocksToSave[i]->GetTarget() == blocksRead[i]->GetTarget());
				REQUIRE(blocksToSave[i]->GetNonce() == blocksRead[i]->GetNonce());
			}
		}

		SECTION("Merkle Block delete one by one test") {
			DatabaseManager dbm(DBFILE);

			std::vector<MerkleBlockPtr> blocksBeforeDelete = dbm.GetAllMerkleBlocks(ISO, pluginType);

			for (int i = 0; i < blocksBeforeDelete.size(); ++i) {
				REQUIRE(dbm.DeleteMerkleBlock(ISO, blocksBeforeDelete.size() + i + 1));
			}

			std::vector<MerkleBlockPtr> blocksAfterDelete = dbm.GetAllMerkleBlocks(ISO, pluginType);
			REQUIRE(0 == blocksAfterDelete.size());
		}
	}

	SECTION("Peer test") {
#define TEST_PEER_RECORD_CNT DEFAULT_RECORD_CNT

		static std::vector<PeerEntity> peerToSave;

		SECTION("Peer Prepare for test") {
			for (int i = 0; i < TEST_PEER_RECORD_CNT; i++) {
				PeerEntity peer;
				peer.address = getRandUInt128();
				peer.port = (uint16_t) rand();
				peer.timeStamp = (uint64_t) rand();
				peerToSave.push_back(peer);
			}

			REQUIRE(TEST_PEER_RECORD_CNT == peerToSave.size());
		}

		SECTION("Peer save test") {
			DatabaseManager dbm(DBFILE);
			REQUIRE(dbm.PutPeers(ISO, peerToSave));
		}

		SECTION("Peer read test") {
			DatabaseManager dbm(DBFILE);
			std::vector<PeerEntity> peers = dbm.GetAllPeers(ISO);
			REQUIRE(peers.size() == peerToSave.size());
			for (int i = 0; i < peers.size(); i++) {
				REQUIRE(peers[i].address == peerToSave[i].address);
				REQUIRE(peers[i].port == peerToSave[i].port);
				REQUIRE(peers[i].timeStamp == peerToSave[i].timeStamp);
			}
		}

		SECTION("Peer delete test") {
			DatabaseManager *dbm = new DatabaseManager(DBFILE);
			REQUIRE(dbm->DeleteAllPeers());
			std::vector<PeerEntity> peers = dbm->GetAllPeers(ISO);
			REQUIRE(peers.size() == 0);
			delete dbm;
		}

		SECTION("Peer save one by one test") {
			DatabaseManager dbm(DBFILE);
			for (int i = 0; i < peerToSave.size(); ++i) {
				REQUIRE(dbm.PutPeer(ISO, peerToSave[i]));
			}
		}

		SECTION("Peer read test") {
			DatabaseManager dbm(DBFILE);
			std::vector<PeerEntity> peers = dbm.GetAllPeers(ISO);
			REQUIRE(peers.size() == peerToSave.size());
			for (int i = 0; i < peers.size(); i++) {
				REQUIRE(peers[i].address == peerToSave[i].address);
				REQUIRE(peers[i].port == peerToSave[i].port);
				REQUIRE(peers[i].timeStamp == peerToSave[i].timeStamp);
			}
		}

		SECTION("Peer delete one by one test") {
			DatabaseManager dbm(DBFILE);

			std::vector<PeerEntity> PeersBeforeDelete = dbm.GetAllPeers(ISO);
			REQUIRE(PeersBeforeDelete.size() == peerToSave.size());

			for (int i = 0; i < PeersBeforeDelete.size(); ++i) {
				REQUIRE(dbm.DeletePeer(ISO, PeersBeforeDelete[i]));
			}

			std::vector<PeerEntity> PeersAfterDelete = dbm.GetAllPeers(ISO);
			REQUIRE(0 == PeersAfterDelete.size());
		}

	}

	SECTION("CoinBase UTXO test") {
#define TEST_TX_RECORD_CNT DEFAULT_RECORD_CNT
		static std::vector<UTXOPtr> txToSave;
		static std::vector<UTXOPtr> txToUpdate;

		SECTION("prepare for testing") {
			for (uint64_t i = 0; i < TEST_TX_RECORD_CNT; ++i) {
				OutputPtr o(new TransactionOutput(getRandBigInt(), Address(getRandUInt168()), getRanduint256()));
				o->SetOutputLock(getRandUInt32());
				UTXOPtr entity(new UTXO(getRanduint256(), getRandUInt16(), getRandUInt32(), getRandUInt32(), o));

				txToSave.push_back(entity);
			}

			for (uint64_t i = 0; i < TEST_TX_RECORD_CNT; ++i) {
				UTXOPtr entity(new UTXO());

				entity->SetTimestamp(12345);
				entity->SetBlockHeight(12345678);
				entity->SetHash(txToSave[i]->Hash());
				txToUpdate.push_back(entity);
			}
		}

		SECTION("save test") {
			DatabaseManager dbm(DBFILE);
			for (int i = 0; i < txToSave.size(); ++i)
				REQUIRE(dbm.PutCoinBase(txToSave[i]));
		}

		SECTION("read test") {
			DatabaseManager dbm(DBFILE);
			std::vector<UTXOPtr> readTx = dbm.GetAllCoinBase();
			REQUIRE(txToSave.size() == readTx.size());

			for (int i = 0; i < readTx.size(); ++i) {
				REQUIRE(readTx[i]->Spent() == txToSave[i]->Spent());
				REQUIRE(readTx[i]->Index() == txToSave[i]->Index());
				REQUIRE(readTx[i]->Output()->ProgramHash() == txToSave[i]->Output()->ProgramHash());
				REQUIRE(readTx[i]->Output()->AssetID() == txToSave[i]->Output()->AssetID());
				REQUIRE(readTx[i]->Output()->OutputLock() == txToSave[i]->Output()->OutputLock());
				REQUIRE(readTx[i]->Output()->Amount() == txToSave[i]->Output()->Amount());
				REQUIRE(readTx[i]->Timestamp() == txToSave[i]->Timestamp());
				REQUIRE(readTx[i]->BlockHeight() == txToSave[i]->BlockHeight());
				REQUIRE(readTx[i]->Hash() == txToSave[i]->Hash());
			}
		}

		SECTION("udpate test") {
			DatabaseManager dbm(DBFILE);
			std::vector<uint256> hashes;

			for (int i = 0; i < txToUpdate.size(); ++i)
				hashes.push_back(uint256(txToUpdate[i]->Hash()));
			REQUIRE(dbm.UpdateCoinBase(hashes, txToUpdate[0]->BlockHeight(), txToUpdate[0]->Timestamp()));
		}

		SECTION("read after update test") {
			DatabaseManager dbm(DBFILE);
			std::vector<UTXOPtr> readTx = dbm.GetAllCoinBase();
			REQUIRE(TEST_TX_RECORD_CNT == readTx.size());

			for (int i = 0; i < readTx.size(); ++i) {
				REQUIRE(readTx[i]->Hash() == txToSave[i]->Hash());
				REQUIRE(readTx[i]->Spent() == txToSave[i]->Spent());
				REQUIRE(readTx[i]->Index() == txToSave[i]->Index());
				REQUIRE(readTx[i]->Output()->ProgramHash() == txToSave[i]->Output()->ProgramHash());
				REQUIRE(readTx[i]->Output()->AssetID() == txToSave[i]->Output()->AssetID());
				REQUIRE(readTx[i]->Output()->OutputLock() == txToSave[i]->Output()->OutputLock());
				REQUIRE(readTx[i]->Output()->Amount() == txToSave[i]->Output()->Amount());

				REQUIRE(readTx[i]->Timestamp() == txToUpdate[i]->Timestamp());
				REQUIRE(readTx[i]->BlockHeight() == txToUpdate[i]->BlockHeight());
			}
		}

		SECTION("delete by txHash test") {
			DatabaseManager dbm(DBFILE);

			for (int i = 0; i < txToUpdate.size(); ++i) {
				REQUIRE(dbm.DeleteCoinBase(txToUpdate[i]->Hash()));
			}

			std::vector<UTXOPtr> readTx = dbm.GetAllCoinBase();
			REQUIRE(0 == readTx.size());
		}

	}

	SECTION("Transaction test") {
#define TEST_TX_RECORD_CNT DEFAULT_RECORD_CNT
		static std::vector<TransactionPtr> txToSave;
		static std::vector<TransactionPtr> txToUpdate;

		SECTION("Transaction prepare for testing") {
			for (uint64_t i = 0; i < TEST_TX_RECORD_CNT; ++i) {
				TransactionPtr tx(new Transaction());

				for (size_t i = 0; i < 2; ++i) {
					InputPtr input(new TransactionInput());
					input->SetTxHash(getRanduint256());
					input->SetIndex(getRandUInt16());
					input->SetSequence(getRandUInt32());
					tx->AddInput(input);
				}
				for (size_t i = 0; i < 20; ++i) {
					Address toAddress("EJKPFkAwx7G6dniGMvsb7eG1V8gmhxFU9Z");
					OutputPtr output(new TransactionOutput(10, toAddress));
					tx->AddOutput(output);
				}
				tx->SetBlockHeight(getRandUInt32());
				tx->SetTimestamp(getRandUInt32());
				txToSave.push_back(tx);
			}

			for (uint64_t i = 0; i < TEST_TX_RECORD_CNT; ++i) {
				TransactionPtr tx(new Transaction());
				tx->FromJson(txToSave[i]->ToJson());
				tx->SetBlockHeight((uint32_t) 1234);
				tx->SetTimestamp((uint32_t) 12345678);
				txToUpdate.push_back(tx);
			}
		}

		SECTION("Transaction save test") {
			DatabaseManager dbm(DBFILE);
			for (int i = 0; i < txToSave.size(); ++i) {
				REQUIRE(dbm.PutTransaction(ISO, txToSave[i]));
			}
		}

		SECTION("Transaction read test") {
			DatabaseManager dbm(DBFILE);
			std::vector<TransactionPtr> readTx = dbm.GetAllTransactions(CHAINID_MAINCHAIN);
			REQUIRE(txToSave.size() == readTx.size());

			for (int i = 0; i < readTx.size(); ++i) {
				ByteStream toSaveStream;
				txToSave[i]->Serialize(toSaveStream);
				ByteStream readStream;
				readTx[i]->Serialize(readStream);
				REQUIRE(readStream.GetBytes() == toSaveStream.GetBytes());
				REQUIRE(readTx[i]->GetHash() == txToSave[i]->GetHash());
				REQUIRE(readTx[i]->GetTimestamp() == txToSave[i]->GetTimestamp());
				REQUIRE(readTx[i]->GetBlockHeight() == txToSave[i]->GetBlockHeight());
			}
		}

		SECTION("Transaction udpate test") {
			DatabaseManager dbm(DBFILE);

			std::vector<uint256> hashes;
			for (int i = 0; i < txToUpdate.size(); ++i) {
				hashes.push_back(uint256(txToUpdate[i]->GetHash()));
			}
			REQUIRE(dbm.UpdateTransaction(hashes, txToUpdate[0]->GetBlockHeight(), txToUpdate[0]->GetTimestamp()));
		}

		SECTION("Transaction read after update test") {
			DatabaseManager dbm(DBFILE);
			std::vector<TransactionPtr> readTx = dbm.GetAllTransactions(CHAINID_MAINCHAIN);
			REQUIRE(TEST_TX_RECORD_CNT == readTx.size());

			for (int i = 0; i < readTx.size(); ++i) {
				ByteStream updateStream;
				txToUpdate[i]->Serialize(updateStream);
				ByteStream readStream;
				readTx[i]->Serialize(readStream);

				REQUIRE(readStream.GetBytes() == updateStream.GetBytes());
				REQUIRE(readTx[i]->GetHash() == txToUpdate[i]->GetHash());
				REQUIRE(readTx[i]->GetTimestamp() == txToUpdate[i]->GetTimestamp());
				REQUIRE(readTx[i]->GetBlockHeight() == txToUpdate[i]->GetBlockHeight());
			}
		}

		SECTION("Transaction delete by txHash test") {
			DatabaseManager dbm(DBFILE);

			for (int i = 0; i < txToUpdate.size(); ++i) {
				REQUIRE(dbm.DeleteTxByHash(txToUpdate[i]->GetHash()));
			}

			std::vector<TransactionPtr> readTx = dbm.GetAllTransactions(CHAINID_MAINCHAIN);
			REQUIRE(0 == readTx.size());
		}

	}

	SECTION("DID test") {
#define TEST_DID_RECORD_CNT DEFAULT_RECORD_CNT
		DatabaseManager dm(DBFILE);
		static std::vector<DIDEntity> didToSave;
		// save
		SECTION("Prepare data") {
			for (int i = 0; i < TEST_DID_RECORD_CNT; ++i) {
				DIDEntity didEntity;
				didEntity.DID = getRandHexString(21);
				didEntity.PayloadInfo = getRandBytes(200);
				didEntity.BlockHeight = getRandUInt32();
				didEntity.TimeStamp = getRandUInt64();
				didEntity.TxHash = getRanduint256().GetHex();
				didEntity.CreateTime = getRandUInt64();
				didToSave.push_back(didEntity);

				REQUIRE(dm.PutDID(ISO, didEntity));
			}
		}

		SECTION("Verify prepare data") {
			std::vector<DIDEntity> didVerify = dm.GetAllDID();
			REQUIRE(didVerify.size() == TEST_DID_RECORD_CNT);
			REQUIRE(didVerify.size() == didToSave.size());

			for (size_t i = 0; i < TEST_DID_RECORD_CNT; ++i) {
				REQUIRE(didVerify[i].DID == didToSave[i].DID);
				REQUIRE(didVerify[i].PayloadInfo == didToSave[i].PayloadInfo);
				REQUIRE(didVerify[i].BlockHeight == didToSave[i].BlockHeight);
				REQUIRE(didVerify[i].TimeStamp == didToSave[i].TimeStamp);
				REQUIRE(didVerify[i].CreateTime == didToSave[i].CreateTime);
				REQUIRE(didVerify[i].TxHash == didToSave[i].TxHash);
			}

			DIDEntity detail;
			dm.GetDIDDetails(didToSave[0].DID, detail);
			REQUIRE(detail.DID == didToSave[0].DID);
			REQUIRE(detail.PayloadInfo == didToSave[0].PayloadInfo);
			REQUIRE(detail.BlockHeight == didToSave[0].BlockHeight);
			REQUIRE(detail.TimeStamp == didToSave[0].TimeStamp);
			REQUIRE(detail.CreateTime == didToSave[0].CreateTime);
			REQUIRE(detail.TxHash == didToSave[0].TxHash);
		}

		SECTION("update test") {
			std::vector<DIDEntity> didList = dm.GetAllDID();
			std::vector<uint256> hashList;
			time_t updateTime = getRandUInt64();
			uint32_t updateHeight = getRandUInt32();
			for (size_t i = 0; i < didList.size(); ++i) {
				hashList.push_back(uint256(didToSave[i].TxHash));
			}

			REQUIRE(dm.UpdateDID(hashList, updateHeight, updateTime));

			std::vector<DIDEntity> verifyList = dm.GetAllDID();
			REQUIRE(verifyList.size() == didList.size());

			for (size_t i = 0; i < verifyList.size(); ++i) {
				REQUIRE(verifyList[i].BlockHeight == updateHeight);
				REQUIRE(verifyList[i].TimeStamp == updateTime);
				REQUIRE(verifyList[i].CreateTime == didList[i].CreateTime);
			}
		}

		SECTION("delete test") {
			int idx = getRandUInt8() % didToSave.size();
			REQUIRE(dm.DeleteDID(didToSave[idx].DID));
			std::vector<DIDEntity> didVerify = dm.GetAllDID();
			REQUIRE(didVerify.size() == didToSave.size() - 1);

			idx = getRandUInt8() % didToSave.size();
			REQUIRE(dm.DeleteDIDByTxHash(didToSave[idx].TxHash));
			didVerify = dm.GetAllDID();
			REQUIRE(didVerify.size() == didToSave.size() - 2);

			REQUIRE(dm.DeleteAllDID());
			didVerify = dm.GetAllDID();
			REQUIRE(didVerify.size() == 0);
		}

	}

}
