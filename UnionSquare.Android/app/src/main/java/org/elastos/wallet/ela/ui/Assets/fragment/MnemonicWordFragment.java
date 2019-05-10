package org.elastos.wallet.ela.ui.Assets.fragment;


import android.os.Bundle;
import android.support.v7.widget.AppCompatTextView;
import android.support.v7.widget.Toolbar;
import android.text.TextUtils;
import android.view.View;
import android.widget.CompoundButton;
import android.widget.TextView;

import com.allen.library.SuperTextView;
import com.qmuiteam.qmui.layout.QMUILinearLayout;

import org.elastos.wallet.R;
import org.elastos.wallet.ela.ElaWallet.MyWallet;
import org.elastos.wallet.ela.base.BaseFragment;
import org.elastos.wallet.ela.bean.CreateWalletBean;
import org.elastos.wallet.ela.db.RealmUtil;
import org.elastos.wallet.ela.db.listener.RealmTransactionAbs;
import org.elastos.wallet.ela.db.table.SubWallet;
import org.elastos.wallet.ela.db.table.Wallet;
import org.elastos.wallet.ela.ui.Assets.presenter.CommonCreateSubWalletPresenter;
import org.elastos.wallet.ela.ui.Assets.presenter.MnemonicWordPresenter;
import org.elastos.wallet.ela.ui.Assets.viewdata.CommonCreateSubWalletViewData;
import org.elastos.wallet.ela.ui.Assets.viewdata.MnemonicWordViewData;
import org.elastos.wallet.ela.utils.AppUtlis;
import org.elastos.wallet.ela.utils.RxEnum;
import org.elastos.wallet.ela.utils.SPUtil;
import org.elastos.wallet.ela.widget.keyboard.SecurityEditText;

import butterknife.BindView;
import butterknife.OnClick;

/**
 * 助记词
 */
public class MnemonicWordFragment extends BaseFragment implements MnemonicWordViewData, CommonCreateSubWalletViewData {


    @BindView(R.id.toolbar_title)
    TextView toolbarTitle;
    @BindView(R.id.toolbar)
    Toolbar toolbar;
    @BindView(R.id.st_pws)
    SuperTextView stPws;
    @BindView(R.id.et_walletpws)
    SecurityEditText etWalletpws;
    @BindView(R.id.et_walletpws_next)
    SecurityEditText etWalletpwsNext;
    @BindView(R.id.ll_mnemonic_pws)
    QMUILinearLayout llMnemonicPws;
    @BindView(R.id.ll_mnemonic_word)
    QMUILinearLayout ll_mnemonic_word;
    public boolean Checked = false;
    @BindView(R.id.tv_mnemonic)
    AppCompatTextView tv_mnemonic;
    private CreateWalletBean createWalletBean;

    private MnemonicWordPresenter presenter;
    private RealmUtil realmUtil;


    @Override
    protected int getLayoutId() {
        return R.layout.fragment_mnemonic_word;
    }

    @Override
    protected void setExtraData(Bundle data) {
        createWalletBean = data.getParcelable("CreateWalletBean");

    }

    @Override
    protected void initInjector() {
    }

    @Override
    protected void initView(View view) {
        setToobar(toolbar, toolbarTitle, getString(R.string.mnemonic));
        //开启自定义键盘
       // AppUtlis.securityKeyboard(ll_mnemonic_word);
        //开关
        stPws.setOnSuperTextViewClickListener(new SuperTextView.OnSuperTextViewClickListener() {
            @Override
            public void onClickListener(SuperTextView superTextView) {
                superTextView.setSwitchIsChecked(!superTextView.getSwitchIsChecked());
            }
        }).setSwitchCheckedChangeListener(new SuperTextView.OnSwitchCheckedChangeListener() {
            @Override
            public void onCheckedChanged(CompoundButton buttonView, boolean isChecked) {
                Checked = isChecked;
                if (isChecked) {
                    llMnemonicPws.setVisibility(View.VISIBLE);
                } else {
                    llMnemonicPws.setVisibility(View.GONE);
                }
            }
        });

        String type = (1 == new SPUtil(getContext()).getLanguage()) ? "english" : "chinese";
        presenter = new MnemonicWordPresenter();
        //创建助记词
        presenter.generateMnemonic(type, this);


    }


