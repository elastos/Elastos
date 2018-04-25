
#include "Wrapper/ChainParams.h"
#include "BRChainParams.h"

#include "TestConnectPeer.h"
#include "TestWallet.h"

using namespace Elastos::SDK;

static int BRMainNetVerifyDifficulty(const BRMerkleBlock *block, const BRSet *blockSet)
{
	const BRMerkleBlock *previous, *b = NULL;
	uint32_t i;

	assert(block != NULL);
	assert(blockSet != NULL);

	// check if we hit a difficulty transition, and find previous transition block
	if ((block->height % BLOCK_DIFFICULTY_INTERVAL) == 0) {
		for (i = 0, b = block; b && i < BLOCK_DIFFICULTY_INTERVAL; i++) {
			b = (const BRMerkleBlock *)BRSetGet(blockSet, &b->prevBlock);
		}
	}

	previous = (const BRMerkleBlock *)BRSetGet(blockSet, &block->prevBlock);
	return BRMerkleBlockVerifyDifficulty(block, previous, (b) ? b->timestamp : 0);
}

void TestConnectPeer::runPeerConnectTest() {

	static const BRCheckPoint BRMainNetCheckpoints[] = {
		{      0, uint256("000000000019d6689c085ae165831e934ff763ae46a2a6c172b3f1b60a8ce26f"), 1231006505, 0x1d00ffff },
		{  20160, uint256("000000000f1aef56190aee63d33a373e6487132d522ff4cd98ccfc96566d461e"), 1248481816, 0x1d00ffff },
		{  40320, uint256("0000000045861e169b5a961b7034f8de9e98022e7a39100dde3ae3ea240d7245"), 1266191579, 0x1c654657 },
		{  60480, uint256("000000000632e22ce73ed38f46d5b408ff1cff2cc9e10daaf437dfd655153837"), 1276298786, 0x1c0eba64 },
		{  80640, uint256("0000000000307c80b87edf9f6a0697e2f01db67e518c8a4d6065d1d859a3a659"), 1284861847, 0x1b4766ed },
		{ 100800, uint256("000000000000e383d43cc471c64a9a4a46794026989ef4ff9611d5acb704e47a"), 1294031411, 0x1b0404cb },
		{ 120960, uint256("0000000000002c920cf7e4406b969ae9c807b5c4f271f490ca3de1b0770836fc"), 1304131980, 0x1b0098fa },
		{ 141120, uint256("00000000000002d214e1af085eda0a780a8446698ab5c0128b6392e189886114"), 1313451894, 0x1a094a86 },
		{ 161280, uint256("00000000000005911fe26209de7ff510a8306475b75ceffd434b68dc31943b99"), 1326047176, 0x1a0d69d7 },
		{ 181440, uint256("00000000000000e527fc19df0992d58c12b98ef5a17544696bbba67812ef0e64"), 1337883029, 0x1a0a8b5f },
		{ 201600, uint256("00000000000003a5e28bef30ad31f1f9be706e91ae9dda54179a95c9f9cd9ad0"), 1349226660, 0x1a057e08 },
		{ 221760, uint256("00000000000000fc85dd77ea5ed6020f9e333589392560b40908d3264bd1f401"), 1361148470, 0x1a04985c },
		{ 241920, uint256("00000000000000b79f259ad14635739aaf0cc48875874b6aeecc7308267b50fa"), 1371418654, 0x1a00de15 },
		{ 262080, uint256("000000000000000aa77be1c33deac6b8d3b7b0757d02ce72fffddc768235d0e2"), 1381070552, 0x1916b0ca },
		{ 282240, uint256("0000000000000000ef9ee7529607286669763763e0c46acfdefd8a2306de5ca8"), 1390570126, 0x1901f52c },
		{ 302400, uint256("0000000000000000472132c4daaf358acaf461ff1c3e96577a74e5ebf91bb170"), 1400928750, 0x18692842 },
		{ 322560, uint256("000000000000000002df2dd9d4fe0578392e519610e341dd09025469f101cfa1"), 1411680080, 0x181fb893 },
		{ 342720, uint256("00000000000000000f9cfece8494800d3dcbf9583232825da640c8703bcd27e7"), 1423496415, 0x1818bb87 },
		{ 362880, uint256("000000000000000014898b8e6538392702ffb9450f904c80ebf9d82b519a77d5"), 1435475246, 0x1816418e },
		{ 383040, uint256("00000000000000000a974fa1a3f84055ad5ef0b2f96328bc96310ce83da801c9"), 1447236692, 0x1810b289 },
		{ 403200, uint256("000000000000000000c4272a5c68b4f55e5af734e88ceab09abf73e9ac3b6d01"), 1458292068, 0x1806a4c3 },
		{ 423360, uint256("000000000000000001630546cde8482cc183708f076a5e4d6f51cd24518e8f85"), 1470163842, 0x18057228 },
		{ 443520, uint256("00000000000000000345d0c7890b2c81ab5139c6e83400e5bed00d23a1f8d239"), 1481765313, 0x18038b85 },
		{ 463680, uint256("000000000000000000431a2f4619afe62357cd16589b638bb638f2992058d88e"), 1493259601, 0x18021b3e },
		{ 483840, uint256("0000000000000000008e5d72027ef42ca050a0776b7184c96d0d4b300fa5da9e"), 1504704195, 0x1801310b },
		{ 504000, uint256("0000000000000000006cd44d7a940c79f94c7c272d159ba19feb15891aa1ea54"), 1515827554, 0x177e578c }
	};

	static const char *BRMainNetDNSSeeds[] = {
		"seed.breadwallet.com.",
		"seed.bitcoin.sipa.be.",
		"dnsseed.bluematt.me.",
		"dnsseed.bitcoin.dashjr.org.",
		"seed.bitcoinstats.com.",
		"bitseed.xf2.org.",
		"seed.bitcoin.jonasschnelli.ch.",
		NULL
	};

	const BRChainParams BRMainNetParams = {
		BRMainNetDNSSeeds,
		10866,       // standardPort
		7630401,     // magicNumber
		0,           // services
		BRMainNetVerifyDifficulty,
		BRMainNetCheckpoints,
		sizeof(BRMainNetCheckpoints) / sizeof(*BRMainNetCheckpoints)
	};
	ChainParams cp = ChainParams(BRMainNetParams);

	std::string phrase = "a test seed ha";
	MasterPubKeyPtr pk = MasterPubKeyPtr(new MasterPubKey(phrase));

	TestWallet * wallet = new TestWallet(pk, cp, time(nullptr) );
	wallet->getWallet();
	PeerManagerPtr pm = wallet->getPeerManager();
	pm->connect();

	while (BRPeerManagerPeerCount(pm->getRaw()) > 0) sleep(1);
	//process end
}
