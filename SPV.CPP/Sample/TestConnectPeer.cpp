

#include <climits>
#include <iostream>
#include <boost/scoped_ptr.hpp>

#include "BRChainParams.h"

#include "Wrapper/ChainParams.h"
#include "MasterWalletManager.h"
#include "Log.h"

#include "TestConnectPeer.h"

using namespace Elastos::ElaWallet;

static int BRMainNetVerifyDifficulty(const BRMerkleBlock *block, const BRSet *blockSet) {
	const BRMerkleBlock *previous, *b = NULL;
	uint32_t i;

	assert(block != NULL);
	assert(blockSet != NULL);

	// check if we hit a difficulty transition, and find previous transition block
	if ((block->height % BLOCK_DIFFICULTY_INTERVAL) == 0) {
		for (i = 0, b = block; b && i < BLOCK_DIFFICULTY_INTERVAL; i++) {
			b = (const BRMerkleBlock *) BRSetGet(blockSet, &b->prevBlock);
		}
	}

	previous = (const BRMerkleBlock *) BRSetGet(blockSet, &block->prevBlock);
	return BRMerkleBlockVerifyDifficulty(block, previous, (b) ? b->timestamp : 0);
}

void TestConnectPeer::runPeerConnectTest_WalletFactory() {

	boost::scoped_ptr<MasterWalletManager> walletFactory(new MasterWalletManager("Data"));

	std::string mnemonic = "abandon abandon abandon abandon abandon abandon abandon abandon abandon abandon abandon about";
	std::string phrasePassword = "";
	std::string payPassword = "payPassword";
	IMasterWallet *masterWallet = walletFactory->ImportWalletWithMnemonic("ELA", mnemonic, phrasePassword, payPassword);

	ISubWallet *subWallet = masterWallet->CreateSubWallet("ELA", payPassword, false);
	nlohmann::json addresses = subWallet->GetAllAddress(0, INT_MAX);
	std::cout << "wallet addrs: " << addresses << std::endl;
	Log::getLogger()->info("wallet balance = {}", subWallet->GetBalance());

	sleep(4);

	bool hasSentTransaction = false;

	while (true) {
		sleep(10);
		uint64_t balance = subWallet->GetBalance();
		Log::getLogger()->info("wallet balance = {}", balance);

		if (balance > 1000000 && !hasSentTransaction) {
			try {
				nlohmann::json tx = subWallet->CreateTransaction("EZ3PoRzcr95ADMrDLCDb8DQAMRs7j8DkB2", "ERcEon7MC8fUBZSadvCUTVYmdHyRK1Jork", balance / 2, 100000, "");
				nlohmann::json result = subWallet->SendRawTransaction(tx, 100000, payPassword);
				Log::getLogger()->info("send tx result = {}", result.dump());
			} catch (std::exception e) {
				Log::getLogger()->error("send transaction from EZ3PoRzcr95ADMrDLCDb8DQAMRs7j8DkB2 to ERcEon7MC8fUBZSadvCUTVYmdHyRK1Jork error: {}", e.what());
			}
			hasSentTransaction = true;
		}
	}

	masterWallet->DestroyWallet(subWallet);
	walletFactory->DestroyWallet(masterWallet->GetId());
}
