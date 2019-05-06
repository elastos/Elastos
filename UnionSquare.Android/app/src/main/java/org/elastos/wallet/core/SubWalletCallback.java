// Copyright (c) 2012-2019 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

package org.elastos.wallet.core;

import android.util.Log;

public class SubWalletCallback {
    private long mInstance;
    private String mMasterWalletID;
    private String mSubWalletID;
    private String TAG = "SubWalletCallback";

    public long GetProxy() {
        return mInstance;
    }

    public SubWalletCallback(String masterWalletID, String subWalletID) {
        mMasterWalletID = masterWalletID;
        mSubWalletID = subWalletID;

        mInstance = InitSubWalletCallback();
    }

    public String GetWalletID() {
        return mMasterWalletID + ":" + mSubWalletID + " ";
    }

    /*
     * nNed to do GC of jni manually, because the finalization mechanism is Deprecated.
     */
    public void Dispose() {
        Log.w(TAG, GetWalletID() + "Dispose");
        DisposeNative(mInstance);
    }

    /**
     * Callback method fired when status of a transaction changed.
     *
     * @param txID     indicate hash of the transaction.
     * @param status   can be "Added", "Deleted" or "Updated".
     * @param desc     is an detail description of transaction status.
     * @param confirms is confirm count util this callback fired.
     */
    public void OnTransactionStatusChanged(String txID, String status, String desc, int confirms) {
        Log.i(TAG, GetWalletID() + "[OnTransactionStatusChanged] " + txID + "," + status + "," + confirms);
    }

    /**
     * Callback method fired when block begin synchronizing with a peer. This callback could be used to show progress.
     */
    public void OnBlockSyncStarted() {
        Log.i(TAG, GetWalletID() + "[OnBlockSyncStarted]");
    }

    /**
     * Callback method fired when best block chain height increased. This callback could be used to show progress.
     *
     * @param currentBlockHeight is the of current block when callback fired.
     * @param estimatedHeight    is max height of blockchain.
     * @param lastBlockTime      timestamp of the last block.
     */
    public void OnBlockSyncProgress(int currentBlockHeight, int estimatedHeight, long lastBlockTime) {
        Log.i(TAG, GetWalletID() + "[OnBlockSyncProgress] (" + currentBlockHeight + "/" + estimatedHeight + ") t = " + lastBlockTime);
    }

    /**
     * Callback method fired when block end synchronizing with a peer. This callback could be used to show progress.
     */
    public void OnBlockSyncStopped() {
        Log.i(TAG, GetWalletID() + "[OnBlockSyncStopped]");
    }

    public void OnBalanceChanged(String assetID, long balance) {
        Log.i(TAG, GetWalletID() + "[OnBalanceChanged] " + assetID + " = " + balance);
    }

    public void OnTxPublished(String hash, String result) {
        Log.i(TAG, GetWalletID() + "[OnTxPublished] " + hash + ", result: " + result);
    }

    public void OnTxDeleted(String hash, boolean notifyUser, boolean recommendRescan) {
        Log.i(TAG, GetWalletID() + "[OnTxDeleted] " + hash + ", notifyUser: " + notifyUser + ", recommendRescan: " + recommendRescan);
    }

    private native long InitSubWalletCallback();

    private native void DisposeNative(long proxy);
}
