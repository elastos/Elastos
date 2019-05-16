package org.elastos.wallet.ela.ui.Assets.fragment;


import android.os.Bundle;
import android.support.v4.widget.NestedScrollView;
import android.support.v7.widget.AppCompatEditText;
import android.text.TextUtils;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.CheckBox;
import android.widget.CompoundButton;

import com.allen.library.SuperTextView;
import com.qmuiteam.qmui.layout.QMUIRelativeLayout;

import org.elastos.wallet.R;
import org.elastos.wallet.ela.ElaWallet.MyWallet;
import org.elastos.wallet.ela.base.BaseFragment;
import org.elastos.wallet.ela.db.RealmUtil;
import org.elastos.wallet.ela.db.listener.RealmTransactionAbs;
import org.elastos.wallet.ela.db.table.SubWallet;
import org.elastos.wallet.ela.db.table.Wallet;
import org.elastos.wallet.ela.ui.Assets.presenter.CommonCreateSubWalletPresenter;
import org.elastos.wallet.ela.ui.Assets.presenter.ImportMnemonicPresenter;
import org.elastos.wallet.ela.ui.Assets.viewdata.CommonCreateSubWalletViewData;
import org.elastos.wallet.ela.ui.Assets.viewdata.ImportMnemonicViewData;
import org.elastos.wallet.ela.utils.AppUtlis;
import org.elastos.wallet.ela.utils.ClearEditText;
import org.elastos.wallet.ela.utils.RxEnum;

import butterknife.BindView;
import butterknife.ButterKnife;
import butterknife.OnClick;
import butterknife.Unbinder;

/**
 * 助记词导入
 */
public class ImportMnemonicFragment extends BaseFragment implements ImportMnemonicViewData, CommonCreateSubWalletViewData {


    @BindView(R.id.et_mnemonic)
    AppCompatEditText etMnemonic;
    @BindView(R.id.et_walletname)
    ClearEditText etWalletname;
    @BindView(R.id.et_walletpws)
    ClearEditText etWalletpws;
    @BindView(R.id.et_walletpws_agin)
    ClearEditText etWalletpwsAgin;
    @BindView(R.id.cb)
    CheckBox cb;
    @BindView(R.id.st_pws)
    SuperTextView stPws;
    @BindView(R.id.et_mnemonic_pwd)
    ClearEditText etMnemonicPwd;
    @BindView(R.id.qrl_pws)
    QMUIRelativeLayout qrlPws;
    Unbinder unbinder;

    ImportMnemonicPresenter importMnemonicPresenter;
    @BindView(R.id.nsv)
    NestedScrollView nsv;
    Unbinder unbinder1;
    private String walletName;
    private String mnemonic;
    private String masterWalletID;

    @Override
    protected int getLayoutId() {
        return R.layout.fragment_import_mnemonic;
    }


    @Override
    protected void initView(View view) {
        stPws.setOnSuperTextViewClickListener(new SuperTextView.OnSuperTextViewClickListener() {
            @Override
            public void onClickListener(SuperTextView superTextView) {
                superTextView.setSwitchIsChecked(!superTextView.getSwitchIsChecked());
            }
        }).setSwitchCheckedChangeListener(new SuperTextView.OnSwitchCheckedChangeListener() {
            @Override
            public void onCheckedChanged(CompoundButton buttonView, boolean isChecked) {
                if (isChecked) {
                    qrlPws.setVisibility(View.VISIBLE);
                } else {
                    qrlPws.setVisibility(View.GONE);
                }
            }
        });
        importMnemonicPresenter = new ImportMnemonicPresenter();
        view.setOnClickListener(new View.OnClickListener() {
                                    @Override
                                    public void onClick(View v) {
                                        Log.i("feafs", "ewtewt");
                                    }
                                }
        );
    /*    nsv.setOnTouchListener(new View.OnTouchListener() {
            @Override
            public boolean onTouch(View v, MotionEvent event) {
                Log.i("feafs", "fsf");
                InputMethodManager inputmanger = (InputMethodManager) getActivity().getSystemService(Context.INPUT_METHOD_SERVICE);
                if (inputmanger.isActive()) {
                    if (view != null) {
                        View view = getActivity().getWindow().peekDecorView();
                        inputmanger.hideSoftInputFromWindow(view.getWindowToken(), 0);
                    }
                }
                return false;
            }
        });
      //  get*/
    }


    @OnClick({R.id.sb_sure})
    public void onViewClicked(View view) {
        switch (view.getId()) {
            case R.id.sb_sure:
                //确认导入钱包

                mnemonic = etMnemonic.getText().toString().trim();
                if (TextUtils.isEmpty(mnemonic)) {
                    showToastMessage(getString(R.string.mnemonicinputwrong));
                    return;
                }
                walletName = etWalletname.getText().toString().trim();
                if (TextUtils.isEmpty(walletName)) {
                    showToastMessage(getString(R.string.walletnamenotnull));
                    return;
                }
              /*  if (!MatcherUtil.isRightName(walletName)) {
                    showToastMessage(getString(R.string.walletnameruler));
                    return;
                }*/
                String payPassword = etWalletpws.getText().toString().trim();
                if (TextUtils.isEmpty(payPassword)) {
                    showToastMessage(getString(R.string.pwdnoempty));
                    return;
                }
                String walletPwdAgin = etWalletpwsAgin.getText().toString().trim();

                if (TextUtils.isEmpty(walletPwdAgin)) {
                    showToastMessage(getString(R.string.aginpwdnotnull));
                    return;
                }
                if (!payPassword.equals(walletPwdAgin)) {
                    showToastMessage(getString(R.string.keynotthesame));
                    return;
                }
                String phrasePassword = etMnemonicPwd.getText().toString().trim();
                masterWalletID = AppUtlis.getStringRandom(8);
                importMnemonicPresenter.importWalletWithMnemonic(masterWalletID, mnemonic, phrasePassword,
                        payPassword, cb.isChecked(), this);
                break;

        }
    }

    @Override
    public void onImportMnemonic(String data) {
        new CommonCreateSubWalletPresenter().createSubWallet(masterWalletID, MyWallet.ELA, this);


    }

    @Override
    public void onCreateSubWallet(String data) {
        RealmUtil realmUtil = new RealmUtil();
        if (data != null) {
            //创建Mainchain子钱包

            Wallet masterWallet = new Wallet();
            masterWallet.setWalletName(walletName);
            masterWallet.setWalletId(masterWalletID);
            masterWallet.setSingleAddress(cb.isChecked());
            realmUtil.updateWalletDetial(masterWallet);

            SubWallet subWallet = new SubWallet();
            subWallet.setBelongId(masterWalletID);
            subWallet.setChainId(data);
            realmUtil.updateSubWalletDetial(subWallet, new RealmTransactionAbs() {
                @Override
                public void onSuccess() {
                    realmUtil.updateWalletDefault(masterWalletID, new RealmTransactionAbs() {
                        @Override
                        public void onSuccess() {
                            post(RxEnum.ONE.ordinal(), null, masterWallet);
                            toMainFragment();
                        }
                    });
                }
            });


        }
    }

    @Override
    public View onCreateView(LayoutInflater inflater, ViewGroup container, Bundle savedInstanceState) {
        // TODO: inflate a fragment view
        View rootView = super.onCreateView(inflater, container, savedInstanceState);
        unbinder1 = ButterKnife.bind(this, rootView);
        return rootView;
    }

    @Override
    public void onDestroyView() {
        super.onDestroyView();
        unbinder1.unbind();
    }
}
