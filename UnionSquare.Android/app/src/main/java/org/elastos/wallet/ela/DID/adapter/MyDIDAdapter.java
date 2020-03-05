package org.elastos.wallet.ela.DID.adapter;

import org.elastos.did.DIDAdapter;
import org.elastos.wallet.ela.DID.listener.MyDIDTransactionCallback;
import org.elastos.wallet.ela.base.BaseFragment;

public class MyDIDAdapter implements DIDAdapter {


    public MyDIDAdapter() {

    }

    private TransactionCallback callback;
    private MyDIDTransactionCallback myDIDTransactionCallback;

    private String txId;

    @Override
    public void createIdTransaction(String payload, String memo, int confirms, TransactionCallback callback) {
        //todo 手续费    confirms0 进交易池callback 有confirms给callback
        //如何指定那个钱包扣款 publish?
        // spv  createtran   sign  publish
        //baseFragment.post();
        //status 0 成功  非0失败
        this.callback = callback;
        if (myDIDTransactionCallback != null) {
            myDIDTransactionCallback.createIdTransaction(payload, memo, confirms, callback);
        }
//        confirms 0 or 1
        // callback.accept("", 0, null);// registcallback OnTransactionStatusChanged判断confirms时执行
        // ontxpublish错误时候callback.accept("非0","","错误信息");

    }

    public TransactionCallback getCallback() {
        return callback;
    }

    public void setCallback(TransactionCallback callback) {
        this.callback = callback;
    }

    public MyDIDTransactionCallback getMyDIDTransactionCallback() {
        return myDIDTransactionCallback;
    }

    public void setMyDIDTransactionCallback(MyDIDTransactionCallback myDIDTransactionCallback) {
        this.myDIDTransactionCallback = myDIDTransactionCallback;
    }

    public String getTxId() {
        return txId;
    }

    public void setTxId(String txId) {
        this.txId = txId;
    }
}
