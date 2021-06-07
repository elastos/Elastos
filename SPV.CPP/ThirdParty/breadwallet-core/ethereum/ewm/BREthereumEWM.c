//
//  BREthereumEWM
//  Core Ethereum
//
//  Created by Ed Gamble on 3/5/18.
//  Copyright © 2018-2019 Breadwinner AG.  All rights reserved.
//
//  See the LICENSE file at the project root for license information.
//  See the CONTRIBUTORS file at the project root for a list of contributors.

#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <stdarg.h>
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <errno.h>
#include <dirent.h>
#include <sys/stat.h>
#include "support/BRArray.h"
#include "support/BRBIP39Mnemonic.h"
#include "support/BRAssert.h"
#include "ethereum/event/BREvent.h"
#include "ethereum/event/BREventAlarm.h"
#include "BREthereumEWMPrivate.h"

#define EWM_SLEEP_SECONDS (10)

// When using a BRD sync, offset the start block by N days of Ethereum blocks
#define EWM_BRD_SYNC_START_BLOCK_OFFSET        (3 * 24 * 60 * 4)   /* 4 per minute (every 15 seconds) */


#define EWM_INITIAL_SET_SIZE_DEFAULT         (25)

/* Forward Declaration */
static void
ewmPeriodicDispatcher (BREventHandler handler,
                       BREventTimeout *event);

/* Forward Implementation */

/// MARK: - Ethereum Wallet Manager

static BREthereumEWM
ewmCreateErrorHandler (BREthereumEWM ewm, int fileService, const char* reason) {
    if (NULL != ewm) free (ewm);
    if (fileService)
        eth_log ("EWM", "on ewmCreate: FileService Error: %s", reason);
    else
        eth_log ("EWM", "on ewmCreate: Error: %s", reason);

    return NULL;
}

static void
ewmAssertRecovery (BREthereumEWM ewm);

extern BREthereumEWM
ewmCreate (BREthereumNetwork network,
           BREthereumAccount account,
           BREthereumTimestamp accountTimestamp,
           BRCryptoSyncMode mode,
           const char *storagePath,
           uint64_t blockHeight,
           uint64_t confirmationsUntilFinal) {
    BREthereumEWM ewm = (BREthereumEWM) calloc (1, sizeof (struct BREthereumEWMRecord));

    ewm->network = network;
    ewm->account = account;
    ewm->accountTimestamp = accountTimestamp;
    ewm->blockHeight = blockHeight;
    ewm->confirmationsUntilFinal = confirmationsUntilFinal;

    {
        char address [ADDRESS_ENCODED_CHARS];
        addressFillEncodedString (accountGetPrimaryAddress(account), 1, address);
        eth_log ("EWM", "Account: %s", address);
    }

    // Our one and only coder
    ewm->coder = rlpCoderCreate();

    // Create the EWM lock - do this early in case any `init` functions use it.
    {
        pthread_mutexattr_t attr;
        pthread_mutexattr_init(&attr);
        pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);

        pthread_mutex_init(&ewm->lock, &attr);
        pthread_mutexattr_destroy(&attr);
    }

    // Save the recovered tokens
    // TODO: hero
//    ewm->tokens = tokens;

    // Create the alarm clock, but don't start it.
    alarmClockCreateIfNecessary(0);

    array_new (ewm->wallets, DEFAULT_WALLET_CAPACITY);

    // Create a default ETH wallet; other wallets will be created 'on demand'.  This will signal
    // a WALLET_EVENT_CREATED event.
    ewm->walletHoldingEther = walletCreate (ewm->account, ewm->network);
    ewmInsertWallet (ewm, ewm->walletHoldingEther);

    BRAssertDefineRecovery ((BRAssertRecoveryInfo) ewm,
                            (BRAssertRecoveryHandler) ewmAssertRecovery);

    // mark as 'sync in progress' - we can't sent transactions until we have the nonce.
    return ewm;

}

extern BREthereumEWM
ewmCreateWithPaperKey (BREthereumNetwork network,
                       const char *paperKey,
                       BREthereumTimestamp accountTimestamp,
                       BRCryptoSyncMode mode,
                       const char *storagePath,
                       uint64_t blockHeight,
                       uint64_t confirmationsUntilFinal) {
    return ewmCreate (network,
                      createAccount (paperKey),
                      accountTimestamp,
                      mode,
                      storagePath,
                      blockHeight,
                      confirmationsUntilFinal);
}

