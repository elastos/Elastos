//
//  BREthereumTransactionReceipt.c
//  BRCore
//
//  Created by Ed Gamble on 5/10/18.
//  Copyright © 2018-2019 Breadwinner AG.  All rights reserved.
//
//  See the LICENSE file at the project root for license information.
//  See the CONTRIBUTORS file at the project root for a list of contributors.

#include <string.h>
#include <assert.h>
#include "support/BRArray.h"
#include "BREthereumLog.h"
#include "BREthereumTransactionReceipt.h"

//
// Transaction Receipt
//
struct BREthereumTransactionReceiptRecord {
    /**
     * the cumulative gas used in the block containing the transaction receipt as of
     * immediately after the transaction has happened, Ru,
     */
    uint64_t gasUsed;

    /**
     * the set of logs created through execution of the transaction, Rl
     */
    BREthereumLog *logs;

    /**
     * and the status code of the transaction, Rz
     */
    BRRlpData stateRoot;
};

extern uint64_t
transactionReceiptGetGasUsed (BREthereumTransactionReceipt receipt) {
    return receipt->gasUsed;
}

extern size_t
transactionReceiptGetLogsCount (BREthereumTransactionReceipt receipt) {
    return array_count(receipt->logs);
}

extern BREthereumLog
transactionReceiptGetLog (BREthereumTransactionReceipt receipt, size_t index) {
    return (index < array_count(receipt->logs)
            ? receipt->logs[index]
            : NULL);
}

extern void
transactionReceiptRelease (BREthereumTransactionReceipt receipt) {
    if (NULL != receipt) {
        for (size_t index = 0; index < array_count(receipt->logs); index++)
            logRelease(receipt->logs[index]);
        array_free(receipt->logs);
        rlpDataRelease(receipt->stateRoot);
        free (receipt);
    }
}

//
// Transaction Receipt Logs - RLP Encode/Decode
//
static BRRlpItem
transactionReceiptLogsRlpEncode (BREthereumTransactionReceipt log,
                                 BRRlpCoder coder) {
    size_t itemsCount = array_count(log->logs);
    BRRlpItem items[itemsCount];
    
    for (int i = 0; i < itemsCount; i++)
        items[i] = logRlpEncode(log->logs[i], RLP_TYPE_NETWORK, coder);
    
    return rlpEncodeListItems(coder, items, itemsCount);
}

static BREthereumLog *
transactionReceiptLogsRlpDecode (BRRlpItem item,
                                 BRRlpCoder coder) {
    size_t itemsCount = 0;
    const BRRlpItem *items = rlpDecodeList(coder, item, &itemsCount);

    BREthereumLog *logs;
    array_new(logs, itemsCount);

    for (int i = 0; i < itemsCount; i++) {
        BREthereumLog log = logRlpDecode(items[i], RLP_TYPE_NETWORK, coder);
        array_add(logs, log);
    }

    return logs;
}

extern void
transactionReceiptsRelease (BRArrayOf(BREthereumTransactionReceipt) receipts) {
    if (NULL != receipts) {
        size_t count = array_count(receipts);
        for (size_t index = 0; index < count; index++)
            transactionReceiptRelease (receipts[index]);
        array_free (receipts);
    }
}
