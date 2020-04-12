/*
 * Copyright (c) 2019 Elastos Foundation
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

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
