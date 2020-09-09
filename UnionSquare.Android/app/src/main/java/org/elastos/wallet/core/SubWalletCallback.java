// Copyright (c) 2012-2019 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

package org.elastos.wallet.core;

import com.google.gson.JsonObject;

import org.elastos.wallet.ela.ui.Assets.listener.ISubWalletListener;
import org.elastos.wallet.ela.utils.Log;
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

    public void setListener(ISubWalletListener listener) {
        mListener = listener;
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
            jsonObject.put("ChainID", mSubWalletID);

            mListener.OnTransactionStatusChanged(jsonObject);
        } catch (JSONException e) {
            e.printStackTrace();
        }
    }

    /**
     * Callback method fired when best block chain height increased. This callback could be used to show progress.
     *
     * @param progressInfo progress info contain detail as below:
     *                     {
     *                     "Progress": 50,                    # 0% ~ 100%
     *                     "BytesPerSecond": 12345678,        # 12.345678 MByte / s
     *                     "LastBlockTime": 1573799697,       # timestamp of last block
     *                     "DownloadPeer": "127.0.0.1"        # IP address of node
     *                     }
     */
    public void OnBlockSyncProgress(String progressInfo) {
        try {
            JSONObject jProgressInfo = new JSONObject(progressInfo);
            jProgressInfo.put("MasterWalletID", mMasterWalletID);
            jProgressInfo.put("ChainID", mSubWalletID);

            mListener.OnBlockSyncProgress(jProgressInfo);
        } catch (JSONException e) {
            e.printStackTrace();

        }
    }

    public void OnBalanceChanged(String assetID, String balance) {
//        Log.i(TAG, GetWalletID() + "[OnBalanceChanged] " + assetID + " = " + balance);
        JSONObject jsonObject = new JSONObject();
        try {
            jsonObject.put("Asset", assetID);
            jsonObject.put("Balance", balance);
            jsonObject.put("MasterWalletID", mMasterWalletID);
            jsonObject.put("ChainID", mSubWalletID);

            mListener.OnBalanceChanged(jsonObject);
        } catch (JSONException e) {
            e.printStackTrace();
        }
    }

    public void OnTxPublished(String hash, String result) {
//        Log.i(TAG, GetWalletID() + "[OnTxPublished] " + hash + ", result: " + result);
        JSONObject jsonObject = new JSONObject();
        try {
            jsonObject.put("hash", hash);
            jsonObject.put("result", result);
            jsonObject.put("MasterWalletID", mMasterWalletID);
            jsonObject.put("ChainID", mSubWalletID);

            mListener.OnTxPublished(jsonObject);
        } catch (JSONException e) {
            e.printStackTrace();
        }
    }

    public void OnAssetRegistered(String asset, String info) {
//        Log.i(TAG, GetWalletID() + "[OnAssetRegistered] " + asset + ", info: " + info);
        JSONObject jsonObject = new JSONObject();
        try {
            jsonObject.put("asset", asset);
            jsonObject.put("info", info);
            jsonObject.put("MasterWalletID", mMasterWalletID);
            jsonObject.put("ChainID", mSubWalletID);
            mListener.OnAssetRegistered(jsonObject);
        } catch (JSONException e) {
            e.printStackTrace();
        }
    }

    public void OnConnectStatusChanged(String status) {
        JSONObject jsonObject = new JSONObject();
        Log.i(TAG, GetWalletID() + "[OnConnectStatusChanged] status=" + status);


        try {
            jsonObject.put("status", status);
            jsonObject.put("MasterWalletID", mMasterWalletID);
            jsonObject.put("ChainID", mSubWalletID);
            mListener.OnConnectStatusChanged(jsonObject);
        } catch (JSONException e) {
            e.printStackTrace();
        }
    }

    public void OnETHSCEventHandled(String event) {
        JSONObject jsonObject = new JSONObject();

        try {
            jsonObject.put("event", event);
            jsonObject.put("MasterWalletID", mMasterWalletID);
            jsonObject.put("ChainID", mSubWalletID);
            mListener.OnConnectStatusChanged(jsonObject);
        } catch (JSONException e) {
            e.printStackTrace();
        }
    }

    public String GasPrice(int id) {
        JSONObject jsonObject = new JSONObject();

        return jsonObject.toString();
    }

    /**
     * @param from
     * @param to
     * @param amount
     * @param gasPrice
     * @param data
     * @param id request id
     * @return If successful, return below. Otherwise {} or null will be returned to indicate the error.
     * {
     *   "id": 0,
     *   "result": "0x5208" // 21000
     * }
     */
    public String EstimateGas(String from, String to, String amount, String gasPrice, String data, int id) {
        JsonObject jsonObject = new JsonObject();

        return jsonObject.toString();
    }

    /**
     * @param address
     * @param id
     * @return
     * {
     *   "id": 0,
     *   "result": "0x0234c8a3397aab58" // 158972490234375000
     * }
     */
    public String GetBalance(String address, int id) {
        JsonObject jsonObject = new JsonObject();

        return jsonObject.toString();
    }

    /**
     * @param tx Signed raw transaction.
     * @param id
     * @return If successful, return below. Otherwise {} or null will be returned to indicate the error.
     * {
     *   "id": 0,
     *   "result": "0xe670ec64341771606e55d6b4ca35a1a6b75ee3d5145a99d05921026d1527331"
     * }
     */
    public String SubmitTransaction(String tx, int id) {
        JsonObject jsonObject = new JsonObject();

        return jsonObject.toString();
    }

    /**
     * @param begBlockNumber
     * @param endBlockNumber
     * @return If successful, return below. Otherwise {} or null will be returned to indicate the error.
     * {
     *   "id": 0,
     *   "result": [{
     *     "hash":"0x88df016429689c079f3b2f6ad39fa052532c56795b733da78a91ebe6a713944b",
     *     "from":"0xa7d9ddbe1f17865597fbd27ec712455208b6b76d",
     *     "to":"0xf02c1c8e6114b1dbe8937a39260b5b0a374432bb",
     *     "contract":"0xb60e8dd61c5d32be8058bb8eb970870f07233155", // or "", if none was created
     *     "amount":"0xf3dbb76162000", // 4290000000000000
     *     "gasLimit":"0x1388",
     *     "gasPrice":"0x4a817c800", // 20000000000
     *     "data":"0x68656c6c6f21", // input
     *     "nonce":"0x15", // 21
     *     "gasUsed":"0xc350", // 50000
     *     "blockNumber":"0x5daf3b", // 6139707
     *     "blockHash":"0x1d59ff54b1eb26b013ce3cb5fc9dab3705b415a67127a003c3e61eb445bb8df2",
     *     "blockConfirmations":"0x100", // 256
     *     "blockTransactionIndex":"0x41", // 65
     *     "blockTimestamp":"0x55ba467c",
     *     "isError": "0"
     *   },
     *   {
     *     ...
     *   }]
     * }
     */
    public String GetTransactions(String address, long begBlockNumber, long endBlockNumber, int id) {
        JsonObject jsonObject = new JsonObject();

        return jsonObject.toString();
    }

    /**
     * @param contract
     * @param address
     * @param event
     * @param begBlockNumber
     * @param endBlockNumber
     * @param id
     * @return If successful, return below. Otherwise {} or null will be returned to indicate the error.
     * {
     *   "id": 0,
     *   "result": [{
     *     "hash":"0xdf829c5a142f1fccd7d8216c5785ac562ff41e2dcfdf5785ac562ff41e2dcf",
     *     "contract":"0xb60e8dd61c5d32be8058bb8eb970870f07233155", // or "", if none was created
     *     "topics":["0x59ebeb90bc63057b6515673c3ecf9438e5058bca0f92585014eced636878c9a5"]
     *     "data":"0x0000000000000000000000000000000000000000000000000000000000000000",
     *     "gasPrice":"0x4a817c800", // 20000000000
     *     "gasUsed":"0x4dc", // 1244
     *     "logIndex":"0x1", // 1
     *     "blockNumber":"0x1b4", // 436
     *     "blockTransactionIndex":"0x0", // 0
     *     "blockTimestamp":"0x55ba467c",
     *   },{
     *    ...
     *   }]
     * }
     */
    public String GetLogs(String contract, String address, String event, long begBlockNumber, long endBlockNumber, int id) {
        JsonObject jsonObject = new JsonObject();

        return jsonObject.toString();
    }

    /**
     * @param id
     * @return If successful, return below. Otherwise {} or null will be returned to indicate the error.
     * [{
     *   "id":0,
     *   "address": "0x407d73d8a49eeb85d32cf465507dd71d507100c1",
     *   "symbol": "ELA",
     *   "name": "elastos",
     *   "description": "desc",
     *   "decimals": 18,
     *   "defaultGasLimit": "0x1388",
     *   "defaultGasPrice": "0x1dfd14000" // 8049999872 Wei
     * },{
     *   ...
     * }]
     */
    public String GetTokens(int id) {
        JsonObject jsonObject = new JsonObject();

        return jsonObject.toString();
    }

    /**
     * @param id
     * @return If successful, return below. Otherwise {} or null will be returned to indicate the error.
     * {
     *   "id":0,
     *   "result": "0x4b7" // 1207
     * }
     */
    public String GetBlockNumber(int id) {
        JsonObject jsonObject = new JsonObject();

        return jsonObject.toString();
    }

    /**
     * @param address
     * @param id
     * @return If successful, return below. Otherwise {} or null will be returned to indicate the error.
     * {
     *   "id": 0,
     *   "result": "0x1" // 1
     * }
     */
    public String GetNonce(String address, int id) {
        JsonObject jsonObject = new JsonObject();

        return jsonObject.toString();
    }

    private native long InitSubWalletCallback();

    private native void DisposeNative(long proxy);
}
