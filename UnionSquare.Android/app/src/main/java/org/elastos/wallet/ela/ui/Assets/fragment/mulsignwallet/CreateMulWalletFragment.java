package org.elastos.wallet.ela.ui.Assets.fragment.mulsignwallet;

import android.os.Bundle;
import android.support.v7.widget.RecyclerView;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.CheckBox;
import android.widget.CompoundButton;
import android.widget.ImageView;
import android.widget.LinearLayout;
import android.widget.TextView;

import org.elastos.wallet.R;
import org.elastos.wallet.ela.base.BaseFragment;
import org.elastos.wallet.ela.bean.BusEvent;
import org.elastos.wallet.ela.bean.CreateWalletBean;
import org.elastos.wallet.ela.utils.ClearEditText;
import org.elastos.wallet.ela.utils.Log;
import org.elastos.wallet.ela.utils.RxEnum;
import org.greenrobot.eventbus.Subscribe;
import org.greenrobot.eventbus.ThreadMode;

import butterknife.BindView;
import butterknife.ButterKnife;
import butterknife.OnClick;
import butterknife.Unbinder;

public class CreateMulWalletFragment extends BaseFragment implements CompoundButton.OnCheckedChangeListener {

    @BindView(R.id.tv_title)
    TextView tvTitle;
    @BindView(R.id.ll_addmainkey)
    LinearLayout llAddmainkey;
    @BindView(R.id.et_walletname)
    ClearEditText etWalletname;
    @BindView(R.id.tv_signnum)
    TextView tvSignnum;
    @BindView(R.id.cb)
    CheckBox cb;
    @BindView(R.id.rv)
    RecyclerView rv;
    @BindView(R.id.iv_status)
    ImageView ivStatus;
    @BindView(R.id.tv_status)
    TextView tvStatus;
    Unbinder unbinder;
    private CreateWalletBean createWalletBean;


    @Override
    protected int getLayoutId() {
        return R.layout.fragment_create_mul;
    }

    @Override
    protected void initView(View view) {
        tvTitle.setText(getString(R.string.createmulwalllet));
        cb.setOnCheckedChangeListener(this);
        registReceiver();
    }

    @OnClick({R.id.ll_addmainkey, R.id.tv_create, R.id.iv_add, R.id.iv_chosenum})
    public void onViewClick(View view) {
        switch (view.getId()) {
            case R.id.ll_addmainkey:
                start(MainPrivateKeyFragment.class);
                break;
            case R.id.tv_create:
                break;
            case R.id.iv_add:
                break;
            case R.id.iv_chosenum:

                break;
        }
    }


    @Override
    public void onCheckedChanged(CompoundButton buttonView, boolean isChecked) {
        if (isChecked) {
            llAddmainkey.setEnabled(false);
            llAddmainkey.setAlpha(0.5f);
        } else {
            llAddmainkey.setEnabled(true);
            llAddmainkey.setAlpha(1);
        }
    }

    @Subscribe(threadMode = ThreadMode.MAIN)
    public void Event(BusEvent result) {
        int integer = result.getCode();
        if (integer == RxEnum.CREATEPRIVATEKEY.ordinal()) {
            //添加主私钥额回调
            createWalletBean = (CreateWalletBean) result.getObj();
        }
        if (integer == RxEnum.IMPORTRIVATEKEY.ordinal()) {
            //导入助记词回调
            createWalletBean = (CreateWalletBean) result.getObj();
        }
        if (integer == RxEnum.SELECTRIVATEKEY.ordinal()) {
            //选择已有钱包回调
            setMainPrivaKeyStatus();
            createWalletBean = (CreateWalletBean) result.getObj();

        }

        Log.i("?????", createWalletBean.toString());

    }

    private void setMainPrivaKeyStatus() {
        ivStatus.setImageResource(R.mipmap.asset_adding_select);
        tvStatus.setText(getString(R.string.editmainprivatekey));
    }


}
