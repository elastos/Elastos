//
//  BREthereumEWMPrivate.h
//  BRCore
//
//  Created by Ed Gamble on 5/7/18.
//  Copyright © 2018-2019 Breadwinner AG.  All rights reserved.
//
//  See the LICENSE file at the project root for license information.
//  See the CONTRIBUTORS file at the project root for a list of contributors.

#ifndef BR_Ethereum_EWM_Private_H
#define BR_Ethereum_EWM_Private_H

#include <stdint.h>
#include <pthread.h>
#include "BREthereumTransfer.h"
#include "BREthereumWallet.h"
#include "BREthereumEWM.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    CLIENT_CHANGE_ADD,
    CLIENT_CHANGE_REM,
    CLIENT_CHANGE_UPD
} BREthereumClientChangeType;


#define DEFAULT_LISTENER_CAPACITY 3
#define DEFAULT_WALLET_CAPACITY 10
#define DEFAULT_BLOCK_CAPACITY 100
#define DEFAULT_TRANSACTION_CAPACITY 1000

/// MISPLACED
extern void
ewmInsertWallet (BREthereumEWM ewm,
                 BREthereumWallet wallet);

//
// EWM
//
struct BREthereumEWMRecord {

    /**
     * The network
     */
    BREthereumNetwork network;

    /**
     * The account
     */
    BREthereumAccount account;
    BREthereumTimestamp accountTimestamp;

    /**
     * The wallets 'managed/handled' by this ewm.  There can be only one wallet holding ETHER;
     * all the other wallets hold TOKENs and only one wallet per TOKEN.
     */
    BREthereumWallet *wallets;
    BREthereumWallet  walletHoldingEther;

    /**
     * ERC20 Tokens
     */
    BRSetOf(BREthereumToken) tokens;


    /**
     * The BlockHeight is the largest block number seen or computed.  [Note: the blockHeight may
     * be computed from a Log event as (log block number + log confirmations).  This is the block
     * number for the block at the head of the blockchain.
     *
     * This gets initialized from ewmCreate() based on the 'known' block height of the network or
     * with zero if the network's block height is unknown.
     */
    uint64_t blockHeight;

    /**
     * The number of blocks required to be mined before until a transfer can be considered final
     */
    uint64_t confirmationsUntilFinal;

    /**
     * The Lock ensuring single thread access to EWM state.
     */
    pthread_mutex_t lock;

    /**
     * The RLP Coder
     */
    BRRlpCoder coder;
};

//
// Block Event
//
#if defined (NEVER_DEFINED)
extern void
ewmSignalBlockEvent(BREthereumEWM ewm,
                          BREthereumBlock bid,
                          BREthereumBlockEvent event);

extern void
ewmHandleBlockEvent(BREthereumEWM ewm,
                          BREthereumBlock bid,
                          BREthereumBlockEvent event);
#endif

/// MARK: - Handler For Main



#ifdef __cplusplus
}
#endif

#endif //BR_Ethereum_EWM_Private_H
