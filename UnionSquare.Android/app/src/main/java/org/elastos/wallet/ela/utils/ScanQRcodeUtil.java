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

package org.elastos.wallet.ela.utils;

import android.content.Intent;

import org.elastos.wallet.ela.base.BaseFragment;

import javax.inject.Inject;

import io.github.xudaojie.qrcodelib.CaptureActivity;

/**
 * Created by wangdongfeng on 2018/4/26.
 */

public class ScanQRcodeUtil {

    @Inject
    public ScanQRcodeUtil() {
    }



    public static final int SCAN_QR_REQUEST_CODE = 20080;

    public void scanQRcode(BaseFragment activity) {
        Intent intent = new Intent(activity.getContext(), CaptureActivity.class);
        activity.startActivityForResult(intent, SCAN_QR_REQUEST_CODE);
    }
}
