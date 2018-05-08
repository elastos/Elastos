
#include "Wrapper/ChainParams.h"
#include "BRChainParams.h"

#include "TestConnectPeer.h"
#include "TestWalletManager.h"

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

void TestConnectPeer::runPeerConnectTest() {

	TestWalletManager *wallet = new TestWalletManager();
	wallet->start();

	WalletPtr pwallet = wallet->getWallet();
	std::vector<std::string> addresses = pwallet->getAllAddresses();
	for (size_t i = 0; i < addresses.size(); ++i) {
		std::cout << "wallet addr: " << addresses[i] << std::endl;
	}


	wallet->testSendTransaction();

	while (BRPeerManagerPeerCount(wallet->getPeerManager()->getRaw()) > 0) sleep(1);
	//process end
}