extern BREthereumEWM
ewmCreateWithPublicKey (BREthereumNetwork network,
                        BRKey publicKey,
                        BREthereumTimestamp accountTimestamp,
                        BRCryptoSyncMode mode,
                        const char *storagePath,
                        uint64_t blockHeight,
                        uint64_t confirmationsUntilFinal) {
    return ewmCreate (network,
                      createAccountWithPublicKey(publicKey),
                      accountTimestamp,
                      mode,
                      storagePath,
                      blockHeight,
                      confirmationsUntilFinal);
}

extern void
ewmDestroy (BREthereumEWM ewm) {
    // Stop, including disconnect.  This WILL take `ewm->lock` and it MUST be available.
    ewmStop (ewm);

    pthread_mutex_lock(&ewm->lock);

    walletsRelease (ewm->wallets);
    ewm->wallets = NULL;

    BRSetFreeAll (ewm->tokens, (void (*) (void*)) tokenRelease);
    ewm->tokens = NULL;

    rlpCoderRelease(ewm->coder);

    // Finally remove the assert recovery handler
    BRAssertRemoveRecovery((BRAssertRecoveryInfo) ewm);

    pthread_mutex_unlock (&ewm->lock);
    pthread_mutex_destroy (&ewm->lock);

    memset (ewm, 0, sizeof(*ewm));
    free (ewm);
}

/// MARK: - Start/Stop

extern void
ewmStart (BREthereumEWM ewm) {
    // TODO: Check on a current state before starting.

    // Start the alarm clock.
    alarmClockStart(alarmClock);
}

extern void
ewmStop (BREthereumEWM ewm) {
    // TODO: Check on a current state before stopping.

    // TODO: Are their disconnect events that we need to process before stopping the handler?

    // Stop the alarm clock
    alarmClockStop (alarmClock);
}

/// MARK: - Connect / Disconnect
static void
ewmAssertRecovery (BREthereumEWM ewm) {
    eth_log ("EWM", "Recovery%s", "");
}

extern BREthereumNetwork
ewmGetNetwork (BREthereumEWM ewm) {
    return ewm->network; // constant
}

extern BREthereumAccount
ewmGetAccount (BREthereumEWM ewm) {
    return ewm->account; // constant
}

extern char *
ewmGetAccountPrimaryAddress(BREthereumEWM ewm) {
    return accountGetPrimaryAddressString(ewmGetAccount(ewm)); // constant
}

extern BRKey // key.pubKey
ewmGetAccountPrimaryAddressPublicKey(BREthereumEWM ewm) {
    return accountGetPrimaryAddressPublicKey(ewmGetAccount(ewm)); // constant
}

extern BRKey
ewmGetAccountPrimaryAddressPrivateKey(BREthereumEWM ewm,
                                           const char *paperKey) {
    return accountGetPrimaryAddressPrivateKey (ewmGetAccount(ewm), paperKey); // constant

}

/// MARK: - Sync

typedef struct {
    BREthereumEWM ewm;
    uint64_t begBlockNumber;
    uint64_t endBlockNumber;
} BREthereumSyncTransferContext;

static int
ewmSyncUpdateTransferPredicate (BREthereumSyncTransferContext *context,
                                BREthereumTransfer transfer,
                                unsigned int index) {
    uint64_t blockNumber = 0;

    // Do we return true for anything besides 'included' - like for 'error'.  For 'error' the
    // answer is 'no - because the blockchain has no information about non-included transactios
    // and logs'.  The other status types (created, submitted, etc) will either be resolved by
    // another sync or won't matter.

    return (transferExtractStatusIncluded (transfer, NULL, &blockNumber, NULL, NULL, NULL) &&
            context->begBlockNumber <= blockNumber && blockNumber <= context->endBlockNumber);
}

typedef struct {
    BREthereumEWM ewm;
    uint64_t transferBlockHeight;
    uint64_t confirmedBlockHeight;
} ewmSyncToDepthGetLastConfirmedSendTransferHeightContext;

