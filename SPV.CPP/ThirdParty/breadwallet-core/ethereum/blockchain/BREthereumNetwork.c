//
//  BREthereumNetwork
//  breadwallet-core Ethereum
//
//  Created by Ed Gamble on 3/13/18.
//  Copyright © 2018 Breadwinner AG.  All rights reserved.
//
//  See the LICENSE file at the project root for license information.
//  See the CONTRIBUTORS file at the project root for a list of contributors.

#include <stdlib.h>
#include "BREthereumNetwork.h"

#define NUMBER_OF_SEEDS_LIMIT       (5)
#define NUMBER_OF_ENODES_LIMIT      (10)

static void
networkInitilizeAllIfAppropriate (void);

//
// Network
//
struct BREthereumNetworkRecord {
    const char *name;
    int chainId;
    int networkId;
    BREthereumHash genesisBlockHeaderHash;
    BREthereumHash trustedCheckpointBlockHeaderHash;
    const char *seeds[NUMBER_OF_SEEDS_LIMIT + 1];
    const char *enodesBRD[NUMBER_OF_ENODES_LIMIT + 1];
    const char *enodesCOM[NUMBER_OF_ENODES_LIMIT + 1];

    const char *enodesLCLParity[4];
    const char *enodesLCLGeth[4];
};

extern BREthereumChainId
networkGetChainId (BREthereumNetwork network) {
	networkInitilizeAllIfAppropriate();
	return network->chainId;
}

extern BREthereumNetworkId
networkGetNetworkId (BREthereumNetwork network) {
    networkInitilizeAllIfAppropriate();
    return network->networkId;
}

extern BREthereumHash
networkGetGenesisBlockHeaderHash (BREthereumNetwork network) {
    networkInitilizeAllIfAppropriate();
    return network->genesisBlockHeaderHash;
}

extern BREthereumHash
networkGetTrustedCheckpointBlockHeaderHash (BREthereumNetwork network) {
    networkInitilizeAllIfAppropriate();
    return network->trustedCheckpointBlockHeaderHash;
}

extern const char *
networkGetName (BREthereumNetwork network) {
    return network->name;
}

extern const char**
networkGetSeeds (BREthereumNetwork network) {
    return network->seeds;
}

extern size_t
networkGetSeedsCount (BREthereumNetwork network) {
    size_t i = 0;
    while (NULL != network->seeds[i]) i++;
    return i;
}

extern const char**
networkGetEnodesBRD (BREthereumNetwork network) {
    return network->enodesBRD;
}

extern const char**
networkGetEnodesCommunity (BREthereumNetwork network) {
    return network->enodesCOM;
}

extern const char**
networkGetEnodesLocal (BREthereumNetwork network, int parity) {
    return parity ?  network->enodesLCLParity : network->enodesLCLGeth;
}

/// MARK: - Static Network Definitions

//
// Mainnet
//
static struct BREthereumNetworkRecord ethereumMainnetRecord = {
    "Mainnet",
    0,
    1,
    EMPTY_HASH_INIT,
    EMPTY_HASH_INIT,
    // Seeds
    { //"seed.mainnet.eth.brd.breadwallet.com",
        //"seed.mainnet.eth.community.breadwallet.com",
        NULL },

    // Enodes

    // BRD
    { 	"enode://dee112e94b17b3b49366e5dec78d7e8a1ee342ff363b490819d40a55482046d333b2bd51b3d1ce250078c5315bf302758d13f63ac94fd8e43f6e54be8412c316@52.74.28.202:20630",
		 "enode://152fae4134f4db49d24905762ade694fc86e0a24124c0927c9c1cbc816bb9929e790d4fba236c7a55c9d9817df72c1d23353c2dccc3796bd397d72320a722ef1@52.62.113.83:20630",
		 "enode://dbfdb62b5cf4cb5a12ee1df68bfb4c0626ad5335ec5ee0c594c315b08a61e7f0bc8ce5b264136eec0db17db1e55f1bb0f1de67f9bb9c57bea77feef74f2baa2c@35.156.51.127:20630",
		 "enode://a1a37849c8a0d5247870fc2d70da053fdae503b99498daf63905728bc801a57818577a88b02895763a5af8037ab5378b3ea12eb01ec2546712cf5ebaab3e94c6@35.177.89.244:20630",
		 "enode://9b43d046414b722ec3e6237083ab413280555cf4ee5765c9a4c55c9fc86c979c7389840a35377846cf0c6923afa7fa3cd115f6d8152e1f8df24273466cb15cf1@52.53.134.102:20630",
        NULL },

