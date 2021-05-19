// Copyright (c) 2012-2019 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

package org.elastos.wallet.core;

public class WalletException extends RuntimeException {
    private int mErrorCode = 0;
    private String mErrorInfo = null;

    public WalletException() {
        super();
    }

    public WalletException(String message) {
        super(message);
        mErrorInfo = message;
    }

    public WalletException(int errorCode, String message) {
        super(message);
        mErrorCode = errorCode;
        mErrorInfo = message;
    }

    public int GetErrorCode() {
        return mErrorCode;
    }

    public String GetErrorInfo() {
        return mErrorInfo;
    }
}
