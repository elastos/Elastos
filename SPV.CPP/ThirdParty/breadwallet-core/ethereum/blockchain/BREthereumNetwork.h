//
//  BREthereumNetwork
//  Core Ethereum
//
//  Created by Ed Gamble on 3/13/18.
//  Copyright © 2018-2019 Breadwinner AG.  All rights reserved.
//
//  See the LICENSE file at the project root for license information.
//  See the CONTRIBUTORS file at the project root for a list of contributors.

#ifndef BR_Ethereum_Network_h
#define BR_Ethereum_Network_h

#include "support/BRInt.h"
#include "ethereum/base/BREthereumHash.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * A Ethereum Network is the whole of an Ethereum P2P network represented with a `chainId` and,
 * incidentally, a name.
 *
 * An ethereum network also holds seeds, enodes and hashes that are needed when connecting to
 * a peer.  Specifially each 'seed' is a host with a DNS 'txt' record where by each txt record
 * specified an enode.  (An enode is a triple of {public key, ip address, port}).
 *
 * Three static network are created - one each of: 'mainnet/foundation', 'testnet/ropsten' and
 * 'rinkeby'
 */
typedef struct BREthereumNetworkRecord *BREthereumNetwork;

typedef int BREthereumChainId;  // 'Officially' UInt256
typedef int BREthereumNetworkId;

extern const char *
networkGetName (BREthereumNetwork network);

extern char *
networkCopyNameAsLowercase (BREthereumNetwork network);

extern BREthereumChainId
networkGetChainId (BREthereumNetwork network);

extern BREthereumChainId
networkGetChainIdOld(BREthereumNetwork network);

extern BREthereumNetworkId
networkGetNetworkId (BREthereumNetwork network);

extern void InsertEthereumNetwork(const char *name, int chainId, int networkId);

extern BREthereumNetwork FindEthereumNetwork(const char *name);

#ifdef __cplusplus
}
#endif

#endif // BR_Ethereum_Network_h