static int
ewmSyncToDepthGetLastConfirmedSendTransferHeightPredicate (ewmSyncToDepthGetLastConfirmedSendTransferHeightContext *context,
                                                           BREthereumTransfer transfer,
                                                           unsigned int index) {
    BREthereumAccount account = ewmGetAccount (context->ewm);
    BREthereumAddress accountAddress = accountGetPrimaryAddress (account);

    BREthereumAddress source = transferGetSourceAddress (transfer);
    BREthereumAddress *target = transferGetTargetAddress (transfer);

    BREthereumBoolean accountIsSource = addressEqual (source, accountAddress);
    BREthereumBoolean accountIsTarget = NULL == target ? ETHEREUM_BOOLEAN_FALSE : addressEqual (*target, accountAddress);

    uint64_t blockNumber = 0;
    // check that the transfer has been included, is a send and has been confirmed as final
    return (transferExtractStatusIncluded (transfer, NULL, &blockNumber, NULL, NULL, NULL) &&
            accountIsSource == ETHEREUM_BOOLEAN_TRUE && accountIsTarget == ETHEREUM_BOOLEAN_FALSE &&
            blockNumber < (context->confirmedBlockHeight));
}

static void
ewmSyncToDepthGetLastConfirmedSendTransferHeightWalker (ewmSyncToDepthGetLastConfirmedSendTransferHeightContext *context,
                                                        BREthereumTransfer transfer,
                                                        unsigned int index) {
    uint64_t blockNumber = 0;
    transferExtractStatusIncluded (transfer, NULL, &blockNumber, NULL, NULL, NULL);
    context->transferBlockHeight = (blockNumber > context->transferBlockHeight ?
                                    blockNumber : context->transferBlockHeight);
    return;
}

/// MARK: - Mode

extern void
ewmWipe (BREthereumNetwork network,
         const char *storagePath) {
    fileServiceWipe (storagePath, "eth", networkGetName(network));
}

/// MARK: - Blocks

extern uint64_t
ewmGetBlockHeight(BREthereumEWM ewm) {
    pthread_mutex_lock(&ewm->lock);
    uint64_t height = ewm->blockHeight;
    pthread_mutex_unlock(&ewm->lock);
    return height;
}

/// MARK: - Transfers
/// MARK: - Wallets

extern void
ewmInsertWallet (BREthereumEWM ewm,
                 BREthereumWallet wallet) {
    pthread_mutex_lock(&ewm->lock);
    array_add (ewm->wallets, wallet);
    pthread_mutex_unlock(&ewm->lock);
}

extern BREthereumWallet *
ewmGetWallets (BREthereumEWM ewm) {
    pthread_mutex_lock(&ewm->lock);

    unsigned long count = array_count(ewm->wallets);
    BREthereumWallet *wallets = calloc (count + 1, sizeof (BREthereumWallet));

    for (size_t index = 0; index < count; index++) {
        wallets [index] = ewm->wallets[index];
    }
    wallets[count] = NULL;

    pthread_mutex_unlock(&ewm->lock);
    return wallets;
}

extern size_t
ewmGetWalletsCount (BREthereumEWM ewm) {
    pthread_mutex_lock(&ewm->lock);
    size_t count = array_count(ewm->wallets);
    pthread_mutex_unlock(&ewm->lock);
    return count;
}

extern BREthereumWallet
ewmGetWallet(BREthereumEWM ewm) {
    return ewm->walletHoldingEther; // constant
}

extern BREthereumWallet
ewmGetWalletHoldingToken(BREthereumEWM ewm,
                         BREthereumToken token) {
    BREthereumWallet wallet = NULL;

    pthread_mutex_lock(&ewm->lock);
    for (int i = 0; i < array_count(ewm->wallets); i++)
        if (token == walletGetToken(ewm->wallets[i])) {
            wallet = ewm->wallets[i];
            break;
        }

    if (NULL == wallet) {
        wallet = walletCreateHoldingToken(ewm->account,
                                          ewm->network,
                                          token);
        ewmInsertWallet(ewm, wallet);
    }
    pthread_mutex_unlock(&ewm->lock);
    return wallet;
}


extern BREthereumTransfer
ewmWalletCreateTransfer(BREthereumEWM ewm,
                        BREthereumWallet wallet,
                        const char *recvAddress,
                        BREthereumAmount amount,
                        uint64_t nonce) {
    BREthereumTransfer transfer = NULL;
	BREthereumAddress *addressPtr = NULL, brAddress;
	if (strlen(recvAddress) > 0) {
		brAddress = addressCreate(recvAddress);
		addressPtr = &brAddress;
	}

    pthread_mutex_lock(&ewm->lock);
    transfer = walletCreateTransfer(wallet, addressPtr, amount, nonce);
    pthread_mutex_unlock(&ewm->lock);

    return transfer;
}

