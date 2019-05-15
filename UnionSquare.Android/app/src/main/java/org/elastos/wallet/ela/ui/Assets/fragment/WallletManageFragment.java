package org.elastos.wallet.ela.ui.Assets.fragment;

import android.app.Dialog;
import android.os.Bundle;
import android.text.TextUtils;
import android.view.View;
import android.widget.EditText;
import android.widget.LinearLayout;
import android.widget.TextView;

import org.elastos.wallet.R;
import org.elastos.wallet.ela.base.BaseFragment;
import org.elastos.wallet.ela.bean.BusEvent;
import org.elastos.wallet.ela.db.RealmUtil;
import org.elastos.wallet.ela.db.table.Wallet;
import org.elastos.wallet.ela.ui.Assets.presenter.WallletManagePresenter;
import org.elastos.wallet.ela.ui.Assets.viewdata.WalletManageViewData;
import org.elastos.wallet.ela.utils.DialogUtil;
import org.elastos.wallet.ela.utils.RxEnum;
import org.elastos.wallet.ela.utils.listener.WarmPromptListener;
import org.greenrobot.eventbus.Subscribe;
import org.greenrobot.eventbus.ThreadMode;

import java.util.List;

import butterknife.BindView;
import butterknife.OnClick;
import butterknife.Unbinder;

public class WallletManageFragment extends BaseFragment implements WarmPromptListener, WalletManageViewData {

    private static final String DELETE = "delete";

    private static final String OUTPORTMN = "outportmm";
    @BindView(R.id.tv_title)
    TextView tvTitle;
    @BindView(R.id.tv_1)
    TextView tv1;
    @BindView(R.id.ll_1)
    LinearLayout ll1;
    @BindView(R.id.ll_2)
    LinearLayout ll2;
    @BindView(R.id.ll_3)
    LinearLayout ll3;
    @BindView(R.id.ll_4)
    LinearLayout ll4;

    Unbinder unbinder;
    private DialogUtil dialogUtil;
    String dialogAction = null;
    private Dialog dialog;
    private Wallet wallet;
    private WallletManagePresenter presenter;

    @Override
    protected int getLayoutId() {
        return R.layout.fragment_wallet_manage;
    }

    @Override
    protected void setExtraData(Bundle data) {
        super.setExtraData(data);
        wallet = data.getParcelable("wallet");
        if (wallet != null)
            tv1.setText(wallet.getWalletName());
    }

    @Override
    protected void initView(View view) {
        tvTitle.setText(R.string.walletmanage);
        dialogUtil = new DialogUtil();
        presenter = new WallletManagePresenter();
        registReceiver();
    }

    @OnClick({R.id.tv_delete, R.id.ll_1, R.id.ll_2, R.id.ll_3, R.id.ll_4})
    public void onViewClicked(View view) {
        Bundle bundle = null;
        dialogAction = null;
        switch (view.getId()) {
            case R.id.tv_delete:
                //删除钱包弹框
                dialogUtil.showWarmPrompt1(getBaseActivity(), getString(R.string.deletewallletornot), new WarmPromptListener() {
                    @Override
                    public void affireBtnClick(View view) {
                        dialog = dialogUtil.showWarmPromptInput(getBaseActivity(), null, null, WallletManageFragment.this);
                        dialogAction = DELETE;
                    }
                });
                break;
            case R.id.ll_1:
                //修改钱包名称
                bundle = new Bundle();
                bundle.putString("walletId", wallet.getWalletId());
                start(WalletUpdateName.class, bundle);
                break;
            case R.id.ll_2:
                //修改密码
                bundle = new Bundle();
                bundle.putString("walletId", wallet.getWalletId());
                start(WalletUpdataPwdFragment.class, bundle);
                break;
            case R.id.ll_3:
                //导出keystore
                bundle = new Bundle();
                bundle.putParcelable("wallet", wallet);
                start(OutportKeystoreFragment.class, bundle);
                break;
            case R.id.ll_4:
                //导出助记词弹框
                dialog = dialogUtil.showWarmPromptInput(getBaseActivity(), null, null, this);
                dialogAction = OUTPORTMN;
                break;

        }
    }


    @Override
    public void affireBtnClick(View view) {
//这里只见他showWarmPromptInput的确认
        String pwd = ((EditText) view).getText().toString().trim();
        if (TextUtils.isEmpty(pwd)) {
            showToastMessage(getString(R.string.pwdnoempty));
            return;
        }

        if (OUTPORTMN.equals(dialogAction)) {
            //导出助记词
            presenter.exportWalletWithMnemonic(wallet.getWalletId(), pwd, this);

        } else if (DELETE.equals(dialogAction)) {
            //删除钱包
            presenter.exportWalletWithMnemonic(wallet.getWalletId(), pwd, this);
        }
    }

    @Subscribe(threadMode = ThreadMode.MAIN)
    public void Event(BusEvent result) {
        int integer = result.getCode();
        if (integer == RxEnum.UPDATA_WALLET_NAME.ordinal()) {
            wallet.setWalletName(result.getName());
            tv1.setText(result.getName());

        }
    }

    @Override
    public void onOutportMnemonic(String data) {
        dialog.dismiss();
        if (DELETE.equals(dialogAction)) {
            //删除的回调
            presenter.destroyWallet(wallet.getWalletId(), this);
            return;
        }
        Bundle bundle = new Bundle();
        bundle.putString("mnemonic", data);
        start(OutportMnemonicFragment.class, bundle);
    }

    @Override
    public void onDestoryWallet(String data) {
        dialog.dismiss();
        RealmUtil realmUtil = new RealmUtil();
        realmUtil.deleteWallet(wallet.getWalletId());//删除钱包和其子钱包
        // realmUtil.deleteSubWallet(wallet.getWalletId());
        List<Wallet> wallets = realmUtil.queryUserAllWallet();
        if (wallets == null || wallets.size() == 0) {
            //没有其他钱包了
            toHomeWalletFragment();
            return;
        }
        post(RxEnum.DELETE.ordinal(), null, wallet.getWalletId());
        showToastMessage(getString(R.string.deletewalletsucess));
        popBackFragment();
    }
}
