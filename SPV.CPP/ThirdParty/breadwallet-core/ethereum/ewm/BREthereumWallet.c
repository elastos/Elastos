//
//  BBREthereumWallet.c
//  Core Ethereum
//
//  Created by Ed Gamble on 2/21/2018.
//  Copyright © 2018-2019 Breadwinner AG.  All rights reserved.
//
//  See the LICENSE file at the project root for license information.
//  See the CONTRIBUTORS file at the project root for a list of contributors.

#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "support/BRArray.h"
#include "BREthereumWallet.h"
#include "BREthereumTransfer.h"

#define DEFAULT_ETHER_GAS_PRICE_NUMBER   500000000 // 0.5 GWEI
#define DEFAULT_ETHER_GAS_PRICE_UNIT     WEI

#define DEFAULT_TRANSFER_CAPACITY     20

/* Forward Declarations */
static BREthereumGasPrice
walletCreateDefaultGasPrice (BREthereumWallet wallet);

static BREthereumGas
walletCreateDefaultGasLimit (BREthereumWallet wallet, BREthereumAmount amount);

static int // -1 if not found
walletLookupTransferIndex (BREthereumWallet wallet,
                           BREthereumTransfer transfer);

//
// Wallet
//
struct BREthereumWalletRecord {
    
    /**
     * The wallet's account.  The account is used to sign transfers.
     */
    BREthereumAccount account;
    
    /**
     * The wallet's primary address - perhaps the sole address.  Must be an address
     * from the wallet's account.
     */
    BREthereumAddress address;      // Primary Address
    
    /**
     * The wallet's network.
     */
    BREthereumNetwork network;
    
    /**
     * The wallet' default gasPrice. gasPrice is the maximum price of gas you are willing to pay
     * for a transfer of this wallet's holding type.  This default value can be 'overridden'
     * when creating a specific transfer.
     *
     * The gasPrice determines how 'interested' a miner is in 'blocking' a transfer.  Thus,
     * the gasPrice determines how quickly your transfer will be added to the block chain.
     */
    BREthereumGasPrice defaultGasPrice;
    
    /**
     * The wallet's default gasLimit. gasLimit is the maximum gas you are willing to pay for
     * a transfer of this wallet's holding type.  This default value can be 'overridden'
     * when creating a specific transfer.
     *
     * The gasLimit prevents your transfer's computation from 'spinning out of control' and
     * thus consuming unexpectedly large amounts of Ether.
     */
    BREthereumGas defaultGasLimit;

    /**
     * An optional ERC20 token specification.  Will be NULL (and unused) for holding ETHER.
     */
    BREthereumToken token; // optional
};

//
// Wallet Creation
//
static BREthereumWallet
walletCreateDetailed (BREthereumAccount account,
                      BREthereumAddress address,
                      BREthereumNetwork network,
                      BREthereumAmountType type,
                      BREthereumToken optionalToken) {
    
    assert (NULL != account);
    //    assert (NULL != address);
    assert (AMOUNT_TOKEN != type || NULL != optionalToken);
    
    BREthereumWallet wallet = calloc(1, sizeof(struct BREthereumWalletRecord));
    
    wallet->account = account;
    wallet->address = address;
    wallet->network = network;
    
    wallet->token = optionalToken;
    BREthereumAmount balance = (AMOUNT_ETHER == type
                       ? amountCreateEther(etherCreate(UINT256_ZERO))
                       : amountCreateToken(createTokenQuantity (wallet->token, UINT256_ZERO)));
    
    wallet->defaultGasLimit = AMOUNT_ETHER == type
    ? walletCreateDefaultGasLimit(wallet, balance)
    : tokenGetGasLimit (optionalToken);
    
    wallet->defaultGasPrice = AMOUNT_ETHER == type
    ? walletCreateDefaultGasPrice(wallet)
    : tokenGetGasPrice (optionalToken);
    
    return wallet;
}

extern BREthereumWallet
walletCreate(BREthereumAccount account,
             BREthereumNetwork network)
{
    return walletCreateDetailed (account,
                                 accountGetPrimaryAddress(account),
                                 network,
                                 AMOUNT_ETHER,
                                 NULL);
}

extern BREthereumWallet
walletCreateHoldingToken(BREthereumAccount account,
                         BREthereumNetwork network,
                         BREthereumToken token) {
    return walletCreateDetailed (account,
                                 accountGetPrimaryAddress(account),
                                 network,
                                 AMOUNT_TOKEN,
                                 token);
}