extern BREthereumTransfer
ewmWalletCreateTransferGeneric(BREthereumEWM ewm,
                               BREthereumWallet wallet,
                               const char *recvAddress,
                               BREthereumEther amount,
                               BREthereumGasPrice gasPrice,
                               BREthereumGas gasLimit,
                               const char *data,
                               uint64_t nonce) {
    BREthereumTransfer transfer = NULL;

    BREthereumAddress *addressPtr = NULL, brAddress;
    if (strlen(recvAddress) > 0) {
		brAddress = addressCreate(recvAddress);
		addressPtr = &brAddress;
	}

    pthread_mutex_lock(&ewm->lock);
    transfer = walletCreateTransferGeneric(wallet,
                                              addressPtr,
                                              amount,
                                              gasPrice,
                                              gasLimit,
                                              data,
                                              nonce);
    pthread_mutex_unlock(&ewm->lock);

    return transfer;
}

extern BREthereumTransfer
ewmWalletCreateTransferWithFeeBasis (BREthereumEWM ewm,
                                     BREthereumWallet wallet,
                                     const char *recvAddress,
                                     BREthereumAmount amount,
                                     BREthereumFeeBasis feeBasis,
                                     uint64_t nonce) {
    BREthereumTransfer transfer = NULL;

	BREthereumAddress *addressPtr = NULL, brAddress;
	if (strlen(recvAddress) > 0) {
		brAddress = addressCreate(recvAddress);
		addressPtr = &brAddress;
	}

    pthread_mutex_lock(&ewm->lock);
    transfer = walletCreateTransferWithFeeBasis (wallet, addressPtr, amount, feeBasis, nonce);
    pthread_mutex_unlock(&ewm->lock);

    return transfer;
}

extern BREthereumEther
ewmWalletEstimateTransferFee(BREthereumEWM ewm,
                             BREthereumWallet wallet,
                             BREthereumAmount amount,
                             int *overflow) {
    pthread_mutex_lock(&ewm->lock);
    BREthereumEther fee = walletEstimateTransferFee(wallet, amount, overflow);
    pthread_mutex_unlock(&ewm->lock);
    return fee;
}

extern BREthereumEther
ewmWalletEstimateTransferFeeForBasis(BREthereumEWM ewm,
                                     BREthereumWallet wallet,
                                     BREthereumAmount amount,
                                     BREthereumGasPrice price,
                                     BREthereumGas gas,
                                     int *overflow) {
    pthread_mutex_lock(&ewm->lock);
    BREthereumEther fee = walletEstimateTransferFeeDetailed (wallet, amount, price, gas, overflow);
    pthread_mutex_unlock(&ewm->lock);
    return fee;
}

extern void
ewmWalletEstimateTransferFeeForTransfer (BREthereumEWM ewm,
                                         BREthereumWallet wallet,
                                         BREthereumCookie cookie,
                                         BREthereumAddress source,
                                         BREthereumAddress *target,
                                         BREthereumAmount amount,
                                         BREthereumGasPrice gasPrice,
                                         BREthereumGas gasLimit,
                                         uint64_t nonce) {
    BREthereumToken  ethToken  = ewmWalletGetToken (ewm, wallet);

    // use transfer, instead of transaction, due to the need to fill out the transaction data based on if
    // it is a token transfer or not
    BREthereumTransfer transfer = transferCreate (source,
                                                  target,
                                                  amount,
                                                  (BREthereumFeeBasis) {FEE_BASIS_GAS, {.gas = {gasLimit, gasPrice}}},
                                                  (NULL == ethToken ? TRANSFER_BASIS_TRANSACTION : TRANSFER_BASIS_LOG), nonce);

    transferRelease (transfer);
}

extern void // status, error
ewmWalletSignTransfer (BREthereumEWM ewm,
                       BREthereumWallet wallet,
                       BREthereumTransfer transfer,
                       BRKey privateKey) {
    pthread_mutex_lock(&ewm->lock);
    walletSignTransferWithPrivateKey (wallet, transfer, privateKey);
    pthread_mutex_unlock(&ewm->lock);
}

extern void // status, error
ewmWalletSignTransferWithPaperKey(BREthereumEWM ewm,
                                  BREthereumWallet wallet,
                                  BREthereumTransfer transfer,
                                  const char *paperKey) {
    pthread_mutex_lock(&ewm->lock);
    walletSignTransfer (wallet, transfer, paperKey);
    pthread_mutex_unlock(&ewm->lock);
}

