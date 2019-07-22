package org.elastos.wallet.ela.ui.Assets.fragment;


import android.os.Bundle;
import android.support.v7.widget.AppCompatTextView;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.view.WindowManager;
import android.widget.TextView;

import org.elastos.wallet.R;
import org.elastos.wallet.ela.base.BaseFragment;

import butterknife.BindView;
import butterknife.ButterKnife;
import butterknife.OnClick;
import butterknife.Unbinder;

/**
 * 助记词
 */
public class OutportMnemonicFragment extends BaseFragment /*<MnemonicWordPresenter>implements MnemonicWordContract.View */ {


    @BindView(R.id.tv_title)
    TextView tvTitle;
    @BindView(R.id.tv_mnemonic)
    AppCompatTextView tvMnemonic;
    Unbinder unbinder;
    private String mnemonic;

    @Override
    protected int getLayoutId() {
        getBaseActivity().getWindow().setFlags(WindowManager.LayoutParams.FLAG_SECURE, WindowManager.LayoutParams.FLAG_SECURE);
        return R.layout.fragment_outport_mnemonic;
    }

    @Override
    protected void setExtraData(Bundle data) {
        mnemonic = data.getString("mnemonic");
        tvMnemonic.setText(mnemonic);
      /*  String[] re = mnemonic.split(" ");//用split()函数直接分割
        tvMnemonic.setText(regex(Arrays.toString(Arrays.copyOfRange(re, 0, 8))) + "\n\n" +
                regex(Arrays.toString(Arrays.copyOfRange(re, 8, re.length))));*/
    }

    @Override
    protected void initView(View view) {
        tvTitle.setText(getString(R.string.outportMnemonic));

    }


    @OnClick({R.id.sb})
    public void onViewClicked(View view) {
        switch (view.getId()) {

            case R.id.sb:

                Bundle bundle = new Bundle();
                bundle.putString("openType", "manager");
                bundle.putString("mnemonic", mnemonic);
                start(VerifyMnemonicWordsFragment.class,bundle);
                break;
        }
    }


    //正则去掉逗号 括号
    private String regex(String at) {
        return at.replaceAll("\\[", "").
                replaceAll("\\]", "").replaceAll(",", "");
    }

    @Override
    public void onDestroy() {
        super.onDestroy();
        getBaseActivity().getWindow().clearFlags(WindowManager.LayoutParams.FLAG_SECURE);
    }
}
