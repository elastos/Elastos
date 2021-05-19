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

import android.content.ClipData;
import android.content.ClipboardManager;
import android.content.Context;
import android.text.TextUtils;

import org.elastos.wallet.R;
import org.elastos.wallet.ela.base.BaseActivity;

import javax.inject.Inject;

import static android.content.Context.CLIPBOARD_SERVICE;

/**
 * 剪切板工具类
 * Created by wangdongfeng on 2018/4/18.
 */

public class ClipboardUtil {
    @Inject
    public ClipboardUtil() {
    }

    public static void copyClipboar(BaseActivity context, String wallrtAddr) {
        if (!TextUtils.isEmpty(wallrtAddr)) {
            ClipboardManager cm = (ClipboardManager) context.getSystemService(Context.CLIPBOARD_SERVICE);
            ClipData mClipData = ClipData.newPlainText("Label", wallrtAddr);
            cm.setPrimaryClip(mClipData);
            context.showToastMessage(context.getString(R.string.copysucess));
        } /*else {
            context.showToastMessage(context.getString(R.string.copyfail));
        }*/
    }

    public static void copyClipboar(BaseActivity context, String wallrtAddr, String text) {
        if (!TextUtils.isEmpty(wallrtAddr)) {
            ClipboardManager cm = (ClipboardManager) context.getSystemService(Context.CLIPBOARD_SERVICE);
            ClipData mClipData = ClipData.newPlainText("Label", wallrtAddr);
            cm.setPrimaryClip(mClipData);
            context.showToastMessage(text);
        } else {
            context.showToastMessage(context.getString(R.string.copyfail));
        }
    }



    public static String paste(Context activity) {
        ClipboardManager mClipboardManager = (ClipboardManager) (activity.getSystemService(CLIPBOARD_SERVICE));
        if (mClipboardManager.hasPrimaryClip()) {//判断剪切板是否有数据
            ClipData.Item item = mClipboardManager.getPrimaryClip().getItemAt(0);
            CharSequence text = item.getText();
            if (text == null) {
                return "";
            }
            return text.toString();

        }
        return "";
    }
}