extern BREthereumAddress
ewmWalletGetAddress (BREthereumEWM ewm,
                     BREthereumWallet wallet) {
    return walletGetAddress(wallet);
}

extern BREthereumBoolean
ewmWalletHasAddress (BREthereumEWM ewm,
                     BREthereumWallet wallet,
                     BREthereumAddress address) {
    return addressEqual(address, walletGetAddress(wallet));
}

extern BREthereumToken
ewmWalletGetToken (BREthereumEWM ewm,
                   BREthereumWallet wallet) {
    return walletGetToken(wallet); // constant
}


extern BREthereumGas
ewmWalletGetGasEstimate(BREthereumEWM ewm,
                        BREthereumWallet wallet,
                        BREthereumTransfer transfer) {
    pthread_mutex_lock(&ewm->lock);
    BREthereumGas gas = transferGetGasEstimate(transfer);
    pthread_mutex_unlock(&ewm->lock);
    return gas;

}

extern BREthereumGas
ewmWalletGetDefaultGasLimit(BREthereumEWM ewm,
                            BREthereumWallet wallet) {
    pthread_mutex_lock(&ewm->lock);
    BREthereumGas gas = walletGetDefaultGasLimit(wallet);
    pthread_mutex_unlock(&ewm->lock);
    return gas;
}

extern void
ewmWalletSetDefaultGasLimit(BREthereumEWM ewm,
                            BREthereumWallet wallet,
                            BREthereumGas gasLimit) {
    pthread_mutex_lock(&ewm->lock);
    walletSetDefaultGasLimit(wallet, gasLimit);
    pthread_mutex_unlock(&ewm->lock);
}

extern BREthereumGasPrice
ewmWalletGetDefaultGasPrice(BREthereumEWM ewm,
                            BREthereumWallet wallet) {
    pthread_mutex_lock(&ewm->lock);
    BREthereumGasPrice price = walletGetDefaultGasPrice(wallet);
    pthread_mutex_unlock(&ewm->lock);
    return price;
}

extern void
ewmWalletSetDefaultGasPrice(BREthereumEWM ewm,
                            BREthereumWallet wallet,
                            BREthereumGasPrice gasPrice) {
    pthread_mutex_lock(&ewm->lock);
    walletSetDefaultGasPrice(wallet, gasPrice);
    pthread_mutex_unlock(&ewm->lock);
}

#if defined (NEVER_DEFINED)
/**
 * Handle a `gasEstimate` for `transaction` in `wallet`
 *
 * @param ewm
 * @param wallet
 * @param transaction
 * @param gasEstimate
 */
extern void
ewmHandleGasEstimate (BREthereumEWM ewm,
                      BREthereumWallet wallet,
                      BREthereumTransfer transfer,
                      BREthereumGas gasEstimate) {
    transferSetGasEstimate(transfer, gasEstimate);

    ewmSignalTransferEvent(ewm,
                           wallet,
                           transfer,
                           (BREthereumTransferEvent) {
                               TRANSFER_EVENT_GAS_ESTIMATE_UPDATED,
                               SUCCESS
                           });
}
#endif
// ==============================================================================================
//
// LES(BCS)/BRD Handlers
//

extern void
ewmTransferFillRawData (BREthereumEWM ewm,
                        BREthereumWallet wallet,
                        BREthereumTransfer transfer,
                        uint8_t **bytesPtr, size_t *bytesCountPtr) {
    assert (NULL != bytesCountPtr && NULL != bytesPtr);

    pthread_mutex_lock (&ewm->lock);

    BREthereumTransaction transaction = transferGetOriginatingTransaction (transfer);
    assert (NULL != transaction);
    assert (ETHEREUM_BOOLEAN_IS_TRUE (transactionIsSigned(transaction)));
    pthread_mutex_unlock (&ewm->lock);

    BRRlpItem item = transactionRlpEncode(transaction,
                                          ewm->network,
                                          (transactionIsSigned(transaction)
                                           ? RLP_TYPE_TRANSACTION_SIGNED
                                           : RLP_TYPE_TRANSACTION_UNSIGNED),
                                          ewm->coder);
    BRRlpData data = rlpGetData (ewm->coder, item);

    *bytesCountPtr = data.bytesCount;
    *bytesPtr = data.bytes;

    rlpReleaseItem(ewm->coder, item);
}

