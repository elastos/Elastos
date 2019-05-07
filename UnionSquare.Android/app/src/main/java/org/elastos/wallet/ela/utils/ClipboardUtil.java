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
        } else {
            context.showToastMessage(context.getString(R.string.copyfail));
        }
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

    public static void copyClipboar(BaseActivity context, String wallrtAddr, int yu) {
        if (!TextUtils.isEmpty(wallrtAddr)) {
            ClipboardManager cm = (ClipboardManager) context.getSystemService(Context.CLIPBOARD_SERVICE);
            ClipData mClipData = ClipData.newPlainText("Label", wallrtAddr);
            cm.setPrimaryClip(mClipData);
            //context.showToastMessage("已复制到剪切板");
        } else {
            context.showToastMessage(context.getString(R.string.copyfail));
        }
    }

    public static String paste(BaseActivity activity) {
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