    // Community
    { NULL },

    // Local - Parity
    { NULL },

    // Local - Geth
    { NULL }
};
const BREthereumNetwork ethereumMainnet = &ethereumMainnetRecord;

/*
// MainnetChainConfig is the chain parameters to run a node on the main network.
MainnetChainConfig = &ChainConfig{
  ChainId:        big.NewInt(1),
  HomesteadBlock: big.NewInt(1150000),
  DAOForkBlock:   big.NewInt(1920000),
  DAOForkSupport: true,
  EIP150Block:    big.NewInt(2463000),
  EIP150Hash:     common.HexToHash("0x2086799aeebeae135c246c65021c82b4e15a2c451340993aacfd2751886514f0"),
  EIP155Block:    big.NewInt(2675000),
  EIP158Block:    big.NewInt(2675000),
  ByzantiumBlock: big.NewInt(4370000),
  Ethash: new(EthashConfig),
}
*/

//
// Testnet
//
static struct BREthereumNetworkRecord ethereumTestnetRecord = {
    "Testnet",
    3,
	3,
    EMPTY_HASH_INIT,
    EMPTY_HASH_INIT,
    // Seeds
    {   //"seed.ropsten.eth.brd.breadwallet.com",
        //"seed.ropsten.eth.community.breadwallet.com",
        NULL },

    // Enodes

    // BRD
    {   "enode://5e1d6f9f74e33b2d1e2fda87efaf60a788b338c08eefd3a435e9c7de98645bc041421c27d9ed3927c7b5195febd691aff30de881842749f3030089df0e135232@3.208.184.54:20630",
		"enode://30dc2b7986e2ec5902498ec26fad6fcecece617aa1652f227f684ede6a0939bb7a205ada1c91420d30b427c86bbdcc31fdfd6d955dd8f5854370f583025a0708@3.209.35.13:20630",
		"enode://b0357d45e9070c1660f63f077e0e3b0054a18d93785589d498586b6e0b7ec7c5b39ef608e82e7280ca95019db7c36455275d98a3e8684916ba8f3a7aab4ad38b@3.210.227.193:20630",
        NULL },
    { NULL },
    { NULL },
    { NULL }
};
const BREthereumNetwork ethereumTestnet = &ethereumTestnetRecord;

/*
// TestnetChainConfig contains the chain parameters to run a node on the Ropsten test network.
TestnetChainConfig = &ChainConfig{
  ChainId:        big.NewInt(3),
  HomesteadBlock: big.NewInt(0),
  DAOForkBlock:   nil,
  DAOForkSupport: true,
  EIP150Block:    big.NewInt(0),
  EIP150Hash:     common.HexToHash("0x41941023680923e0fe4d74a34bdac8141f2540e3ae90623718e47d66d1ca4a2d"),
  EIP155Block:    big.NewInt(10),
  EIP158Block:    big.NewInt(10),
  ByzantiumBlock: big.NewInt(1700000),
  Ethash: new(EthashConfig),
}
*/
//
// Rinkeby
//
static struct BREthereumNetworkRecord ethereumRinkebyRecord = {
    "Rinkeby",
    4,
    4,
    EMPTY_HASH_INIT,
    EMPTY_HASH_INIT,
    // Seeds
    { NULL },

    // Enodes
    
    { "enode://fe44bc423f210805daad60cc5d308f449e9282c28a9aba91040d7c727cf5751d1ae9e85d32a430f4a6fe15c8eb52833a1747e8b28e6ed5ae291fdae32e6b9181@18.217.15.245:20630",
	  "enode://777e2a86687d675c05344acc6e24cefbd3e233759e8b89d7b3d101aeffc89e6292f66a115c5bfc30f250c120e6a2354a7a6ea304439cfded706de1c9ade61abf@18.217.15.245:20630",
	  "enode://deb84117dada6c2c8f9c5d9d44f749b6fbbefdc987a1611b683ead6e4e2ce8e0d05a196591a713376eee5d9c165d3888d2e175e8eb842e5a381f273c0268edca@18.217.15.245:20630",
	  NULL },
    { NULL },
    { NULL },
    { NULL }
};
const BREthereumNetwork ethereumRinkeby = &ethereumRinkebyRecord;

