// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

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
	TransactionPtr createTransaction(const TxParam &param);

	SharedWrapperList<Peer, BRPeer*> loadPeers() override;

};


#endif //_ELATSOS_SAMPLE_TESTWALLET_H
