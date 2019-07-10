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
import org.elastos.wallet.ela.ui.common.viewdata.CommmonStringWithMethNameViewData;
import org.elastos.wallet.ela.utils.DialogUtil;
import org.elastos.wallet.ela.utils.RxEnum;
import org.elastos.wallet.ela.utils.listener.WarmPromptListener;
import org.greenrobot.eventbus.Subscribe;
import org.greenrobot.eventbus.ThreadMode;

import java.util.List;

import butterknife.BindView;
import butterknife.OnClick;
import butterknife.Unbinder;

public class WallletManageFragment extends BaseFragment implements WarmPromptListener, WalletManageViewData, CommmonStringWithMethNameViewData {

    private static final String DELETE = "delete";

    private static final String OUTPORTMN = "outportmm";
    @BindView(R.id.tv_title)
    TextView tvTitle;
    Unbinder unbinder;
    @BindView(R.id.tv_updatename)
    TextView tvUpdatename;
    @BindView(R.id.ll_updatename)
    LinearLayout llUpdatename;
    @BindView(R.id.ll_updatepwd)
    LinearLayout llUpdatepwd;
    @BindView(R.id.ll_exportkeystore)
    LinearLayout llExportkeystore;
    @BindView(R.id.ll_exportmnemonic)
    LinearLayout llExportmnemonic;
    @BindView(R.id.ll_sign)
    LinearLayout llSign;
    @BindView(R.id.ll_exportreadonly)
    LinearLayout llExportreadonly;
    @BindView(R.id.ll_showmulpublickey)
    LinearLayout llShowmulpublickey;
    Unbinder unbinder1;
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
            tvUpdatename.setText(wallet.getWalletName());
    }

    @Override
    protected void initView(View view) {
        tvTitle.setText(R.string.walletmanage);
        dialogUtil = new DialogUtil();
        presenter = new WallletManagePresenter();
        registReceiver();
        switch (wallet.getType()) {
            //0 普通单签 1单签只读 2普通多签 3多签只读
            case 0:

                break;
            case 1:
                llUpdatepwd.setVisibility(View.GONE);
                llExportmnemonic.setVisibility(View.GONE);
                break;
            case 2:
                break;
            case 3:
                break;
        }
    }

    @OnClick({R.id.tv_delete, R.id.ll_updatename, R.id.ll_updatepwd, R.id.ll_exportkeystore, R.id.ll_exportmnemonic,
            R.id.ll_sign, R.id.ll_exportreadonly, R.id.ll_showmulpublickey})
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
            case R.id.ll_updatename:
                //修改钱包名称
                bundle = new Bundle();
                bundle.putString("walletId", wallet.getWalletId());
                start(WalletUpdateName.class, bundle);
                break;
            case R.id.ll_updatepwd:
                //修改密码
                bundle = new Bundle();
                bundle.putString("walletId", wallet.getWalletId());
                start(WalletUpdataPwdFragment.class, bundle);
                break;
            case R.id.ll_exportkeystore:
                //导出keystore
                bundle = new Bundle();
                bundle.putParcelable("wallet", wallet);
                start(OutportKeystoreFragment.class, bundle);
                break;
            case R.id.ll_exportmnemonic:
                //导出助记词弹框
                dialog = dialogUtil.showWarmPromptInput(getBaseActivity(), null, null, this);
                dialogAction = OUTPORTMN;
                break;
            case R.id.ll_sign:
                //签名
                bundle = new Bundle();
                bundle.putParcelable("wallet", wallet);
                start(SignTransactionFragment.class, bundle);
                break;
            case R.id.ll_exportreadonly:
                //导出只读
                bundle = new Bundle();
                bundle.putParcelable("wallet", wallet);
                start(ExportReadOnlyFragment.class, bundle);
                break;
            case R.id.ll_showmulpublickey:
                //查看多签公钥
                bundle = new Bundle();
                bundle.putParcelable("wallet", wallet);
                start(ShowMulsignPublicKeyFragment.class, bundle);
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
            //删除钱包  用来验证密码
            presenter.exportWalletWithMnemonic(wallet.getWalletId(), pwd, this);
        }
    }

    @Subscribe(threadMode = ThreadMode.MAIN)
    public void Event(BusEvent result) {
        int integer = result.getCode();
        if (integer == RxEnum.UPDATA_WALLET_NAME.ordinal()) {
            wallet.setWalletName(result.getName());
            tvUpdatename.setText(result.getName());

        }
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

    @Override
    public void onGetCommonData(String methodname, String data) {
        //exportWalletWithMnemonic
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


}
