package org.elastos.wallet.ela.ui.Assets;

import android.os.Bundle;
import android.text.method.ScrollingMovementMethod;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.TextView;

import org.elastos.wallet.R;
import org.elastos.wallet.ela.base.BaseFragment;
import org.elastos.wallet.ela.ui.Assets.fragment.AddressListFragment;
import org.elastos.wallet.ela.utils.ClipboardUtil;

import butterknife.BindView;
import butterknife.ButterKnife;
import butterknife.OnClick;
import butterknife.Unbinder;

public class ErrorScanFragment extends BaseFragment {
    @BindView(R.id.tv_erro)
    TextView tvErro;
    @BindView(R.id.tv_title)
    TextView tvTitle;
    @Override
    protected void setExtraData(Bundle data) {
        String result = data.getString("result");
        tvErro.setText(result);
    }

    @Override
    protected int getLayoutId() {
        return R.layout.fragment_errorscan;
    }

    @Override
    protected void initView(View view) {
        tvErro.setScrollbarFadingEnabled(false);
        tvErro.setMovementMethod(ScrollingMovementMethod.getInstance());
    }

    @OnClick({R.id.tv_copy})
    public void onViewClicked(View view) {
        //复制
        ClipboardUtil.copyClipboar(getBaseActivity(), tvErro.getText().toString().trim());
    }
}
