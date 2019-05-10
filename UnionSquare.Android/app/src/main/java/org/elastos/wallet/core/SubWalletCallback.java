// Copyright (c) 2012-2019 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

package org.elastos.wallet.core;

import android.util.Log;

import org.elastos.wallet.ela.ui.Assets.listener.ISubWalletListener;
import org.json.JSONException;
import org.json.JSONObject;

public class SubWalletCallback {
    private long mInstance;
    private String mMasterWalletID;
    private String mSubWalletID;
    private ISubWalletListener mListener;
    private String TAG = "SubWalletCallback";

    public long GetProxy() {
        return mInstance;
    }

    public SubWalletCallback(String masterWalletID, String subWalletID, ISubWalletListener listener) {
        mMasterWalletID = masterWalletID;
        mSubWalletID = subWalletID;
        mListener = listener;
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
        JSONObject jsonObject = new JSONObject();
        try {
            jsonObject.put("txId", txID);
            jsonObject.put("status", status);
            jsonObject.put("desc", desc);
            jsonObject.put("confirms", confirms);
            jsonObject.put("MasterWalletID", mMasterWalletID);
            jsonObject.put("ChaiID", mSubWalletID);
            jsonObject.put("Action", "OnTransactionStatusChanged");

            mListener.OnTransactionStatusChanged(jsonObject);
        } catch (JSONException e) {
            e.printStackTrace();
        }
    }

    /**
     * Callback method fired when block begin synchronizing with a peer. This callback could be used to show progress.
     */
    public void OnBlockSyncStarted() {
        Log.i(TAG, GetWalletID() + "[OnBlockSyncStarted]");
        JSONObject jsonObject = new JSONObject();
        try {
            jsonObject.put("MasterWalletID", mMasterWalletID);
            jsonObject.put("ChaiID", mSubWalletID);
            jsonObject.put("Action", "OnBlockSyncStarted");

            mListener.OnBlockSyncStarted(jsonObject);
        } catch (JSONException e) {
            e.printStackTrace();

        }
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
        JSONObject jsonObject = new JSONObject();
        try {
            jsonObject.put("currentBlockHeight", currentBlockHeight);
            jsonObject.put("estimatedHeight", estimatedHeight);
            jsonObject.put("lastBlockTime", lastBlockTime);
            jsonObject.put("MasterWalletID", mMasterWalletID);
            jsonObject.put("ChaiID", mSubWalletID);
            jsonObject.put("Action", "OnBlockHeightIncreased");

            mListener.OnBlockSyncProgress(jsonObject);
        } catch (JSONException e) {
            e.printStackTrace();

        }
    }

    /**
     * Callback method fired when block end synchronizing with a peer. This callback could be used to show progress.
     */
    public void OnBlockSyncStopped() {
        Log.i(TAG, GetWalletID() + "[OnBlockSyncStopped]");
        JSONObject jsonObject = new JSONObject();
        try {
            jsonObject.put("MasterWalletID", mMasterWalletID);
            jsonObject.put("ChaiID", mSubWalletID);
            jsonObject.put("Action", "OnBlockSyncStopped");

            mListener.OnBlockSyncStopped(jsonObject);
        } catch (JSONException e) {
            e.printStackTrace();
        }
    }

    public void OnBalanceChanged(String assetID, long balance) {
        Log.i(TAG, GetWalletID() + "[OnBalanceChanged] " + assetID + " = " + balance);
        JSONObject jsonObject = new JSONObject();
        try {
            jsonObject.put("Asset", assetID);
            jsonObject.put("Balance", balance);
            jsonObject.put("MasterWalletID", mMasterWalletID);
            jsonObject.put("ChaiID", mSubWalletID);
            jsonObject.put("Action", "OnBalanceChanged");

            mListener.OnBalanceChanged(jsonObject);
        } catch (JSONException e) {
            e.printStackTrace();
        }
    }

    public void OnTxPublished(String hash, String result) {
        Log.i(TAG, GetWalletID() + "[OnTxPublished] " + hash + ", result: " + result);
        JSONObject jsonObject = new JSONObject();
        try {
            jsonObject.put("hash", hash);
            jsonObject.put("result", result);
            jsonObject.put("MasterWalletID", mMasterWalletID);
            jsonObject.put("ChaiID", mSubWalletID);
            jsonObject.put("Action", "OnTxPublished");

            mListener.OnTxPublished(jsonObject);
        } catch (JSONException e) {
            e.printStackTrace();
        }
    }

    public void OnTxDeleted(String hash, boolean notifyUser, boolean recommendRescan) {
        Log.i(TAG, GetWalletID() + "[OnTxDeleted] " + hash + ", notifyUser: " + notifyUser + ", recommendRescan: " + recommendRescan);
        JSONObject jsonObject = new JSONObject();
        try {
            jsonObject.put("hash", hash);
            jsonObject.put("notifyUser", notifyUser);
            jsonObject.put("recommendRescan", recommendRescan);
            jsonObject.put("MasterWalletID", mMasterWalletID);
            jsonObject.put("ChaiID", mSubWalletID);
            jsonObject.put("Action", "OnTxDeleted");

            mListener.OnTxDeleted(jsonObject);
        } catch (JSONException e) {
            e.printStackTrace();
        }
    }

    private native long InitSubWalletCallback();

    private native void DisposeNative(long proxy);
}
