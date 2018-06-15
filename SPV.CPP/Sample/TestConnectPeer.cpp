

#include <climits>
#include <iostream>
#include <boost/scoped_ptr.hpp>
#include <Interface/Enviroment.h>

#include "BRChainParams.h"

#include "Wrapper/ChainParams.h"
#include "MasterWalletManager.h"

#include "TestConnectPeer.h"

using namespace Elastos::SDK;

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
	Enviroment::InitializeRootPath("Data");
	boost::scoped_ptr<MasterWalletManager> walletFactory(new MasterWalletManager);

	std::string mnemonic = "abandon abandon abandon abandon abandon abandon abandon abandon abandon abandon abandon about";
	std::string phrasePassword = "";
	std::string payPassword = "payPassword";
	IMasterWallet *masterWallet = walletFactory->ImportWalletWithMnemonic("MasterWalletId", mnemonic, phrasePassword, payPassword);

	ISubWallet *subWallet = masterWallet->CreateSubWallet("ELA", payPassword, false);
	nlohmann::json addresses = subWallet->GetAllAddress(0, INT_MAX);
	std::cout << "wallet addrs: " << addresses << std::endl;

	while (true) sleep(1);

	masterWallet->DestroyWallet(subWallet);
	walletFactory->DestroyWallet(masterWallet->GetId());
}
