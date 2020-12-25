// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#define CATCH_CONFIG_MAIN

#include <catch.hpp>
#include "TestHelper.h"

#include <Database/DatabaseManager.h>
#include <Common/Log.h>
#include <Plugin/Registry.h>
#include <Plugin/ELAPlugin.h>
#include <Plugin/IDPlugin.h>
#include <Plugin/TokenPlugin.h>

#include <fstream>
#include <Plugin/Transaction/Payload/CoinBase.h>
#include <Plugin/Transaction/Payload/TransferAsset.h>
#include <Plugin/Transaction/Payload/RechargeToSideChain.h>
#include <Plugin/Transaction/Payload/WithdrawFromSideChain.h>
#include <Plugin/Transaction/Payload/TransferCrossChainAsset.h>
#include <Plugin/Transaction/Payload/ProducerInfo.h>
#include <Plugin/Transaction/Payload/CancelProducer.h>
#include <Plugin/Transaction/Payload/ReturnDepositCoin.h>
#include <Plugin/Transaction/Payload/NextTurnDPoSInfo.h>
#include <Plugin/Transaction/Payload/CRInfo.h>
#include <Plugin/Transaction/Payload/UnregisterCR.h>

using namespace Elastos::ElaWallet;

#define DBFILE "wallet.db"

