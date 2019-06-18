package org.elastos.wallet.ela.ui.mine.fragment;

import android.content.Intent;
import android.net.Uri;
import android.view.View;
import android.widget.TextView;

import org.elastos.wallet.R;
import org.elastos.wallet.ela.base.BaseFragment;
import org.elastos.wallet.ela.utils.ClipboardUtil;
import org.elastos.wallet.ela.utils.Constant;
import org.elastos.wallet.ela.utils.MyUtil;

import butterknife.BindView;
import butterknife.OnClick;

public class AboutFragment extends BaseFragment {

    @BindView(R.id.tv_title)
    TextView tvTitle;

    @BindView(R.id.tv_version_name)
    TextView tvVersionName;


    @Override
    protected int getLayoutId() {
        return R.layout.fragment_about;
    }

    @Override
    protected void initInjector() {

    }

    @Override
    protected void initView(View view) {
        tvTitle.setText(getString(R.string.about));
        tvVersionName.setText(getString(R.string.currentversionname) + MyUtil.getVersionName(getContext()));

    }


    @OnClick({R.id.tv_updatalog, R.id.tv_feedback})
    public void onViewClicked(View view) {
        switch (view.getId()) {
            case R.id.tv_updatalog:
                Intent intent = new Intent("android.intent.action.VIEW");
                intent.setData(Uri.parse("https://news.elastos.org/elastos-dpos-supernode-election-process/"));
                startActivity(intent);
                break;
            case R.id.tv_feedback:
                ClipboardUtil.copyClipboar(getBaseActivity(), Constant.Email);
                break;
        }
    }
}
