//
//  BREthereumNetwork
//  Core Ethereum
//
//  Created by Ed Gamble on 3/13/18.
//  Copyright © 2018-2019 Breadwinner AG.  All rights reserved.
//
//  See the LICENSE file at the project root for license information.
//  See the CONTRIBUTORS file at the project root for a list of contributors.

#include <ctype.h>
#include <stdlib.h>
#include "BREthereumNetwork.h"

#define NUMBER_OF_SEEDS_LIMIT       (5)
#define NUMBER_OF_ENODES_LIMIT      (10)

//
// Network
//
struct BREthereumNetworkRecord {
    const char *name;
    int chainId;
    int networkId;
};

extern BREthereumChainId
networkGetChainId (BREthereumNetwork network) {
    return network->chainId;
}

extern BREthereumChainId
networkGetChainIdOld(BREthereumNetwork network) {
	if (!strcmp(network->name, "mainnet")) {
		return 1;
	} else if (!strcmp(network->name, "testnet")) {
		return 3;
	}

	return 0;
}

extern BREthereumNetworkId
networkGetNetworkId (BREthereumNetwork network) {
    return network->networkId;
}

extern const char *
networkGetName (BREthereumNetwork network) {
    return network->name;
}

extern char *
networkCopyNameAsLowercase (BREthereumNetwork network) {
    char *networkName = strdup (network-> name);
    size_t networkNameLength = strlen (networkName);

    for (size_t index = 0; index < networkNameLength; index++)
        networkName[index] = tolower (networkName[index]);

    return networkName;
}

/// MARK: - Static Network Definitions

static struct BREthereumNetworkRecord *ethereumNetworks = NULL;

extern void InsertEthereumNetwork(const char *name, int chainId, int networkId) {
    if (ethereumNetworks == NULL)
        array_new(ethereumNetworks, 10);

    struct BREthereumNetworkRecord network;
    network.name = strdup(name);
    network.chainId = chainId;
    network.networkId = networkId;

    array_add(ethereumNetworks, network);
}

extern BREthereumNetwork FindEthereumNetwork(const char *name) {
    for (int i = 0; i < array_count(ethereumNetworks); ++i)
        if (0 == strcmp(name, ethereumNetworks[i].name))
            return &ethereumNetworks[i];

    return NULL;
}

#if 0
//
// Mainnet
//
static struct BREthereumNetworkRecord ethereumMainnetRecord = {
    "mainnet",
    20,
    20
};
const BREthereumNetwork ethereumMainnet = &ethereumMainnetRecord;

//
// Testnet
//
static struct BREthereumNetworkRecord ethereumTestnetRecord = {
    "testnet", // aka "ropsten"
    21,
    21
};
const BREthereumNetwork ethereumTestnet = &ethereumTestnetRecord;

//
// Rinkeby
//
static struct BREthereumNetworkRecord ethereumRinkebyRecord = {
    "rinkeby",
    4,
    4
};
const BREthereumNetwork ethereumRinkeby = &ethereumRinkebyRecord;

//
// Prvnet
//
static struct BREthereumNetworkRecord ethereumPrvnetRecord = {
        "prvnet", // aka "ropsten"
        21,
        21
};
const BREthereumNetwork ethereumPrvnet = &ethereumPrvnetRecord;

//
// DID Mainnet
//
static struct BREthereumNetworkRecord ethereumDIDMainnetRecord = {
        "mainnet-did",
        20,
        20
};
const BREthereumNetwork ethereumDIDMainnet = &ethereumDIDMainnetRecord;

//
// DID Testnet
//
static struct BREthereumNetworkRecord ethereumDIDTestnetRecord = {
        "testnet-did", // aka "ropsten"
        21,
        21
};
const BREthereumNetwork ethereumDIDTestnet = &ethereumDIDTestnetRecord;

//
// DID Rinkeby
//
static struct BREthereumNetworkRecord ethereumDIDRinkebyRecord = {
        "rinkeby-did",
        4,
        4
};
const BREthereumNetwork ethereumDIDRinkeby = &ethereumDIDRinkebyRecord;

//
// DID Prvnet
//
static struct BREthereumNetworkRecord ethereumDIDPrvnetRecord = {
        "prvnet-did", // aka "ropsten"
        1,
        20
};
const BREthereumNetwork ethereumDIDPrvnet = &ethereumDIDPrvnetRecord;

//
// HECO Mainnet
//
static struct BREthereumNetworkRecord ethereumHecoMainnetRecord = {
        "mainnet-heco",
        0x80,
        0x80
};
const BREthereumNetwork ethereumHecoMainnet = &ethereumHecoMainnetRecord;

//
// HECO Testnet
//
static struct BREthereumNetworkRecord ethereumHecoTestnetRecord = {
        "testnet-heco", // aka "ropsten"
        256,
        256
};
const BREthereumNetwork ethereumHecoTestnet = &ethereumHecoTestnetRecord;

//
// HECO Rinkeby
//
static struct BREthereumNetworkRecord ethereumHecoRinkebyRecord = {
        "rinkeby-heco",
        255,
        255
};
const BREthereumNetwork ethereumHecoRinkeby = &ethereumHecoRinkebyRecord;

//
// HECO Prvnet
//
static struct BREthereumNetworkRecord ethereumHecoPrvnetRecord = {
        "prvnet-heco", // aka "ropsten"
        255,
        255
};
const BREthereumNetwork ethereumHecoPrvnet = &ethereumHecoPrvnetRecord;

#endif