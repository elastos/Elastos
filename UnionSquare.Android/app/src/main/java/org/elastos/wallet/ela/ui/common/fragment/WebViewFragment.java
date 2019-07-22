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
        setLanguage();
    }

    private void setLanguage() {
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
    }
}
