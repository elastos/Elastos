// Copyright (c) 2012-2019 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

package org.elastos.wallet.core;

/**
 * TokenchainSubWallet jni
 */
public class TokenchainSubWallet extends SidechainSubWallet {
    private long mInstance;

    public String GetBalanceInfo(String assetID) throws WalletException {
        return GetBalanceInfo(mInstance, assetID);
    }

    public String GetBalance(String assetID) throws WalletException {
        return GetBalance(mInstance, assetID);
    }

    public String GetBalanceWithAddress(String assetID, String address) throws WalletException {
        return GetBalanceWithAddress(mInstance, assetID, address);
    }

    public String CreateRegisterAssetTransaction(String name, String descript, String registerToAddr,
                                                 String registerAmount, byte precision, String memo) {
        return CreateRegisterAssetTransaction(mInstance, name, descript, registerToAddr, registerAmount,
                precision, memo);
    }

    public String CreateTransaction(String fromAddr, String toAddr, String amount, String assetID,
                                    String memo) {
        return CreateTransaction(mInstance, fromAddr, toAddr, amount, assetID, memo);
    }

    public String CreateConsolidateTransaction(String assetID, String memo) {
        return CreateConsolidateTransaction(mInstance, assetID, memo);
    }

    public String GetAllAssets() {
        return GetAllAssets(mInstance);
    }

    public TokenchainSubWallet(long instance) {
        super(instance);
        mInstance = instance;
    }

    private native String GetBalanceInfo(long instance, String assetID);

    private native String GetBalance(long instance, String assetID);

    private native String GetBalanceWithAddress(long instance, String assetID, String address);

    private native String CreateRegisterAssetTransaction(long instance, String name, String descript,
                                                         String registerToAddr, String registerAmount,
                                                         byte precision, String memo);

    private native String CreateTransaction(long instance, String fromAddr, String toAddr,
                                            String amount, String assetID, String memo);

    private native String CreateConsolidateTransaction(long instance, String assetID, String memo);

    private native String GetAllAssets(long instance);
}
