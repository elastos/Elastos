// Copyright (c) 2012-2019 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

package org.elastos.wallet.core;

import android.util.Log;

/**
 * SubWallet
 */
public class SubWallet {
    private long mInstance;
    private SubWalletCallback mCallback = null;
    private String TAG = "SubWallet";

    public enum BalanceType {
        Default,
        Voted,
        Total,
    }

    public String GetChainID() throws WalletException {
        return GetChainID(mInstance);
    }

    public String GetBasicInfo() throws WalletException {
        return GetBasicInfo(mInstance);
    }

    public String GetBalanceInfo() throws WalletException {
        return GetBalanceInfo(mInstance);
    }

    public long GetBalance(BalanceType type) throws WalletException {
        Log.d(TAG, "SubWallet [" + mInstance + "] get balance");
        return GetBalance(mInstance, type.ordinal());
    }

    public String CreateAddress() throws WalletException {
        return CreateAddress(mInstance);
    }

    public String GetAllAddress(int start, int count) throws WalletException {
        return GetAllAddress(mInstance, start, count);
    }

    public long GetBalanceWithAddress(String address, BalanceType type) throws WalletException {
        return GetBalanceWithAddress(mInstance, address, type.ordinal());
    }

    public boolean IsCallbackRegistered() {
        return mCallback != null;
    }

    public void AddCallback(SubWalletCallback subCallback) throws WalletException {
        if (mCallback == null) {
            Log.d(TAG, "SubWallet[" + mInstance + "] adding callback");
            AddCallback(mInstance, subCallback.GetProxy());
            mCallback = subCallback;
        } else {
            Log.w(TAG, "SubWallet[" + GetChainID() + "]'s callback already registered");
        }
    }

    public void RemoveCallback() throws WalletException {
        if (mCallback != null) {
            Log.d(TAG, "SubWallet[" + mInstance + "] removing callback");
            RemoveCallback(mInstance, mCallback.GetProxy());
            mCallback.Dispose();
            mCallback = null;
        } else {
            Log.w(TAG, "SubWallet[" + GetChainID() + "]'s callback already remove");
        }
    }

    public String CreateTransaction(String fromAddress, String toAddress, long amount, String memo, String remark, boolean useVotedUTXO) throws WalletException {
        return CreateTransaction(mInstance, fromAddress, toAddress, amount, memo, remark, useVotedUTXO);
    }

    public long CalculateTransactionFee(String rawTransaction, long feePerKb) throws WalletException {
        return CalculateTransactionFee(mInstance, rawTransaction, feePerKb);
    }

    public String UpdateTransactionFee(String rawTransaction, long fee, String fromAddress) throws WalletException {
        return UpdateTransactionFee(mInstance, rawTransaction, fee, fromAddress);
    }

    public String SignTransaction(String rawTransaction, String payPassword) throws WalletException {
        return SignTransaction(mInstance, rawTransaction, payPassword);
    }

    public String GetTransactionSignedSigners(String rawTransaction) throws WalletException {
        return GetTransactionSignedSigners(mInstance, rawTransaction);
    }

    public String PublishTransaction(String rawTransaction) throws WalletException {
        return PublishTransaction(mInstance, rawTransaction);
    }

    public String GetAllTransaction(int start, int count, String addressOrTxId) throws WalletException {
        return GetAllTransaction(mInstance, start, count, addressOrTxId);
    }

    public String Sign(String message, String payPassword) throws WalletException {
        return Sign(mInstance, message, payPassword);
    }

    public boolean CheckSign(String publicKey, String message, String signature) throws WalletException {
        return CheckSign(mInstance, publicKey, message, signature);
    }

    public String GetPublicKey() throws WalletException {
        return GetPublicKey(mInstance);
    }

    public SubWallet(long instance) {
        mInstance = instance;
    }

    protected long GetProxy() {
        return mInstance;
    }

    private native String GetChainID(long subProxy);

    private native String GetBasicInfo(long subProxy);

    private native String GetBalanceInfo(long subProxy);

    private native long GetBalance(long subProxy, int balanceType);

    private native String CreateAddress(long subProxy);

    private native String GetAllAddress(long subProxy, int start, int count);

    private native long GetBalanceWithAddress(long subProxy, String address, int balanceType);

    private native void AddCallback(long subProxy, long subCallback);

    private native void RemoveCallback(long subProxys, long subCallback);

    private native String CreateTransaction(long subProxy, String fromAddress, String toAddress, long amount, String memo, String remark, boolean useVotedUTXO);

    private native long CalculateTransactionFee(long subProxy, String rawTransaction, long feePerKb);

    private native String UpdateTransactionFee(long subProxy, String rawTransaction, long fee, String fromAddress);

    private native String SignTransaction(long subProxy, String rawTransaction, String payPassword);

    private native String GetTransactionSignedSigners(long subProxy, String rawTransaction);

    private native String PublishTransaction(long subProxy, String rawTransaction);

    private native String GetAllTransaction(long subProxy, int start, int count, String addressOrTxId);

    private native String Sign(long subProxy, String message, String payPassword);

    private native boolean CheckSign(long subProxy, String publicKey, String message, String signature);

    private native String GetPublicKey(long jSubProxy);
}
