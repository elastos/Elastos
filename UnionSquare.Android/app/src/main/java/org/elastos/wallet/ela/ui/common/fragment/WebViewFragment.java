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

package org.elastos.wallet.ela.ui.common.fragment;

import android.content.res.Configuration;
import android.content.res.Resources;
import android.os.Build;
import android.os.Bundle;
import android.util.DisplayMetrics;
import android.view.View;
import android.webkit.WebChromeClient;
import android.webkit.WebSettings;
import android.webkit.WebView;

import com.qmuiteam.qmui.util.QMUIStatusBarHelper;

import org.elastos.wallet.R;
import org.elastos.wallet.ela.base.BaseFragment;
import org.elastos.wallet.ela.utils.Constant;
import org.elastos.wallet.ela.utils.SPUtil;

import java.util.Locale;

import butterknife.BindView;

public class WebViewFragment extends BaseFragment {

    @BindView(R.id.webview)
    WebView commonWebView;
    private String url;


    @Override
    protected int getLayoutId() {
        return R.layout.fragment_webview;
    }

    @Override
    protected void setExtraData(Bundle data) {
        super.setExtraData(data);
        url = data.getString(Constant.FRAGMENTTAG, "");
    }

    @Override
    protected void initView(View view) {
        mRootView.setBackgroundResource(R.color.white);
        QMUIStatusBarHelper.setStatusBarLightMode(getActivity());
        WebSettings webSettings = commonWebView.getSettings();
        commonWebView.getSettings().setDomStorageEnabled(true);
        //设置WebView属性，能够执行Javascript脚本
        webSettings.setJavaScriptEnabled(true);
        //webSettings.setCacheMode(WebSettings.LOAD_NO_CACHE);
        // 支持启用缓存模式
        webSettings.setAppCacheEnabled(true);
        commonWebView.setWebChromeClient(new WebChromeClient() {
        });
        /*if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.KITKAT) {
            // chromium, enable hardware acceleration
            commonWebView.setLayerType(View.LAYER_TYPE_HARDWARE, null);//华为手机容易Unable to create layer View  webView.setLayerType(View.LAYER_TYPE_NONE, null);
        } else {
            // older android version, disable hardware acceleration
            commonWebView.setLayerType(View.LAYER_TYPE_SOFTWARE, null);
        }
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.LOLLIPOP) {
            commonWebView.getSettings().setMixedContentMode(WebSettings.MIXED_CONTENT_ALWAYS_ALLOW);//运行httpsapp加载httpwangy
        }*/
        commonWebView.loadUrl(url);

    }

    @Override
    public void onDestroy() {
        super.onDestroy();

        QMUIStatusBarHelper.setStatusBarDarkMode(getActivity());
       // setLanguage();
    }

   /* private void setLanguage() {
        if (new SPUtil(getContext()).getLanguage() == -1) {
            if (Locale.getDefault().getLanguage().equals("zh")) {
                new SPUtil(getContext()).setLanguage(0);
            } else if (Locale.getDefault().getLanguage().equals("en")) {
                new SPUtil(getContext()).setLanguage(1);
            } else {
                new SPUtil(getContext()).setLanguage(1);
                changeAppLanguage();
            }
        } else {
            changeAppLanguage();
        }

    }

    public void changeAppLanguage() {
        String sta = new SPUtil(getContext()).getLanguage() == 0 ? "zh" : "en";//这是SharedPreferences工具类，用于保存设置，代码很简单，自己实现吧
        // 本地语言设置
        Locale myLocale = new Locale(sta);
        Resources res = getResources();
        DisplayMetrics dm = res.getDisplayMetrics();
        Configuration conf = res.getConfiguration();
        conf.locale = myLocale;
        res.updateConfiguration(conf, dm);
    }*/
}