extern void
walletRelease (BREthereumWallet wallet) {
    free (wallet);
}

extern void
walletsRelease (OwnershipGiven BRArrayOf(BREthereumWallet) wallets) {
    if (NULL != wallets) {
        size_t count = array_count(wallets);
        for (size_t index = 0; index < count; index++)
            walletRelease(wallets[index]);
        array_free(wallets);
    }
}

//
// Transfer
//
extern BREthereumEther
walletEstimateTransferFee (BREthereumWallet wallet,
                           BREthereumAmount amount,
                           int *overflow) {
    return walletEstimateTransferFeeDetailed (wallet,
                                              amount,
                                              wallet->defaultGasPrice,
                                              amountGetGasEstimate(amount),
                                              overflow);
}

/**
 * Estimate the transfer fee (in Ether) for transferring amount.
 */
extern BREthereumEther
walletEstimateTransferFeeDetailed (BREthereumWallet wallet,
                                   BREthereumAmount amount,
                                   BREthereumGasPrice price,
                                   BREthereumGas gas,
                                   int *overflow) {
    return etherCreate (mulUInt256_Overflow (price.etherPerGas.valueInWEI,
                                             createUInt256(gas.amountOfGas),
                                             overflow));
}

//
// Transfer Creation
//
extern BREthereumTransfer
walletCreateTransferWithFeeBasis (BREthereumWallet wallet,
                                  BREthereumAddress *recvAddress,
                                  BREthereumAmount amount,
                                  BREthereumFeeBasis feeBasis,
                                  uint64_t nonce) {
    BREthereumTransfer transfer = transferCreate (wallet->address, recvAddress, amount, feeBasis,
                                                  (NULL == wallet->token
                                                   ? TRANSFER_BASIS_TRANSACTION
                                                   : TRANSFER_BASIS_LOG), nonce);
    return transfer;
}

extern BREthereumTransfer
walletCreateTransfer(BREthereumWallet wallet,
                     BREthereumAddress *recvAddress,
                     BREthereumAmount amount,
                     uint64_t nonce) {
    
    return walletCreateTransferWithFeeBasis (wallet, recvAddress, amount,
                                             (BREthereumFeeBasis) {
                                                 FEE_BASIS_GAS,
                                                 { .gas = {
                                                     wallet->defaultGasLimit,
                                                     wallet->defaultGasPrice
                                                 }}},
                                                 nonce);
}

extern BREthereumTransfer
walletCreateTransferGeneric (BREthereumWallet wallet,
                             BREthereumAddress *recvAddress,
                             BREthereumEther amount,
                             BREthereumGasPrice gasPrice,
                             BREthereumGas gasLimit,
                             const char *data,
                             uint64_t nonce) {

    BREthereumTransaction originatingTransaction = transactionCreate (walletGetAddress(wallet),
                                                                      recvAddress,
                                                                      amount,
                                                                      gasPrice,
                                                                      gasLimit,
                                                                      data,
                                                                      nonce);

    BREthereumTransfer transfer =
    transferCreateWithTransactionOriginating (originatingTransaction,
                                              (NULL == walletGetToken(wallet)
                                               ? TRANSFER_BASIS_TRANSACTION
                                               : TRANSFER_BASIS_LOG));

    return transfer;
}

//
// Transfer Signing and Encoding
//

/**
 * Sign the transfer with a paper key.
 *
 * @param wallet
 * @param transfer
 * @param paperKey
 */
extern void
walletSignTransfer (BREthereumWallet wallet,
                    BREthereumTransfer transfer,
                    const char *paperKey) {
    transferSign (transfer,
                  wallet->network,
                  wallet->account,
                  wallet->address,
                  paperKey);
}

/**
 * Sign the transfer with a private key
 *
 * @param wallet
 * @param transfer
 * @param privateKey
 */
extern void
walletSignTransferWithPrivateKey (BREthereumWallet wallet,
                                  BREthereumTransfer transfer,
                                  BRKey privateKey) {
    transferSignWithKey (transfer,
                         wallet->network,
                         wallet->account,
                         wallet->address,
                         privateKey);
}

//
// Wallet 'Field' Accessors
//

extern BREthereumAddress
walletGetAddress(BREthereumWallet wallet) {
    return wallet->address;
}

extern BREthereumToken
walletGetToken (BREthereumWallet wallet) {
    return wallet->token;
}
// Gas Limit