/*
// RinkebyChainConfig contains the chain parameters to run a node on the Rinkeby test network.
RinkebyChainConfig = &ChainConfig{
  ChainId:        big.NewInt(4),
  HomesteadBlock: big.NewInt(1),
  DAOForkBlock:   nil,
  DAOForkSupport: true,
  EIP150Block:    big.NewInt(2),
  EIP150Hash:     common.HexToHash("0x9b095b36c15eaf13044373aef8ee0bd3a382a5abb92e402afa44b8249c3a90e9"),
  EIP155Block:    big.NewInt(3),
  EIP158Block:    big.NewInt(3),
  ByzantiumBlock: big.NewInt(1035301),
  Clique: &CliqueConfig{
    Period: 15,
    Epoch:  30000,
  },
}
*/

/// MARK: - Trusted Checkpoints

/*
// trustedCheckpoint represents a set of post-processed trie roots (CHT and BloomTrie) associated with
// the appropriate section index and head hash. It is used to start light syncing from this checkpoint
// and avoid downloading the entire header chain while still being able to securely access old headers/logs.
type trustedCheckpoint struct {
    name                                string
    sectionIdx                          uint64
    sectionHead, chtRoot, bloomTrieRoot common.Hash
}

var (
     mainnetCheckpoint = trustedCheckpoint{
     name:          "mainnet",
     sectionIdx:    153,
     sectionHead:   common.HexToHash("04c2114a8cbe49ba5c37a03cc4b4b8d3adfc0bd2c78e0e726405dd84afca1d63"),
     chtRoot:       common.HexToHash("d7ec603e5d30b567a6e894ee7704e4603232f206d3e5a589794cec0c57bf318e"),
     bloomTrieRoot: common.HexToHash("0b139b8fb692e21f663ff200da287192201c28ef5813c1ac6ba02a0a4799eef9"),
     }

     ropstenCheckpoint = trustedCheckpoint{
     name:          "ropsten",
     sectionIdx:    79,
     sectionHead:   common.HexToHash("1b1ba890510e06411fdee9bb64ca7705c56a1a4ce3559ddb34b3680c526cb419"),
     chtRoot:       common.HexToHash("71d60207af74e5a22a3e1cfbfc89f9944f91b49aa980c86fba94d568369eaf44"),
     bloomTrieRoot: common.HexToHash("70aca4b3b6d08dde8704c95cedb1420394453c1aec390947751e69ff8c436360"),
     }
     )

// trustedCheckpoints associates each known checkpoint with the genesis hash of the chain it belongs to
var trustedCheckpoints = map[common.Hash]trustedCheckpoint{
    params.MainnetGenesisHash: mainnetCheckpoint,
    params.TestnetGenesisHash: ropstenCheckpoint,
}

 // Rinkeby: genesis for all intents and purposes.
 // > INFO [06-06|11:34:07] Block synchronisation started
 // INFO [06-06|11:34:08] Imported new block headers               count=192 elapsed=76.267ms number=192 hash=8c570c…ba360c ignored=0

*/
static void
networkInitilizeAllIfAppropriate (void) {
    static int needsInitialization = 1;

    if (needsInitialization) {

        // Mainnet

        ethereumMainnetRecord.genesisBlockHeaderHash =
        hashCreate ("0xd4e56740f876aef8c010b86a40d5f56745a118d0906a34e69aec8c0db1cb8fa3");

        ethereumMainnetRecord.trustedCheckpointBlockHeaderHash =
        hashCreate("0x04c2114a8cbe49ba5c37a03cc4b4b8d3adfc0bd2c78e0e726405dd84afca1d63");

        // Testnet / 'Ropsten'

        ethereumTestnetRecord.genesisBlockHeaderHash =
        hashCreate("0x41941023680923e0fe4d74a34bdac8141f2540e3ae90623718e47d66d1ca4a2d");

        ethereumTestnetRecord.trustedCheckpointBlockHeaderHash =
        hashCreate("0x1b1ba890510e06411fdee9bb64ca7705c56a1a4ce3559ddb34b3680c526cb419");

        // Rinkeby

        ethereumRinkebyRecord.genesisBlockHeaderHash =
        hashCreate("0x6341fd3daf94b748c72ced5a5b26028f2474f5f00d824504e4fa37a75767e177");
        
        ethereumRinkebyRecord.trustedCheckpointBlockHeaderHash =
        hashCreate("0x6341fd3daf94b748c72ced5a5b26028f2474f5f00d824504e4fa37a75767e177");

        // Notable RACE
        needsInitialization = 0;

    }
}
