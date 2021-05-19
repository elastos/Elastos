// Copyright (c) 2012-2019 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

package org.elastos.wallet.core;

/**
 * ISidechainSubWallet jni
 */
public class SidechainSubWallet extends SubWallet {
    private long mInstance;

    public String CreateWithdrawTransaction(String fromAddress, String amount, String mainChainAddress, String memo) throws WalletException {
        return CreateWithdrawTransaction(mInstance, fromAddress, amount, mainChainAddress, memo);
    }

    public String GetGenesisAddress() throws WalletException {
        return GetGenesisAddress(mInstance);
    }

    public SidechainSubWallet(long proxy) {
        super(proxy);
        mInstance = proxy;
    }

    private native String CreateWithdrawTransaction(long proxy, String fromAddress, String amount, String mainChainAddress, String memo);

    private native String GetGenesisAddress(long proxy);
}
