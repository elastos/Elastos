
#include <iostream>
#include <BRKey.h>
#include <BRPeer.h>
#include <Common/Log.h>
#include <BRTransaction.h>
#include "TestWalletManager.h"
#include "Utils.h"

#include "Key.h"

TestWalletManager::TestWalletManager() :
		WalletManager("a test seed ha") {
}

WrapperList<Peer, BRPeer> TestWalletManager::loadPeers() {
	Peer p1(7630401);
	BRPeer *brp1 = p1.getRaw();
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

	WrapperList<Peer, BRPeer> peers(1);
	int test = peers.size();
	peers[0] = p1;
/*	peers[1] = p2;
	peers[2] = p3;*/
	return peers;
}

void TestWalletManager::testSendTransaction() {
	std::string str = "02000100133535373730303637393139343737373934313001ff8972534dcdd3700a2279442ca5cd6b9b6639f4710467a18244ba637c672f030100feffffff02b037db964a231458d2d6ffd5ea18944c4f90e63d547c5d3b9874df66a4ead0a300c2eb0b0000000000000000215505da55ee9de910658619b4d5e4e6c59acaeb00b037db964a231458d2d6ffd5ea18944c4f90e63d547c5d3b9874df66a4ead0a31ef2f90800000000000000002119acda6f6e57aceb7572260ff1e9e6fc50b997330000000001414040dac1c038dfc66c208743bf5be6e9857e2132ede1365a92e60f59b51035e3c818b9a833aecca9eeb658e0e73ecf13109beb7611bc893de37c949a5849b05a1f232103bb6a8c5a716a5002f0d802aecba3766eb58f11dd04d01033f932ce96fd121db7ac";

	uint8_t *script = new uint8_t[str.length() / 2];
	Utils::decodeHex(script, str.length() / 2, (char *)str.c_str(), str.length());

	Transaction transaction;
	ByteStream byteStream(script, str.length() / 2);
	transaction.Deserialize(byteStream);

	BRTransaction *brTransaction = transaction.convertToRaw();
	BRTransaction *temp = BRTransactionNew();
	brTransaction->version = temp->version;
	brTransaction->lockTime = temp->lockTime;
	brTransaction->blockHeight = temp->blockHeight;

	TransactionPtr ptr(new Transaction(brTransaction));
	UInt256 hash = signAndPublishTransaction(ptr);

	Log::getLogger()->info("signAndPublishTransaction hash:{}",Utils::UInt256ToString(hash));
}