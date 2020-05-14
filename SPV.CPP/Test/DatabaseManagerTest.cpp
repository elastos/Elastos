// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#define CATCH_CONFIG_MAIN

#include <catch.hpp>
#include "TestHelper.h"

#include <Database/TransactionNormal.h>
#include <Database/DatabaseManager.h>
#include <Common/Log.h>
#include <Wallet/UTXO.h>
#include <Plugin/Registry.h>
#include <Plugin/ELAPlugin.h>
#include <Plugin/IDPlugin.h>
#include <Plugin/TokenPlugin.h>

#include <fstream>

using namespace Elastos::ElaWallet;

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
			REQUIRE(dm.PutAsset("ELA", asset));
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
		REGISTER_MERKLEBLOCKPLUGIN(IDChain, getIDPluginComponent);
		REGISTER_MERKLEBLOCKPLUGIN(TokenChain, getTokenPluginComponent);
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
			MerkleBlockPtr merkleBlock(Registry::Instance()->CreateMerkleBlock(pluginType));

			merkleBlock->SetHeight(110);
			merkleBlock->SetTimestamp(getRandUInt32());
			merkleBlock->SetPrevBlockHash(uint256(getRandBytes(32)));
			merkleBlock->SetTarget(getRandUInt32());
			merkleBlock->SetNonce(getRandUInt32());

			REQUIRE(dbm->PutMerkleBlock(merkleBlock));
			REQUIRE(dbm->GetAllMerkleBlocks(pluginType).size() == 1);

			REQUIRE(dbm->PutMerkleBlocks(true, blocksToSave));
			delete dbm;
		}

		SECTION("Merkle Block read test") {
			DatabaseManager dbm(DBFILE);
			std::vector<MerkleBlockPtr> blocksRead = dbm.GetAllMerkleBlocks(pluginType);
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

			REQUIRE(dbm.DeleteAllBlocks());

			std::vector<MerkleBlockPtr> blocksAfterDelete = dbm.GetAllMerkleBlocks(pluginType);
			REQUIRE(0 == blocksAfterDelete.size());
		}

		SECTION("Merkle Block save one by one test") {
			DatabaseManager dbm(DBFILE);
			for (int i = 0; i < blocksToSave.size(); ++i) {
				REQUIRE(dbm.PutMerkleBlock(blocksToSave[i]));
			}
		}

		SECTION("Merkle Block read test") {
			DatabaseManager dbm(DBFILE);
			std::vector<MerkleBlockPtr> blocksRead = dbm.GetAllMerkleBlocks(pluginType);
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

			dbm.DeleteAllBlocks();

			std::vector<MerkleBlockPtr> blocksAfterDelete = dbm.GetAllMerkleBlocks(pluginType);
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
			REQUIRE(dbm.PutPeers(peerToSave));
		}

		SECTION("Peer read test") {
			DatabaseManager dbm(DBFILE);
			std::vector<PeerEntity> peers = dbm.GetAllPeers();
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
			std::vector<PeerEntity> peers = dbm->GetAllPeers();
			REQUIRE(peers.size() == 0);
			delete dbm;
		}

		SECTION("Peer save one by one test") {
			DatabaseManager dbm(DBFILE);
			for (int i = 0; i < peerToSave.size(); ++i) {
				REQUIRE(dbm.PutPeer(peerToSave[i]));
			}
		}

		SECTION("Peer read test") {
			DatabaseManager dbm(DBFILE);
			std::vector<PeerEntity> peers = dbm.GetAllPeers();
			REQUIRE(peers.size() == peerToSave.size());
			for (int i = 0; i < peers.size(); i++) {
				REQUIRE(peers[i].address == peerToSave[i].address);
				REQUIRE(peers[i].port == peerToSave[i].port);
				REQUIRE(peers[i].timeStamp == peerToSave[i].timeStamp);
			}
		}

		SECTION("Peer delete one by one test") {
			DatabaseManager dbm(DBFILE);

			std::vector<PeerEntity> PeersBeforeDelete = dbm.GetAllPeers();
			REQUIRE(PeersBeforeDelete.size() == peerToSave.size());

			for (int i = 0; i < PeersBeforeDelete.size(); ++i) {
				REQUIRE(dbm.DeletePeer(PeersBeforeDelete[i]));
			}

			std::vector<PeerEntity> PeersAfterDelete = dbm.GetAllPeers();
			REQUIRE(0 == PeersAfterDelete.size());
		}
	}

	SECTION("Peer black list test") {
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
			REQUIRE(dbm.PutBlackPeers(peerToSave));
		}

		SECTION("Peer read test") {
			DatabaseManager dbm(DBFILE);
			std::vector<PeerEntity> peers = dbm.GetAllBlackPeers();
			REQUIRE(peers.size() == peerToSave.size());
			for (int i = 0; i < peers.size(); i++) {
				REQUIRE(peers[i].address == peerToSave[i].address);
				REQUIRE(peers[i].port == peerToSave[i].port);
				REQUIRE(peers[i].timeStamp == peerToSave[i].timeStamp);
			}
			REQUIRE(dbm.PutBlackPeer(peers[0]));
			REQUIRE(dbm.GetAllBlackPeers().size() == peers.size());
		}

		SECTION("Peer delete test") {
			DatabaseManager *dbm = new DatabaseManager(DBFILE);
			REQUIRE(dbm->DeleteAllBlackPeers());
			std::vector<PeerEntity> peers = dbm->GetAllBlackPeers();
			REQUIRE(peers.size() == 0);
			delete dbm;
		}

		SECTION("Peer save one by one test") {
			DatabaseManager dbm(DBFILE);
			for (int i = 0; i < peerToSave.size(); ++i) {
				REQUIRE(dbm.PutBlackPeer(peerToSave[i]));
			}
		}

		SECTION("Peer read test") {
			DatabaseManager dbm(DBFILE);
			std::vector<PeerEntity> peers = dbm.GetAllBlackPeers();
			REQUIRE(peers.size() == peerToSave.size());
			for (int i = 0; i < peers.size(); i++) {
				REQUIRE(peers[i].address == peerToSave[i].address);
				REQUIRE(peers[i].port == peerToSave[i].port);
				REQUIRE(peers[i].timeStamp == peerToSave[i].timeStamp);
			}
		}

		SECTION("Peer delete one by one test") {
			DatabaseManager dbm(DBFILE);

			std::vector<PeerEntity> PeersBeforeDelete = dbm.GetAllBlackPeers();
			REQUIRE(PeersBeforeDelete.size() == peerToSave.size());

			for (int i = 0; i < PeersBeforeDelete.size(); ++i) {
				REQUIRE(dbm.DeleteBlackPeer(PeersBeforeDelete[i]));
			}

			std::vector<PeerEntity> PeersAfterDelete = dbm.GetAllBlackPeers();
			REQUIRE(0 == PeersAfterDelete.size());
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
				REQUIRE(dbm.PutNormalTxn(txToSave[i]));
			}
		}

		SECTION("Transaction read test") {
			DatabaseManager dbm(DBFILE);
			std::vector<TransactionPtr> readTx = dbm.GetNormalTxns(CHAINID_MAINCHAIN);
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

			REQUIRE(dbm.UpdateNormalTxn(txToUpdate));
		}

		SECTION("Transaction read after update test") {
			DatabaseManager dbm(DBFILE);
			std::vector<TransactionPtr> readTx = dbm.GetNormalTxns(CHAINID_MAINCHAIN);
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
				REQUIRE(dbm.DeleteNormalTxn(txToUpdate[i]->GetHash()));
			}

			std::vector<TransactionPtr> readTx = dbm.GetNormalTxns(CHAINID_MAINCHAIN);
			REQUIRE(0 == readTx.size());
		}

	}

	SECTION("UTXO Store Test") {
		if (boost::filesystem::exists(DBFILE) && boost::filesystem::is_regular_file(DBFILE)) {
			boost::filesystem::remove(DBFILE);
		}

		REQUIRE(!boost::filesystem::exists(DBFILE));
#define TEST_UTXO_RECORD_CNT DEFAULT_RECORD_CNT
		std::vector<UTXOEntity> utxoToSave;

		// prepare data
		for (int i = 0; i < TEST_UTXO_RECORD_CNT; ++i) {
			std::string hash = getRanduint256().GetHex();
			uint16_t index = getRandUInt16();

			UTXOEntity entity(hash, index);
			utxoToSave.push_back(entity);
		}

		// save
		DatabaseManager *dbm = new DatabaseManager(DBFILE);

		REQUIRE(dbm->PutUTXOs(utxoToSave));

		// read & verify
		std::vector<UTXOEntity> entitys = dbm->GetUTXOs();
		REQUIRE(TEST_UTXO_RECORD_CNT == entitys.size());
		for (int i = 0; i < TEST_UTXO_RECORD_CNT; ++i) {
			REQUIRE(utxoToSave[i].Hash() == entitys[i].Hash());
			REQUIRE(utxoToSave[i].Index() == entitys[i].Index());
		}

		// delete
		std::vector<UTXOEntity> deleteEntitys;
		for (int i = 0; i < TEST_UTXO_RECORD_CNT; ++i) {
			if (i % 2 == 0) {
				deleteEntitys.push_back(utxoToSave[i]);
			}
		}
		REQUIRE(dbm->DeleteUTXOs(deleteEntitys));

		// read & verify
		entitys = dbm->GetUTXOs();
		REQUIRE(utxoToSave.size() - deleteEntitys.size() == entitys.size());
		for (int i = 0; i < deleteEntitys.size(); ++i) {
			bool found = false;
			for (int j = 0; j < entitys.size(); ++j) {
				if (deleteEntitys[i] == entitys[j]) {
					found = true;
					break;
				}
			}
			REQUIRE(!found);
		}

		// delete all
		REQUIRE(dbm->DeleteAllUTXOs());
		REQUIRE(dbm->GetUTXOs().empty());

		delete dbm;

		DatabaseManager *dbmNew = new DatabaseManager(DBFILE);
		delete dbmNew;
	}

	SECTION("Used address Test") {
		if (boost::filesystem::exists(DBFILE) && boost::filesystem::is_regular_file(DBFILE)) {
			boost::filesystem::remove(DBFILE);
		}

		REQUIRE(!boost::filesystem::exists(DBFILE));
#define TEST_USED_ADDRESS_CNT DEFAULT_RECORD_CNT
		std::vector<std::string> usedAddress;

		// prepare data
		for (int i = 0; i < TEST_USED_ADDRESS_CNT; ++i) {
			std::string hash = getRanduint256().GetHex();
			usedAddress.push_back(hash);
		}

		// save
		DatabaseManager *dbm = new DatabaseManager(DBFILE);
		REQUIRE(dbm->PutUsedAddresses(usedAddress, false));
		// test insert or replace
		REQUIRE(dbm->PutUsedAddresses(usedAddress, false));

		// read & verify
		std::vector<std::string> usedAddressVerify = dbm->GetUsedAddresses();
		REQUIRE(TEST_USED_ADDRESS_CNT == usedAddressVerify.size());
		for (int i = 0; i < TEST_USED_ADDRESS_CNT; ++i)
			REQUIRE(usedAddress[i] == usedAddressVerify[i]);

		// save and replace
		usedAddress.clear();

		// prepare data
		for (int i = 0; i < TEST_USED_ADDRESS_CNT; ++i) {
			std::string hash = getRanduint256().GetHex();
			usedAddress.push_back(hash);
		}

		// save & replace
		REQUIRE(dbm->PutUsedAddresses(usedAddress, true));

		// read & verify
		usedAddressVerify = dbm->GetUsedAddresses();
		REQUIRE(TEST_USED_ADDRESS_CNT == usedAddressVerify.size());
		for (int i = 0; i < TEST_USED_ADDRESS_CNT; ++i)
			REQUIRE(usedAddress[i] == usedAddressVerify[i]);

		// delete all
		REQUIRE(dbm->DeleteAllUsedAddresses());
		REQUIRE(dbm->GetUTXOs().empty());

		delete dbm;
	}

	SECTION("UTXO and tx test") {
		// prepare data
		std::vector<TransactionPtr> txns;
		std::vector<std::string> txHashDPoS;
#define TEST_TXNS_COUNT 10 // 1500

		for (uint64_t i = 0; i < TEST_TXNS_COUNT; ++i) {
			TransactionPtr tx(new Transaction());
			initTransaction(*tx, Transaction::TxVersion::V09);
			txns.push_back(tx);
			if (txHashDPoS.size() < TEST_TXNS_COUNT)
				txHashDPoS.push_back(tx->GetHash().GetHex());
		}

		DatabaseManager dm(DBFILE);
		REQUIRE(dm.PutNormalTxns(txns));
		REQUIRE(dm.PutTxHashDPoS(txHashDPoS));

		std::vector<TransactionPtr> txnsDPoS = dm.GetTxDPoS(CHAINID_MAINCHAIN);

		REQUIRE(txnsDPoS.size() == TEST_TXNS_COUNT);
	}

}

