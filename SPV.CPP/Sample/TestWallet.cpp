
#include <iostream>
#include <BRKey.h>
#include <BRPeer.h>
#include <Common/Log.h>
#include "TestWallet.h"

#include "Key.h"

TestWallet::TestWallet(const MasterPubKeyPtr &masterPubKey, const ChainParams &chainParams, uint32_t earliestPeerTime)
	: CoreWalletManager(masterPubKey, chainParams, earliestPeerTime) {

}

void TestWallet::balanceChanged(uint64_t balance) {
	Elastos::SDK::Log::info("balanceChanged!");
}

void TestWallet::onTxAdded(Transaction *transaction) {
	Elastos::SDK::Log::info("onTxAdded!");
}

void TestWallet::onTxUpdated(const std::string &hash, uint32_t blockHeight, uint32_t timeStamp) {
	Elastos::SDK::Log::info("onTxUpdated!");
}

void TestWallet::onTxDeleted(const std::string &hash, bool notifyUser, bool recommendRescan) {
	Elastos::SDK::Log::info("onTxDeleted!");
}

void TestWallet::syncStarted() {
	Elastos::SDK::Log::info("syncStarted!");
}

void TestWallet::syncStopped(const std::string &error) {
	Elastos::SDK::Log::info("syncStopped!");
}

void TestWallet::txStatusUpdate() {
	Elastos::SDK::Log::info("txStatusUpdate!");
}

void TestWallet::saveBlocks(bool replace, const SharedWrapperList<MerkleBlock, BRMerkleBlock *> &blocks) {
	Elastos::SDK::Log::info("saveBlocks!");
}

void TestWallet::savePeers(bool replace, const WrapperList<Peer, BRPeer> &peers) {
	Elastos::SDK::Log::info("savePeers!");
}

bool TestWallet::networkIsReachable() {
	Elastos::SDK::Log::info("networkIsReachable!");
	return true;
}

void TestWallet::txPublished(const std::string &error) {
	Elastos::SDK::Log::info("txPublished!");
}

SharedWrapperList<Transaction, BRTransaction *> TestWallet::loadTransactions() {
	SharedWrapperList<Transaction, BRTransaction *> transactions;
	return transactions;
}

SharedWrapperList<MerkleBlock, BRMerkleBlock *> TestWallet::loadBlocks() {
	SharedWrapperList<MerkleBlock, BRMerkleBlock *> blocks;
	return blocks;
}

WrapperList<Peer, BRPeer> TestWallet::loadPeers() {
	Peer p1(7630401);
	BRPeer *brp1 = p1.getRaw();
	brp1->address = ((UInt128) {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0xff, 0xff, 0x7f, 0x00, 0x00, 0x01});
	brp1->port = 20866;
	brp1->timestamp = time(nullptr);
	brp1->services = uint64_t(1);

	//todo for test
/*	Peer p2(7630401);
	BRPeer *brp2 = p2.getRaw();
	brp2->address = ((UInt128) {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0xff, 0xff, 0x7f, 0x00, 0x00, 0x01});
	brp2->port = 21338;
	brp2->timestamp = time(nullptr);
	brp2->services = uint64_t(1);

	Peer p3(7630401);
	BRPeer *brp3 = p3.getRaw();
	brp3->address = ((UInt128) {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0xff, 0xff, 0x7f, 0x00, 0x00, 0x01});
	brp3->port = 31338;
	brp3->timestamp = time(nullptr);
	brp3->services = uint64_t(1);*/

	WrapperList<Peer, BRPeer> peers(3);
	int test = peers.size();
	peers[0] = p1;
/*	peers[1] = p2;
	peers[2] = p3;*/
	return peers;
}