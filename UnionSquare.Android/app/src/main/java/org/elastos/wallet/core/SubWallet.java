// Copyright (c) 2012-2019 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

package org.elastos.wallet.core;

import org.elastos.wallet.ela.utils.Log;

import org.elastos.wallet.ela.db.table.Wallet;

/**
 * SubWallet
 */
public class SubWallet {
    private long mInstance;
    private SubWalletCallback mCallback = null;
    private String TAG = "SubWallet";

    public String GetChainID() throws WalletException {
        return GetChainID(mInstance);
    }

    public String GetBasicInfo() throws WalletException {
        return GetBasicInfo(mInstance);
    }

    public String GetBalanceInfo() throws WalletException {
        return GetBalanceInfo(mInstance);
    }

    public String GetBalance() throws WalletException {
        Log.d(TAG, "SubWallet [" + mInstance + "] get balance");
        return GetBalance(mInstance);
    }

    public String CreateAddress() throws WalletException {
        return CreateAddress(mInstance);
    }

    public String GetAllAddress(int start, int count) throws WalletException {
        return GetAllAddress(mInstance, start, count);
    }

    public String GetAllPublicKeys(int start, int count) throws WalletException {
        return GetAllPublicKeys(mInstance, start, count);
    }

    public String GetBalanceWithAddress(String address) throws WalletException {
        return GetBalanceWithAddress(mInstance, address);
    }

    public boolean IsCallbackRegistered() {
        return mCallback != null;
    }

    public SubWalletCallback getCallback() {
        return mCallback;
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
            RemoveCallback(mInstance);
            mCallback.Dispose();
            mCallback = null;
        } else {
            Log.w(TAG, "SubWallet[" + GetChainID() + "]'s callback already remove");
        }
    }

    public String CreateTransaction(String fromAddress, String toAddress, String amount, String memo) throws WalletException {
        return CreateTransaction(mInstance, fromAddress, toAddress, amount, memo);
    }

    public String GetAllUTXOs(int start, int count, String address) {
        return GetAllUTXOs(mInstance, start, count, address);
    }

    public String CreateConsolidateTransaction(String memo) throws WalletException {
        return CreateConsolidateTransaction(mInstance, memo);
    }

    public String SignTransaction(String rawTransaction, String payPassword) throws WalletException {
        return SignTransaction(mInstance, rawTransaction, payPassword);
    }

    public String GetTransactionSignedInfo(String rawTransaction) throws WalletException {
        return GetTransactionSignedInfo(mInstance, rawTransaction);
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

    public String GetOwnerPublicKeyRing() throws WalletException {
        return GetOwnerPublicKeyRing(mInstance);
    }

    public String GetAllCoinBaseTransaction(int start, int count, String txid) throws WalletException {
        return GetAllCoinBaseTransaction(mInstance, start, count, txid);
    }

    public String GetAssetInfo(String assetID) throws WalletException {
        return GetAssetInfo(mInstance, assetID);
    }

    public boolean SetFixedPeer(String address, int port) throws WalletException {
        return SetFixedPeer(mInstance, address, port);
    }

    public void SyncStart() throws WalletException {
        SyncStart(mInstance);
    }

    public void SyncStop() throws WalletException {
        SyncStop(mInstance);
    }

    public void Resync() throws WalletException {
        Resync(mInstance);
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

    private native String GetBalance(long subProxy);

    private native String CreateAddress(long subProxy);

    private native String GetAllAddress(long subProxy, int start, int count);

    private native String GetAllPublicKeys(long subProxy, int start, int count);

    private native String GetBalanceWithAddress(long subProxy, String address);

    private native void AddCallback(long subProxy, long subCallback);

    private native void RemoveCallback(long subProxys);

    private native String CreateTransaction(long subProxy, String fromAddress, String toAddress, String amount, String memo);

    private native String GetAllUTXOs(long subProxy, int start, int count, String address);

    private native String CreateConsolidateTransaction(long subProxy, String memo);

    private native String SignTransaction(long subProxy, String rawTransaction, String payPassword);

    private native String GetTransactionSignedInfo(long subProxy, String rawTransaction);

    private native String PublishTransaction(long subProxy, String rawTransaction);

    private native String GetAllTransaction(long subProxy, int start, int count, String addressOrTxId);

    private native String Sign(long subProxy, String message, String payPassword);

    private native boolean CheckSign(long subProxy, String publicKey, String message, String signature);

    private native String GetOwnerPublicKeyRing(long subProxy);

    private native String GetAllCoinBaseTransaction(long subProxy, int start, int count, String txid);

    private native String GetAssetInfo(long subProxy, String assetID);

    private native boolean SetFixedPeer(long subProxy, String address, int port);

    private native void SyncStart(long proxy);

    private native void SyncStop(long proxy);

    private native void Resync(long proxy);
}
