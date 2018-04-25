
#ifndef _ELATSOS_SAMPLE_TESTWALLET_H
#define _ELATSOS_SAMPLE_TESTWALLET_H


#include "Wrapper/CoreWalletManager.h"
#include "Wrapper/Transaction.h"

using namespace Elastos::SDK;

class TestWallet : public CoreWalletManager {

public:
	TestWallet(const MasterPubKeyPtr &masterPubKey,
					  const ChainParams &chainParams,
					  uint32_t earliestPeerTime);

	void balanceChanged(uint64_t balance) override;

	void onTxAdded(Transaction *transaction) override;

	void onTxUpdated(const std::string &hash, uint32_t blockHeight, uint32_t timeStamp) override;

	void onTxDeleted(const std::string &hash, bool notifyUser, bool recommendRescan) override;

public:
	void syncStarted() override;

	void syncStopped(const std::string &error) override;

	void txStatusUpdate() override;

	void saveBlocks(bool replace, const SharedWrapperList<MerkleBlock, BRMerkleBlock *> &blocks) override;

	void savePeers(bool replace, const WrapperList<Peer, BRPeer> &peers) override;

	bool networkIsReachable() override;

	void txPublished(const std::string &error) override;

protected:
	SharedWrapperList<Transaction, BRTransaction*> loadTransactions() override;

	SharedWrapperList<MerkleBlock, BRMerkleBlock *> loadBlocks() override;

	WrapperList<Peer, BRPeer> loadPeers() override;

};


#endif //_ELATSOS_SAMPLE_TESTWALLET_H
