
#ifndef _ELATSOS_SAMPLE_TESTWALLET_H
#define _ELATSOS_SAMPLE_TESTWALLET_H


#include "Manager/WalletManager.h"
#include "Wrapper/Transaction.h"

using namespace Elastos::SDK;

class TestWalletManager : public WalletManager {

public:
	TestWalletManager();

	void testSendTransaction();

	void testCreateTransaction();

protected:

	SharedWrapperList<Peer, BRPeer*> loadPeers() override;

};


#endif //_ELATSOS_SAMPLE_TESTWALLET_H
