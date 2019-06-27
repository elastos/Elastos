package org.elastos.wallet.ela.ui.Assets.fragment;

import android.content.Intent;
import android.os.Bundle;
import android.support.v7.widget.AppCompatEditText;
import android.text.TextUtils;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.TextView;

import org.elastos.wallet.R;
import org.elastos.wallet.ela.base.BaseFragment;
import org.elastos.wallet.ela.bean.BusEvent;
import org.elastos.wallet.ela.db.table.Wallet;
import org.elastos.wallet.ela.ui.Assets.activity.PwdActivity;
import org.elastos.wallet.ela.ui.vote.bean.Area;
import org.elastos.wallet.ela.utils.ClipboardUtil;
import org.elastos.wallet.ela.utils.DialogUtil;
import org.elastos.wallet.ela.utils.Log;
import org.elastos.wallet.ela.utils.RxEnum;
import org.elastos.wallet.ela.utils.SPUtil;
import org.elastos.wallet.ela.utils.listener.WarmPromptListener;
import org.greenrobot.eventbus.Subscribe;
import org.greenrobot.eventbus.ThreadMode;

import butterknife.BindView;
import butterknife.ButterKnife;
import butterknife.OnClick;
import butterknife.Unbinder;

public class SignTransactionFragment extends BaseFragment {
    @BindView(R.id.tv_title)
    TextView tvTitle;
    @BindView(R.id.et_sign)
    AppCompatEditText etSign;
    @BindView(R.id.et_tosign)
    AppCompatEditText etTosign;
    @BindView(R.id.et_chinid)
    AppCompatEditText etChinid;
    Unbinder unbinder;
    private Wallet wallet;


    @Override
    protected int getLayoutId() {
        return R.layout.fragment_sign_transaction;
    }

    @Override
    protected void setExtraData(Bundle data) {
        super.setExtraData(data);
        wallet = data.getParcelable("wallet");
    }

    @Override
    protected void initView(View view) {
        tvTitle.setText("签名");
        registReceiver();
    }

    @OnClick({R.id.tv_copy, R.id.tv_paste, R.id.tv_tosign})
    public void onViewClicked(View view) {

        switch (view.getId()) {
            case R.id.tv_copy:
                ClipboardUtil.copyClipboar(getBaseActivity(), etSign.getText().toString().trim());
                break;
            case R.id.tv_paste:
                etTosign.setText(ClipboardUtil.paste(getBaseActivity()));
                break;
            case R.id.tv_tosign:
                if (TextUtils.isEmpty(etChinid.getText().toString().trim())) {
                    showToast("请输入子链信息");
                    return;
                }
                if (TextUtils.isEmpty(etTosign.getText().toString().trim())) {
                    showToast("请输入代签名信息");
                    return;
                }
                Intent intent = new Intent(getActivity(), PwdActivity.class);
                intent.putExtra("wallet", wallet);
                intent.putExtra("chainId", etChinid.getText().toString().trim());
                intent.putExtra("attributes", etTosign.getText().toString().trim());
                intent.putExtra("type", 1);
                startActivity(intent);
                break;
        }
    }

    @Subscribe(threadMode = ThreadMode.MAIN)
    public void Event(BusEvent result) {
        int integer = result.getCode();
        if (integer == RxEnum.SIGNSUCCESS.ordinal()) {
            String signData = (String) result.getObj();
            etSign.setText(signData);
            showToast("签名成功且已经复制到剪切板");
            ClipboardUtil.copyClipboar(getBaseActivity(), signData);
        }
    }
}
