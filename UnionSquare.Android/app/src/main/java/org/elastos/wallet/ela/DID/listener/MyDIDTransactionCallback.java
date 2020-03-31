package org.elastos.wallet.ela.DID.listener;

import org.elastos.did.DIDAdapter;

public interface MyDIDTransactionCallback {
    public void createIdTransaction(String payload, String memo,
                                    int confirms, DIDAdapter.TransactionCallback callback);
}
