package org.elastos.wallet.ela.utils.listener;

import android.app.Activity;
import android.content.Context;
import android.view.WindowManager;
import android.widget.PopupWindow;

/**
 * Created by wangdongfeng on 2018/4/16.
 */

public class PopupDismissListener implements PopupWindow.OnDismissListener {

    private Context context;

    public PopupDismissListener(Context context) {
        this.context = context;
    }

    @Override
    public void onDismiss() {
        backgroundAlpha(context,1f);
    }

    public void backgroundAlpha(Context context, float bgAlpha) {
        WindowManager.LayoutParams lp = ((Activity) context).getWindow().getAttributes();
        lp.alpha = bgAlpha; // 0.0-1.0
        ((Activity) context).getWindow().setAttributes(lp);
        this.context = null;
    }
}