extern BREthereumGas
walletGetDefaultGasLimit(BREthereumWallet wallet) {
    return wallet->defaultGasLimit;
}

extern void
walletSetDefaultGasLimit(BREthereumWallet wallet,
                         BREthereumGas gasLimit) {
    wallet->defaultGasLimit = gasLimit;
}

static BREthereumGas
walletCreateDefaultGasLimit (BREthereumWallet wallet, BREthereumAmount amount) {
    return amountGetGasEstimate(amount);
}

// Gas Price

extern BREthereumGasPrice
walletGetDefaultGasPrice(BREthereumWallet wallet) {
    return wallet->defaultGasPrice;
}

extern void
walletSetDefaultGasPrice(BREthereumWallet wallet,
                         BREthereumGasPrice gasPrice) {
    wallet->defaultGasPrice = gasPrice;
}

static BREthereumGasPrice
walletCreateDefaultGasPrice (BREthereumWallet wallet) {
    if (NULL == walletGetToken(wallet)) {
        return gasPriceCreate(etherCreateNumber
                                      (DEFAULT_ETHER_GAS_PRICE_NUMBER,
                                       DEFAULT_ETHER_GAS_PRICE_UNIT));
    } else {
        return tokenGetGasPrice (wallet->token);
    }
}

//
// Transfer 'Observation'
//

extern int
transferPredicateAny (void *ignore,
                      BREthereumTransfer transfer,
                      unsigned int index) {
    return 1;
}

extern int
transferPredicateStatus (BREthereumTransferStatus status,
                         BREthereumTransfer transfer,
                         unsigned int index) {
    return transferHasStatus(transfer, status);
}


//
// Transfer State
//
#if 0
private_extern void
walletTransferSubmitted (BREthereumWallet wallet,
                            BREthereumTransfer transfer,
                            const BREthereumHash hash) {
    transferSetStatus(transfer, transactionStatusCreate (TRANSFER_STATUS_SUBMITTED));
    // balance updated?
}

private_extern void
walletTransferIncluded(BREthereumWallet wallet,
                          BREthereumTransfer transfer,
                          BREthereumGas gasUsed,
                          BREthereumHash blockHash,
                          uint64_t blockNumber,
                          uint64_t blockTransferIndex) {
    transferSetStatus(transfer,
                         transactionStatusCreateIncluded(gasUsed,
                                                         blockHash,
                                                         blockNumber,
                                                         blockTransferIndex));
    walletUpdateTransferSorted(wallet, transfer);
}

private_extern void
walletTransferErrored (BREthereumWallet wallet,
                          BREthereumTransfer transfer,
                          const char *reason) {
    transferSetStatus(transfer,
                         transactionStatusCreateErrored(reason));
}
#endif // 0

/// MARK: - Wallet State

struct BREthereumWalletStateRecord {
    // This is a token address or, if Ether, FAKE_ETHER_ADDRESS_INIT
    BREthereumAddress address;

    // Normally we would save the wallet's balance as `BREthereumAmount` but that concept includes
    // the `BREthereumToken` if the amount is 'token based'.  In the context of WalletState we
    // do not have tokens available (tokens are held as `BRSet` in `BREthereumEWM`, not globally).
    // So, we'll save the balance as `UInt256` and then when recovering the wallet use this
    // `amount` with the above `address` to create a proper balance.
    UInt256 amount;

    // Include the account nonce if the this wallet state is for ETHER.  Hackily.
    uint64_t nonce;
};

#define FAKE_ETHER_ADDRESS_INIT   ((const BREthereumAddress) { \
0xff, 0xff, 0xff, 0xff,   \
0xff, 0xff, 0xff, 0xff,   \
0xff, 0xff, 0xff, 0xff,   \
0xff, 0xff, 0xff, 0xff,   \
0xff, 0xff, 0xff, 0xff \
})

extern void
walletStateRelease (BREthereumWalletState state) {
    free (state);
}

extern BREthereumAddress
walletStateGetAddress (const BREthereumWalletState walletState) {
    return (ETHEREUM_BOOLEAN_IS_TRUE (addressEqual (FAKE_ETHER_ADDRESS_INIT, walletState->address))
            ? EMPTY_ADDRESS_INIT
            : walletState->address);
}

extern UInt256
walletStateGetAmount (const BREthereumWalletState walletState) {
    return walletState->amount;
}