extern const char *
ewmTransferGetRawDataHexEncoded(BREthereumEWM ewm,
                                BREthereumWallet wallet,
                                BREthereumTransfer transfer,
                                const char *prefix) {

    pthread_mutex_lock (&ewm->lock);
    BREthereumTransaction transaction = transferGetOriginatingTransaction (transfer);
    pthread_mutex_unlock (&ewm->lock);
    
    return (NULL == transaction ? NULL
            : transactionGetRlpHexEncoded (transaction,
                                           ewm->network,
                                           (ETHEREUM_BOOLEAN_IS_TRUE (transactionIsSigned(transaction))
                                            ? RLP_TYPE_TRANSACTION_SIGNED
                                            : RLP_TYPE_TRANSACTION_UNSIGNED),
                                           prefix));
}

/// MARK: - Transfer

extern BREthereumAddress*
ewmTransferGetTarget (BREthereumEWM ewm,
                      BREthereumTransfer transfer) {
    return transferGetTargetAddress(transfer);
}

extern BREthereumAddress
ewmTransferGetSource (BREthereumEWM ewm,
                      BREthereumTransfer transfer) {
    return transferGetSourceAddress(transfer);
}

extern BREthereumHash
ewmTransferGetIdentifier(BREthereumEWM ewm,
                         BREthereumTransfer transfer) {
    pthread_mutex_lock (&ewm->lock);
    BREthereumHash identifier = transferGetIdentifier (transfer);
    pthread_mutex_unlock (&ewm->lock);
    return identifier;
}

extern BREthereumHash
ewmTransferGetOriginatingTransactionHash(BREthereumEWM ewm,
                                         BREthereumTransfer transfer) {
    pthread_mutex_lock (&ewm->lock);
    BREthereumHash hash = transferGetOriginatingTransactionHash(transfer);
    pthread_mutex_unlock (&ewm->lock);
    return hash;
}

extern char *
ewmTransferGetAmountEther(BREthereumEWM ewm,
                          BREthereumTransfer transfer,
                          BREthereumEtherUnit unit) {
    BREthereumAmount amount = transferGetAmount(transfer);
    return (AMOUNT_ETHER == amountGetType(amount)
            ? etherGetValueString(amountGetEther(amount), unit)
            : "");
}

extern char *
ewmTransferGetAmountTokenQuantity(BREthereumEWM ewm,
                                  BREthereumTransfer transfer,
                                  BREthereumTokenQuantityUnit unit) {
    BREthereumAmount amount = transferGetAmount(transfer);
    return (AMOUNT_TOKEN == amountGetType(amount)
            ? tokenQuantityGetValueString(amountGetTokenQuantity(amount), unit)
            : "");
}

extern BREthereumAmount
ewmTransferGetAmount(BREthereumEWM ewm,
                     BREthereumTransfer transfer) {
    return transferGetAmount(transfer);
}

extern BREthereumGasPrice
ewmTransferGetGasPrice(BREthereumEWM ewm,
                       BREthereumTransfer transfer,
                       BREthereumEtherUnit unit) {
    return feeBasisGetGasPrice (transferGetFeeBasis(transfer));
}

extern BREthereumGas
ewmTransferGetGasLimit(BREthereumEWM ewm,
                       BREthereumTransfer transfer) {
    return feeBasisGetGasLimit(transferGetFeeBasis(transfer));
}

extern BREthereumFeeBasis
ewmTransferGetFeeBasis (BREthereumEWM ewm,
                        BREthereumTransfer transfer) {
    return transferGetFeeBasis (transfer);
}

extern uint64_t
ewmTransferGetNonce(BREthereumEWM ewm,
                    BREthereumTransfer transfer) {
    pthread_mutex_lock (&ewm->lock);
    uint64_t nonce = transferGetNonce(transfer);
    pthread_mutex_unlock (&ewm->lock);
    return nonce;
}

extern BREthereumBoolean
ewmTransferExtractStatusIncluded (BREthereumEWM ewm,
                                  BREthereumTransfer transfer,
                                  BREthereumHash *blockHash,
                                  uint64_t *blockNumber,
                                  uint64_t *blockTransactionIndex,
                                  uint64_t *blockTimestamp,
                                  BREthereumGas *gasUsed) {
    pthread_mutex_lock (&ewm->lock);
    int included = transferExtractStatusIncluded (transfer,
                                                  blockHash,
                                                  blockNumber,
                                                  blockTransactionIndex,
                                                  blockTimestamp,
                                                  gasUsed);
    pthread_mutex_unlock (&ewm->lock);

    return AS_ETHEREUM_BOOLEAN (included);
}

