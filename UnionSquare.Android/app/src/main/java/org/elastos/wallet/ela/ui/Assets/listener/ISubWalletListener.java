
package org.elastos.wallet.ela.ui.Assets.listener;


import org.json.JSONObject;

public interface ISubWalletListener {

    void OnTransactionStatusChanged(JSONObject jsonObject);

    /**
     * Callback method fired when block begin synchronizing with a peer. This callback could be used to show progress.
     */
    void OnBlockSyncStarted(JSONObject jsonObject);


    void OnBlockSyncProgress(JSONObject jsonObject);

    /**
     * Callback method fired when block end synchronizing with a peer. This callback could be used to show progress.
     */
    void OnBlockSyncStopped(JSONObject jsonObject);

    void OnBalanceChanged(JSONObject jsonObject);


    void OnTxPublished(JSONObject jsonObject);

    void OnTxDeleted(JSONObject jsonObject);

    void OnAssetRegistered(JSONObject jsonObject);

    void OnConnectStatusChanged(JSONObject jsonObject);
}
