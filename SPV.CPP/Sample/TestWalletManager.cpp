
#include <iostream>
#include <BRKey.h>
#include <BRPeer.h>
#include <Common/Log.h>
#include "TestWalletManager.h"

#include "Key.h"

TestWalletManager::TestWalletManager() :
		WalletManager("a test seed ha") {
}

WrapperList<Peer, BRPeer> TestWalletManager::loadPeers() {
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

	WrapperList<Peer, BRPeer> peers(1);
	int test = peers.size();
	peers[0] = p1;
/*	peers[1] = p2;
	peers[2] = p3;*/
	return peers;
}