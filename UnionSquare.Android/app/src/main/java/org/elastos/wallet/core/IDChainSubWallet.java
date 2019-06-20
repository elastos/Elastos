// Copyright (c) 2012-2019 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

package org.elastos.wallet.core;

/**
 * IDChainSubWallet jni
 */
public class IDChainSubWallet extends SidechainSubWallet {
    private long mInstance;

    public String CreateIdTransaction(
            String fromAddress, String payloadJson, String programJson, String memo)
            throws WalletException {
        return CreateIdTransaction(mInstance, fromAddress, payloadJson, programJson, memo);
    }

    public IDChainSubWallet(long instance) {
        super(instance);
        mInstance = instance;
    }

    private native String CreateIdTransaction(
            long instance, String fromAddress, String payloadJson, String programJson, String memo);
}