    @OnClick({R.id.st_pws, R.id.sb_create_wallet})
    public void onViewClicked(View view) {
        switch (view.getId()) {
            case R.id.st_pws:
              /*  if (llMnemonicPws.getVisibility() == View.GONE) {
                    llMnemonicPws.setVisibility(View.VISIBLE);
                } else {
                    llMnemonicPws.setVisibility(View.GONE);
                }*/

                break;
            case R.id.sb_create_wallet:
                //选了助记词密码后的操作
                String pws = "";
                if (Checked) {
                    pws = etWalletpws.getText().toString().trim();
                    String pws_next = etWalletpwsNext.getText().toString().trim();
                    if (TextUtils.isEmpty(tv_mnemonic.getText().toString().trim())) {

                        return;
                    }
                    if (TextUtils.isEmpty(pws)) {
                        showToast(getString(R.string.please_enter_your_mnemonic_password));
                        return;
                    }

                    if (TextUtils.isEmpty(pws_next)) {
                        showToast(getString(R.string.inputWalltPwdAgin));
                        return;
                    }

                    if (!AppUtlis.chenckString(pws_next)) {
                        showToast(getString(R.string.mmgsbd));
                        return;
                    }
                    if (!pws.equals(pws_next)) {
                        showToast(getString(R.string.lcmmsrbyz));
                        return;
                    }
                }
                createWalletBean.setPhrasePassword(pws);
                realmUtil = new RealmUtil();

                presenter.createMasterWallet(createWalletBean.getMasterWalletID(), createWalletBean.getMnemonic(), createWalletBean.getPhrasePassword(),
                        createWalletBean.getPayPassword(), createWalletBean.getSingleAddress(), this);
                break;
        }
    }

    private void toNextPager() {
        Bundle bundle = new Bundle();
        bundle.putString("mnemonic", createWalletBean.getMnemonic());
        start(VerifyMnemonicWordsFragment.class, bundle);
    }


    //正则去掉逗号 括号
    private String regex(String at) {
        return at.replaceAll("\\[", "").
                replaceAll("\\]", "").replaceAll(",", "");
    }


    @Override
    public void onGetMneonic(String mnemonic) {
        if (mnemonic == null) {
            showToastMessage("助记词生成失败");
            return;
        }
        createWalletBean.setMnemonic(mnemonic);
        String masterWalletID = AppUtlis.getStringRandom(8);
        createWalletBean.setMasterWalletID(masterWalletID);
      //  String[] re = mnemonic.split(" ");//用split()函数直接分割
        tv_mnemonic.setText(mnemonic);
        /*tv_mnemonic.setText(regex(Arrays.toString(Arrays.copyOfRange(re, 0, 8))) + "\n\n" +
                regex(Arrays.toString(Arrays.copyOfRange(re, 8, re.length))));*/

    }

    @Override
    public void onCreateMasterWallet(String baseInfo) {
        if (baseInfo != null) {
            new CommonCreateSubWalletPresenter().createSubWallet(createWalletBean.getMasterWalletID(), MyWallet.ELA, this);

        }
    }

    @Override
    public void onCreateSubWallet(String data) {
        if (data != null) {
            //创建Mainchain子钱包

            Wallet masterWallet = new Wallet();
            masterWallet.setWalletName(createWalletBean.getMasterWalletName());
            masterWallet.setWalletId(createWalletBean.getMasterWalletID());
            masterWallet.setSingleAddress(createWalletBean.getSingleAddress());
            realmUtil.updateWalletDetial(masterWallet);
            SubWallet subWallet = new SubWallet();
            subWallet.setBelongId(createWalletBean.getMasterWalletID());
            subWallet.setChainId(data);
            realmUtil.updateSubWalletDetial(subWallet, new RealmTransactionAbs() {
                @Override
                public void onSuccess() {
                    realmUtil.updateWalletDefault(createWalletBean.getMasterWalletID(), new RealmTransactionAbs() {
                        @Override
                        public void onSuccess() {
                            post(RxEnum.ONE.ordinal(), null, masterWallet);
                            toNextPager();
                        }
                    });
                }
            });


        }
    }
}