extern BREthereumHash
ewmTransferGetBlockHash(BREthereumEWM ewm,
                        BREthereumTransfer transfer) {
    BREthereumHash blockHash;
    return (transferExtractStatusIncluded(transfer, &blockHash, NULL, NULL, NULL, NULL)
            ? blockHash
            : hashCreateEmpty());
}

extern uint64_t
ewmTransferGetBlockNumber(BREthereumEWM ewm,
                          BREthereumTransfer transfer) {
    uint64_t blockNumber;
    return (transferExtractStatusIncluded(transfer, NULL, &blockNumber, NULL, NULL, NULL)
            ? blockNumber
            : 0);
}

extern uint64_t
ewmTransferGetTransactionIndex(BREthereumEWM ewm,
                               BREthereumTransfer transfer) {
    uint64_t transactionIndex;
    return (transferExtractStatusIncluded(transfer, NULL, NULL, &transactionIndex, NULL, NULL)
            ? transactionIndex
            : 0);
}


extern uint64_t
ewmTransferGetBlockTimestamp (BREthereumEWM ewm,
                              BREthereumTransfer transfer) {
    uint64_t blockTimestamp;
    return (transferExtractStatusIncluded(transfer, NULL, NULL, NULL, &blockTimestamp, NULL)
            ? blockTimestamp
            : TRANSACTION_STATUS_BLOCK_TIMESTAMP_UNKNOWN);
}

extern BREthereumGas
ewmTransferGetGasUsed(BREthereumEWM ewm,
                      BREthereumTransfer transfer) {
    BREthereumGas gasUsed;
    return (transferExtractStatusIncluded(transfer, NULL, NULL, NULL, NULL, &gasUsed)
            ? gasUsed
            : gasCreate(0));
}

extern uint64_t
ewmTransferGetBlockConfirmations(BREthereumEWM ewm,
                                 BREthereumTransfer transfer) {
    uint64_t blockNumber = 0;
    if (!transferExtractStatusIncluded(transfer, NULL, &blockNumber, NULL, NULL, NULL))
		return 0;

     return (ewmGetBlockHeight(ewm) >= blockNumber) ? (ewmGetBlockHeight(ewm) - blockNumber) : 0;
}

extern BREthereumTransferStatus
ewmTransferGetStatus (BREthereumEWM ewm,
                      BREthereumTransfer transfer) {
    return transferGetStatus (transfer);
}

extern BREthereumBoolean
ewmTransferIsConfirmed(BREthereumEWM ewm,
                       BREthereumTransfer transfer) {
    return transferHasStatus (transfer, TRANSFER_STATUS_INCLUDED);
}

extern BREthereumBoolean
ewmTransferIsSubmitted(BREthereumEWM ewm,
                       BREthereumTransfer transfer) {
    return AS_ETHEREUM_BOOLEAN (ETHEREUM_BOOLEAN_IS_TRUE (transferHasStatus (transfer, TRANSFER_STATUS_SUBMITTED)) ||
                                ETHEREUM_BOOLEAN_IS_TRUE (transferHasStatusOrTwo (transfer,
                                                                                  TRANSFER_STATUS_INCLUDED,
                                                                                  TRANSFER_STATUS_ERRORED)));
}

extern char *
ewmTransferStatusGetError (BREthereumEWM ewm,
                           BREthereumTransfer transfer) {
    char *reason = NULL;

    pthread_mutex_lock (&ewm->lock);
    if (TRANSFER_STATUS_ERRORED == transferGetStatus(transfer))
        transferExtractStatusError (transfer, &reason);
    pthread_mutex_unlock (&ewm->lock);

    return reason;
}

extern int
ewmTransferStatusGetErrorType (BREthereumEWM ewm,
                               BREthereumTransfer transfer) {
    BREthereumTransactionErrorType type = (BREthereumTransactionErrorType) -1;

    pthread_mutex_lock (&ewm->lock);
    transferExtractStatusErrorType (transfer, &type);
    pthread_mutex_unlock (&ewm->lock);

    return type;
}