TEST_CASE("DatabaseManager test", "[DatabaseManager]") {
	Log::registerMultiLogger();
	std::string pluginType = "ELA";
#define DEFAULT_RECORD_CNT 20

	srand(time(nullptr));

	SECTION("Asset test") {
#define TEST_ASSET_RECORD_CNT DEFAULT_RECORD_CNT
		DatabaseManager dm(DBFILE);

		REQUIRE(dm.DeleteAllAssets());

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
			REQUIRE(dbm->DeleteAllBlocks());
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
			REQUIRE(dbm.DeleteAllPeers());
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
			REQUIRE(dbm.DeleteAllBlackPeers());
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
#define TEST_TX_RECORD_CNT 120

		SECTION("Transaction save and read test") {
			DatabaseManager dbm(DBFILE);

			REQUIRE(dbm.DeleteAllTx());
			REQUIRE(dbm.GetAllTxCnt() == 0);
			std::vector<TxEntity> entities;
			std::vector<TransactionPtr> txs;
			std::map<std::string, TransactionPtr> txmap;

			// put 1
			for (int i = 0; i < 10; ++i) {
				TxEntity e;
				PayloadPtr payload = PayloadPtr(new CoinBase(getRandBytes(50)));
				TransactionPtr tx(new Transaction(Transaction::coinBase, payload));
				initTransaction(*tx, Transaction::V09);
				tx->SetPayloadVersion(0);
				tx->Encode(e);
				txs.push_back(tx);
				entities.push_back(e);
				txmap[tx->GetHash().GetHex()] = tx;
			}

			// put 2
			for (int i = 0; i < 10; ++i) {
				TxEntity e;
				PayloadPtr payload = PayloadPtr(new TransferAsset());
				TransactionPtr tx(new Transaction(Transaction::transferAsset, payload));
				initTransaction(*tx, Transaction::V09);
				tx->SetPayloadVersion(0);
				tx->Encode(e);
				txs.push_back(tx);
				entities.push_back(e);
				txmap[tx->GetHash().GetHex()] = tx;
			}

			// put 3
			for (int i = 0; i < 10; ++i) {
				TxEntity e;
				PayloadPtr payload = PayloadPtr(new RechargeToSideChain(getRandBytes(20), getRandBytes(30), getRanduint256()));
				TransactionPtr tx(new Transaction(Transaction::rechargeToSideChain, payload));
				initTransaction(*tx, Transaction::V09);
				tx->SetPayloadVersion(RechargeToSideChain::V1);
				tx->Encode(e);
				txs.push_back(tx);
				entities.push_back(e);
				txmap[tx->GetHash().GetHex()] = tx;
			}

			// put 4
			for (int i = 0; i < 10; ++i) {
				TxEntity e;
				PayloadPtr payload = PayloadPtr(new WithdrawFromSideChain(getRandUInt32(), getRandString(30), {getRanduint256()}));
				TransactionPtr tx(new Transaction(Transaction::withdrawFromSideChain, payload));
				initTransaction(*tx, Transaction::V09);
				tx->SetPayloadVersion(0);
				tx->Encode(e);
				txs.push_back(tx);
				entities.push_back(e);
				txmap[tx->GetHash().GetHex()] = tx;
			}

			// put 5
			for (int i = 0; i < 10; ++i) {
				TxEntity e;
				PayloadPtr payload = PayloadPtr(new TransferCrossChainAsset({TransferInfo(getRandString(30), getRandUInt16(), getRandBigInt())}));
				TransactionPtr tx(new Transaction(Transaction::transferCrossChainAsset, payload));
				initTransaction(*tx, Transaction::V09);
				tx->SetPayloadVersion(0);
				tx->Encode(e);
				txs.push_back(tx);
				entities.push_back(e);
				txmap[tx->GetHash().GetHex()] = tx;
			}

			// put 6
			for (int i = 0; i < 10; ++i) {
				TxEntity e;
				PayloadPtr payload = PayloadPtr(new ProducerInfo(getRandBytes(33),
																 getRandBytes(33),
																 getRandString(10),
																 getRandString(20),
																 getRandUInt64(),
																 getRandString(30),
																 getRandBytes(64)));
				TransactionPtr tx(new Transaction(Transaction::registerProducer, payload));
				initTransaction(*tx, Transaction::V09);
				tx->SetPayloadVersion(0);
				tx->Encode(e);
				txs.push_back(tx);
				entities.push_back(e);
				txmap[tx->GetHash().GetHex()] = tx;
			}

			// put 7
			for (int i = 0; i < 10; ++i) {
				TxEntity e;
				PayloadPtr payload = PayloadPtr(new CancelProducer(getRandBytes(33),
																   getRandBytes(64)));
				TransactionPtr tx(new Transaction(Transaction::cancelProducer, payload));
				initTransaction(*tx, Transaction::V09);
				tx->SetPayloadVersion(0);
				tx->Encode(e);
				txs.push_back(tx);
				entities.push_back(e);
				txmap[tx->GetHash().GetHex()] = tx;
			}

			// put 8
			for (int i = 0; i < 10; ++i) {
				TxEntity e;
				PayloadPtr payload = PayloadPtr(new ReturnDepositCoin());
				TransactionPtr tx(new Transaction(Transaction::returnDepositCoin, payload));
				initTransaction(*tx, Transaction::V09);
				tx->SetPayloadVersion(0);
				tx->Encode(e);
				txs.push_back(tx);
				entities.push_back(e);
				txmap[tx->GetHash().GetHex()] = tx;
			}

			// put 9
			for (int i = 0; i < 10; ++i) {
				TxEntity e;
				PayloadPtr payload = PayloadPtr(new NextTurnDPoSInfo(getRandUInt32(), {getRandBytes(33)}, {getRandBytes(33)}));
				TransactionPtr tx(new Transaction(Transaction::nextTurnDPOSInfo, payload));
				initTransaction(*tx, Transaction::V09);
				tx->SetPayloadVersion(0);
				tx->Encode(e);
				txs.push_back(tx);
				entities.push_back(e);
				txmap[tx->GetHash().GetHex()] = tx;
			}

			// put 10
			for (int i = 0; i < 10; ++i) {
				TxEntity e;
				PayloadPtr payload = PayloadPtr(new CRInfo(getRandBytes(21),
														   getRandUInt168(),
														   getRandUInt168(),
														   getRandString(10),
														   getRandString(30),
														   getRandUInt64(),
														   getRandBytes(64)));
				TransactionPtr tx(new Transaction(Transaction::registerCR, payload));
				initTransaction(*tx, Transaction::V09);
				tx->SetPayloadVersion(CRInfoDIDVersion);
				tx->Encode(e);
				txs.push_back(tx);
				entities.push_back(e);
				txmap[tx->GetHash().GetHex()] = tx;
			}

			// put 11
			for (int i = 0; i < 10; ++i) {
				TxEntity e;
				PayloadPtr payload = PayloadPtr(new UnregisterCR(getRandUInt168(), getRandBytes(64)));
				TransactionPtr tx(new Transaction(Transaction::unregisterCR, payload));
				initTransaction(*tx, Transaction::V09);

				tx->SetPayloadVersion(0);
				tx->Encode(e);
				txs.push_back(tx);
				entities.push_back(e);
				txmap[tx->GetHash().GetHex()] = tx;
			}

			// put 12
			for (int i = 0; i < 10; ++i) {
				TxEntity e;
				PayloadPtr payload = PayloadPtr(new ReturnDepositCoin());
				TransactionPtr tx(new Transaction(Transaction::returnCRDepositCoin, payload));
				initTransaction(*tx, Transaction::V09);
				tx->SetPayloadVersion(0);
				tx->Encode(e);
				txs.push_back(tx);
				entities.push_back(e);
				txmap[tx->GetHash().GetHex()] = tx;
			}

			REQUIRE(dbm.PutTx(entities));

			// read & verify tx
			entities.clear();
			REQUIRE(dbm.GetTx(entities));
			REQUIRE(entities.size() == txs.size());

			for (int i = 0; i < entities.size(); ++i) {
				TransactionPtr tx(new Transaction());
				REQUIRE(tx->Decode(entities[i]));
				REQUIRE((*tx == *txs[i]));
			}

			// update
			std::vector<std::string> hashes;
			for (int i = TEST_TX_RECORD_CNT / 2; i < TEST_TX_RECORD_CNT; ++i)
				hashes.push_back(txs[i]->GetHash().GetHex());
			REQUIRE(dbm.UpdateTx(hashes, TX_UNCONFIRMED, 0));

			// verify update
			entities.clear();
			REQUIRE(dbm.GetTx(entities));
			for (int i = TEST_TX_RECORD_CNT / 2; i < TEST_TX_RECORD_CNT; ++i) {
				TransactionPtr tx(new Transaction());
				REQUIRE(tx->Decode(entities[i]));
				REQUIRE(tx->GetBlockHeight() == TX_UNCONFIRMED);
				REQUIRE(tx->GetTimestamp() == 0);
			}

			// make fake utxo
			std::vector<UTXOEntity> utxoEntities;
			for (int i = 0; i < TEST_TX_RECORD_CNT / 2; ++i) {
				UTXOEntity e(txs[i]->GetHash().GetHex(), 0);
				utxoEntities.push_back(e);
			}
			REQUIRE(dbm.DeleteAllUTXOs());
			REQUIRE(dbm.PutUTXOs(utxoEntities));

			// read & verify utxo tx
			entities.clear();
			REQUIRE(dbm.GetUTXOTx(entities));
			for (int i = 0; i < TEST_TX_RECORD_CNT / 2; ++i)
				REQUIRE(txmap.find(entities[i].GetTxHash()) != txmap.end());

			// delete
			for (int i = 0; i < TEST_TX_RECORD_CNT / 2; ++i)
				REQUIRE(dbm.DeleteTx(txs[i]->GetHash().GetHex()));

			// verify delete
			entities.clear();
			REQUIRE(dbm.GetTx(entities));
			REQUIRE(entities.size() == TEST_TX_RECORD_CNT / 2);
			for (int i = TEST_TX_RECORD_CNT / 2; i < TEST_TX_RECORD_CNT; ++i) {
				TransactionPtr tx(new Transaction());
				REQUIRE(tx->Decode(entities[i - TEST_TX_RECORD_CNT / 2]));
				REQUIRE(tx->GetHash().GetHex() == txs[i]->GetHash().GetHex());
			}
		}
	}

	SECTION("UTXO Store Test") {
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

		REQUIRE(dbm->DeleteAllUTXOs());

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
#define TEST_USED_ADDRESS_CNT DEFAULT_RECORD_CNT
		std::vector<std::string> usedAddress;

		// prepare data
		for (int i = 0; i < TEST_USED_ADDRESS_CNT; ++i) {
			std::string hash = getRanduint256().GetHex();
			usedAddress.push_back(hash);
		}

		// save
		DatabaseManager *dbm = new DatabaseManager(DBFILE);
		REQUIRE(dbm->DeleteAllUsedAddresses());
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

}

