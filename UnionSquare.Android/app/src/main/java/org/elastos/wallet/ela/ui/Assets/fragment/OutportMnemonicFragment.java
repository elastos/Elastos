package org.elastos.wallet.ela.ui.Assets.fragment;


import android.os.Bundle;
import android.support.v7.widget.AppCompatTextView;
import android.text.TextUtils;
import android.view.View;
import android.view.WindowManager;
import android.widget.EditText;
import android.widget.LinearLayout;
import android.widget.TextView;

import com.google.gson.JsonObject;
import com.google.gson.JsonParser;

import org.elastos.wallet.R;
import org.elastos.wallet.ela.base.BaseFragment;
import org.elastos.wallet.ela.db.table.Wallet;
import org.elastos.wallet.ela.rxjavahelp.BaseEntity;
import org.elastos.wallet.ela.rxjavahelp.NewBaseViewData;
import org.elastos.wallet.ela.ui.Assets.presenter.WallletManagePresenter;
import org.elastos.wallet.ela.ui.common.bean.CommmonBooleanEntity;
import org.elastos.wallet.ela.ui.common.bean.CommmonStringEntity;
import org.elastos.wallet.ela.utils.RxEnum;

import butterknife.BindView;
import butterknife.OnClick;

/**
 * 助记词
 */
public class OutportMnemonicFragment extends BaseFragment implements NewBaseViewData {


    @BindView(R.id.tv_title)
    TextView tvTitle;
    @BindView(R.id.tv_mnemonic)
    AppCompatTextView tvMnemonic;
    @BindView(R.id.et_pwd)
    EditText etPwd;
    @BindView(R.id.ll_pwd)
    LinearLayout llPwd;
    private String mnemonic;
    private Wallet wallet;
    private boolean hasPassPhrase;
    private WallletManagePresenter presenter;

    @Override
    protected int getLayoutId() {
        getBaseActivity().getWindow().setFlags(WindowManager.LayoutParams.FLAG_SECURE, WindowManager.LayoutParams.FLAG_SECURE);
        return R.layout.fragment_outport_mnemonic;
    }

    @Override
    protected void setExtraData(Bundle data) {
        mnemonic = data.getString("mnemonic");
        tvMnemonic.setText(mnemonic);
        wallet = data.getParcelable("wallet");
        presenter = new WallletManagePresenter();
        presenter.getMasterWalletBasicInfo(wallet.getWalletId(), this);
    }

    @Override
    protected void initView(View view) {
        tvTitle.setText(getString(R.string.outportMnemonic));

    }


    @OnClick({R.id.sb})
    public void onViewClicked(View view) {
        switch (view.getId()) {

            case R.id.sb:
                if (hasPassPhrase) {
                    String passphrase = etPwd.getText().toString().trim();
                    if (TextUtils.isEmpty(passphrase)) {
                        showToastMessage(getString(R.string.please_enter_your_mnemonic_password));
                        return;
                    }
                    presenter.verifyPrivateKey(wallet.getWalletId(), mnemonic, passphrase, this);
                } else {
                    Bundle bundle = new Bundle();
                    bundle.putInt("openType", RxEnum.MANAGER.ordinal());
                    bundle.putString("mnemonic", mnemonic);
                    start(VerifyMnemonicWordsFragment.class, bundle);
                }

                break;
        }
    }

    @Override
    public void onDestroy() {
        super.onDestroy();
        getBaseActivity().getWindow().clearFlags(WindowManager.LayoutParams.FLAG_SECURE);
    }


    @Override
    public void onGetData(String methodName, BaseEntity baseEntity, Object o) {

        switch (methodName) {
            case "verifyPrivateKey":
                boolean result = ((CommmonBooleanEntity) baseEntity).getData();
                if (result) {
                    Bundle bundle = new Bundle();
                    bundle.putInt("openType", RxEnum.MANAGER.ordinal());
                    bundle.putString("mnemonic", mnemonic);
                    start(VerifyMnemonicWordsFragment.class, bundle);
                } else {
                    showToastMessage(getString(R.string.error_20003));
                }
                break;
            case "getMasterWalletBasicInfo":
                String data = ((CommmonStringEntity) baseEntity).getData();
                JsonObject jsonData = new JsonParser().parse(data).getAsJsonObject();
                hasPassPhrase = jsonData.get("HasPassPhrase").getAsBoolean();
                if (hasPassPhrase) {
                    llPwd.setVisibility(View.VISIBLE);
                }
                break;
        }
    }
}
