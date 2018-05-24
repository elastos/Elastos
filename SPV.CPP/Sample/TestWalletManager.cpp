
#include <iostream>
#include <BRKey.h>
#include <BRPeer.h>
#include <Common/Log.h>
#include <BRTransaction.h>
#include "TestWalletManager.h"
#include "Utils.h"
#include "ELACoreExt/Payload/Asset.h"

#include "Key.h"

TestWalletManager::TestWalletManager() :
	WalletManager((const CMBlock)Utils::convertToMemBlock<uint8_t>("a test seed ha")) {
}

SharedWrapperList<Peer, BRPeer*> TestWalletManager::loadPeers() {
	PeerPtr p1 = PeerPtr(new Peer(7630401));
	BRPeer *brp1 = p1->getRaw();
	brp1->address = ((UInt128) {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0xff, 0xff, 127, 0, 0, 1});
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

	SharedWrapperList<Peer, BRPeer*> peers(1);
	int test = peers.size();
	peers[0] = p1;
/*	peers[1] = p2;
	peers[2] = p3;*/
	return peers;
}

void TestWalletManager::testSendTransaction() {

	Transaction elaCoin;
	elaCoin.setTransactionType(Transaction::Type::RegisterAsset);

	Transaction transaction;
	TransactionInput input;
	input.setAddress("EWEfdKMyjPkAkHtvxiDvsRPssQi2Ymeupr");
	BRTxInput *brTxInput = input.getRaw();
	UInt256 hash = uint256("000000000019d6689c085ae165831e934ff763ae46a2a6c172b3f1b60a8ce26f");
	brTxInput->txHash = hash;
	brTxInput->amount = 400000000;
	brTxInput->index = 567;
	brTxInput->sequence = 98888;
	brTxInput->scriptLen = 0;
	brTxInput->script = nullptr;
	transaction.addInput(input);

	TransactionOutput transactionOutput1;
	transactionOutput1.setAddress("ETFELUtMYwPpb96QrYaP6tBztEsUbQrytP");
	transactionOutput1.setAmount(100000000);
	hash = elaCoin.getHash();
	transactionOutput1.setAssetId(hash);
	transactionOutput1.setOutputLock(1);
	transactionOutput1.setProgramHash(Utils::UInt168FromString("215505da55ee9de910658619b4d5e4e6c59acaeb00"));

	TransactionOutput transactionOutput2;
	transactionOutput2.setAddress("EQuTwZ7sQzXyoteFxuwyqhVHqBsh4kFVhV");
	transactionOutput2.setAmount(150598174);
	hash = elaCoin.getHash();
	transactionOutput2.setAssetId(hash);
	transactionOutput2.setOutputLock(2);
	transactionOutput2.setProgramHash(Utils::UInt168FromString("2119acda6f6e57aceb7572260ff1e9e6fc50b99733"));

	transaction.addOutput(transactionOutput1);
	transaction.addOutput(transactionOutput2);
	transaction.setTransactionType(Transaction::Type::TransferAsset);

	ByteStream byteStream;
	transaction.Serialize(byteStream);
	byteStream.setPosition(0);
	TransactionPtr ptr(new Transaction());
	ptr->Deserialize(byteStream);
	hash = signAndPublishTransaction(ptr);

	Log::getLogger()->info("signAndPublishTransaction hash:{}", Utils::UInt256ToString(hash));

}

void TestWalletManager::testCreateTransaction() {
	Transaction elaCoin;
	elaCoin.setTransactionType(Transaction::Type::RegisterAsset);

	UInt256 hash = elaCoin.getHash();

	TxParam normalParam;
	normalParam.setToAddress("ETFELUtMYwPpb96QrYaP6tBztEsUbQrytP");
	normalParam.setAmount(12);
	normalParam.setAssetId(hash);
	TransactionPtr transaction = createTransaction(normalParam);
	if (transaction) {
		UInt256 hash = signAndPublishTransaction(transaction);

		Log::getLogger()->info("testCreateTransaction hash:{}", Utils::UInt256ToString(hash));
	}
}