extern uint64_t
walletStateGetNonce (const BREthereumWalletState walletState) {
    return walletState->nonce;
}

extern void
walletStateSetNonce (BREthereumWalletState walletState,
                     uint64_t nonce) {
    walletState->nonce = nonce;
}

extern BRRlpItem
walletStateEncode (const BREthereumWalletState state,
                   BRRlpCoder coder) {
    return rlpEncodeList (coder, 3,
                          addressRlpEncode (&state->address, coder),
                          rlpEncodeUInt256 (coder, state->amount, 0),
                          rlpEncodeUInt64  (coder, state->nonce, 0));
}

extern BREthereumWalletState
walletStateDecode (BRRlpItem item,
                   BRRlpCoder coder) {
    BREthereumWalletState state = malloc (sizeof (struct BREthereumWalletStateRecord));

    size_t itemsCount = 0;
    const BRRlpItem *items = rlpDecodeList (coder, item, &itemsCount);
    assert (3 == itemsCount);

    BREthereumAddress *addr = addressRlpDecode (items[0], coder);
    if (NULL != addr) {
        state->address = *addr;
        free(addr);
    } else {
        state->address = EMPTY_ADDRESS_INIT;
    }
    state->amount  = rlpDecodeUInt256 (coder, items[1], 0);
    state->nonce   = rlpDecodeUInt64  (coder, items[2], 0);

    return state;
}

extern BREthereumHash
walletStateGetHash (const BREthereumWalletState state) {
    return addressGetHash (state->address);
}

static inline size_t
walletStateHashValue (const void *t)
{
    return addressHashValue(((BREthereumWalletState) t)->address);
}

static inline int
walletStateHashEqual (const void *t1, const void *t2) {
    return t1 == t2 || addressHashEqual (((BREthereumWalletState) t1)->address,
                                         ((BREthereumWalletState) t2)->address);
}

extern BRSetOf(BREthereumWalletState)
walletStateSetCreate (size_t capacity) {
    return BRSetNew (walletStateHashValue, walletStateHashEqual, capacity);
}

/*
 * https://medium.com/blockchain-musings/how-to-create-raw-transfers-in-ethereum-part-1-1df91abdba7c
 *
 *
 
 // Private key
 const keythereum = require('keythereum');
 const address = '0x9e378d2365b7657ebb0f72ae402bc08812022211';
 const datadir = '/home/administrator/ethereum/data';
 const password = 'password';
 let   privKey; // a 'buffer'
 
 keythereum.importFromFile(address, datadir,
 function (keyObject) {
 keythereum.recover(password, keyObject,
 function (privateKey) {
 console.log(privateKey.toString('hex'));
 privKey = privateKey
 });
 });
 //05a20149c1c76ae9da8457435bf0224a4f81801da1d8204cb81608abe8c112ca
 
 const ethTx = require('ethereumjs-tx');
 
 const txParams = {
 nonce: '0x6', // Replace by nonce for your account on geth node
 gasPrice: '0x09184e72a000',
 gasLimit: '0x30000',
 to: '0xfa3caabc8eefec2b5e2895e5afbf79379e7268a7',
 value: '0x00'
 };
 
 // Transfer is created
 const tx = new ethTx(txParams);
 const privKey = Buffer.from('05a20149c1c76ae9da8457435bf0224a4f81801da1d8204cb81608abe8c112ca', 'hex');
 
 // Transfer is signed
 tx.sign(privKey);
 
 const serializedTx = tx.serialize();
 const rawTx = '0x' + serializedTx.toString('hex');
 console.log(rawTx)
 
 eth.sendRawTransfer(raxTX)
 
 
 */


/*
 *
 * https://ethereum.stackexchange.com/questions/16472/signing-a-raw-transfer-in-go
 
 signer := types.NewEIP155Signer(nil)
 tx := types.NewTransfer(nonce, to, amount, gas, gasPrice, data)
 signature, _ := crypto.Sign(tx.SigHash(signer).Bytes(), privkey)
 signed_tx, _ := tx.WithSignature(signer, signature)
 
 */

/*
 *
 
 web3.eth.accounts.create();
 > {
 address: "0xb8CE9ab6943e0eCED004cDe8e3bBed6568B2Fa01",
 privateKey: "0x348ce564d427a3311b6536bbcff9390d69395b06ed6c486954e971d960fe8709",
 walletSignTransfer: function(tx){...},
 sign: function(data){...},
 encrypt: function(password){...}
 }
 
 */