extern BREthereumBoolean
ewmTransferHoldsToken(BREthereumEWM ewm,
                      BREthereumTransfer transfer,
                      BREthereumToken token) {
    assert (NULL != transfer);
    return (token == transferGetToken(transfer)
            ? ETHEREUM_BOOLEAN_TRUE
            : ETHEREUM_BOOLEAN_FALSE);
}

extern BREthereumToken
ewmTransferGetToken(BREthereumEWM ewm,
                    BREthereumTransfer transfer) {
    assert (NULL !=  transfer);
    return transferGetToken(transfer);
}

extern BREthereumEther
ewmTransferGetFee(BREthereumEWM ewm,
                  BREthereumTransfer transfer,
                  int *overflow) {
    assert (NULL != transfer);

    pthread_mutex_lock (&ewm->lock);
    BREthereumEther fee = transferGetFee(transfer, overflow);
    pthread_mutex_unlock (&ewm->lock);

    return fee;
}

/// MARK: - Amount

extern BREthereumAmount
ewmCreateEtherAmountString(BREthereumEWM ewm,
                           const char *number,
                           BREthereumEtherUnit unit,
                           BRCoreParseStatus *status) {
    return amountCreateEther (etherCreateString(number, unit, status));
}

extern BREthereumAmount
ewmCreateEtherAmountUnit(BREthereumEWM ewm,
                         uint64_t amountInUnit,
                         BREthereumEtherUnit unit) {
    return amountCreateEther (etherCreateNumber(amountInUnit, unit));
}

extern BREthereumAmount
ewmCreateTokenAmountString(BREthereumEWM ewm,
                           BREthereumToken token,
                           const char *number,
                           BREthereumTokenQuantityUnit unit,
                           BRCoreParseStatus *status) {
    return amountCreateTokenQuantityString(token, number, unit, status);
}

extern char *
ewmCoerceEtherAmountToString(BREthereumEWM ewm,
                             BREthereumEther ether,
                             BREthereumEtherUnit unit) {
    return etherGetValueString(ether, unit);
}

extern char *
ewmCoerceTokenAmountToString(BREthereumEWM ewm,
                             BREthereumTokenQuantity token,
                             BREthereumTokenQuantityUnit unit) {
    return tokenQuantityGetValueString(token, unit);
}

/// MARK: - Gas Price / Limit

extern BREthereumGasPrice
ewmCreateGasPrice (uint64_t value,
                   BREthereumEtherUnit unit) {
    return gasPriceCreate(etherCreateNumber(value, unit));
}

extern BREthereumGas
ewmCreateGas (uint64_t value) {
    return gasCreate(value);
}

extern BREthereumToken
ewmLookupToken (BREthereumEWM ewm,
                BREthereumAddress address) {
    pthread_mutex_lock (&ewm->lock);
    BREthereumToken token = (BREthereumToken) BRSetGet (ewm->tokens, &address);
    pthread_mutex_unlock (&ewm->lock);
    return token;
}

extern BREthereumToken
ewmCreateToken (BREthereumEWM ewm,
                const char *address,
                const char *symbol,
                const char *name,
                const char *description,
                int decimals,
                BREthereumGas defaultGasLimit,
                BREthereumGasPrice defaultGasPrice) {
    if (NULL == address || 0 == strlen(address)) return NULL;
    if (ETHEREUM_BOOLEAN_FALSE == addressValidateString(address)) return NULL;

    // This function is called in potentially two threads.  One in EWM event handler (on
    // `ewmHandleAnnounceToken()`) and one in `cryptoWalletManagerInstall...()` (on some App
    // listener thread).  Such a description, used here, is troubling in and of itself.

    BREthereumAddress addr = addressCreate(address);

    // Lock over BRSetGet(), BRSetAdd() and tokenUpdate()
    pthread_mutex_lock (&ewm->lock);
    BREthereumToken token = (BREthereumToken) BRSetGet (ewm->tokens, &addr);
    if (NULL == token) {
        token = tokenCreate (address,
                             symbol,
                             name,
                             description,
                             decimals,
                             defaultGasLimit,
                             defaultGasPrice);
        BRSetAdd (ewm->tokens, token);
    }
    else {
        tokenUpdate (token,
                     symbol,
                     name,
                     description,
                     decimals,
                     defaultGasLimit,
                     defaultGasPrice);
    }
    pthread_mutex_unlock (&ewm->lock);

    return token;
}
