package org.elastos.wallet.ela.ui.vote.signupfor;


import android.content.Intent;
import android.os.Bundle;
import android.text.TextUtils;
import android.view.View;
import android.widget.EditText;
import android.widget.TextView;

import com.blankj.utilcode.util.ToastUtils;

import org.elastos.wallet.R;
import org.elastos.wallet.ela.ElaWallet.MyWallet;
import org.elastos.wallet.ela.base.BaseFragment;
import org.elastos.wallet.ela.bean.BusEvent;
import org.elastos.wallet.ela.db.RealmUtil;
import org.elastos.wallet.ela.db.table.Wallet;
import org.elastos.wallet.ela.ui.Assets.bean.BalanceEntity;
import org.elastos.wallet.ela.ui.Assets.presenter.CommonGetBalancePresenter;
import org.elastos.wallet.ela.ui.Assets.presenter.TransferPresenter;
import org.elastos.wallet.ela.ui.Assets.viewdata.CommonBalanceViewData;
import org.elastos.wallet.ela.ui.common.viewdata.CommmonLongViewData;
import org.elastos.wallet.ela.ui.common.viewdata.CommmonStringWithMethNameViewData;
import org.elastos.wallet.ela.ui.vote.activity.VoteTransferActivity;
import org.elastos.wallet.ela.ui.vote.bean.Area;
import org.elastos.wallet.ela.ui.vote.fragment.AreaCodeFragment;
import org.elastos.wallet.ela.utils.ClearEditText;
import org.elastos.wallet.ela.utils.Constant;
import org.elastos.wallet.ela.utils.DialogUtil;
import org.elastos.wallet.ela.utils.RxEnum;
import org.elastos.wallet.ela.utils.SPUtil;
import org.elastos.wallet.ela.utils.klog.KLog;
import org.elastos.wallet.ela.utils.listener.WarmPromptListener;
import org.greenrobot.eventbus.EventBus;
import org.greenrobot.eventbus.Subscribe;
import org.greenrobot.eventbus.ThreadMode;

import butterknife.BindView;
import butterknife.OnClick;

/**
 * 报名参选
 */
public class SignUpForFragment extends BaseFragment implements CommmonStringWithMethNameViewData, WarmPromptListener, CommonBalanceViewData, CommmonLongViewData {


    @BindView(R.id.tv_title)
    TextView tvTitle;
    @BindView(R.id.et_dotname)
    EditText etDotname;
    @BindView(R.id.et_publickey)
    ClearEditText etPublickey;
    @BindView(R.id.tv_area)
    TextView tvArea;
    @BindView(R.id.et_url)
    EditText etUrl;
    @BindView(R.id.et_net)
    EditText et_net;
    private RealmUtil realmUtil = new RealmUtil();
    private Wallet wallet = realmUtil.queryDefauleWallet();
    SignUpPresenter presenter;
    @BindView(R.id.et_walletkey)
    EditText et_walletkey;
    private DialogUtil dialogUtil = new DialogUtil();
    private TransferPresenter transferpresenter = new TransferPresenter();


    @Override
    protected int getLayoutId() {
        return R.layout.fragment_sign_up_for;
    }

    @Override
    protected void initInjector() {

    }

    @Override
    protected void initView(View view) {
        tvTitle.setText(R.string.sign_up_for);
        presenter = new SignUpPresenter();
        //获取公钥
        presenter.getPublicKeyForVote(wallet.getWalletId(), MyWallet.ELA, this);
        EventBus.getDefault().register(this);
    }

    public static SignUpForFragment newInstance() {
        Bundle args = new Bundle();
        SignUpForFragment fragment = new SignUpForFragment();
        fragment.setArguments(args);
        return fragment;
    }

    String name, publickey, walletkey, net, area, url, pwd;

    @OnClick({R.id.tv_sure, R.id.ll_area})
    public void onViewClicked(View view) {
        switch (view.getId()) {
            case R.id.tv_sure:
                name = etDotname.getText().toString().trim();//节点名称
                publickey = etPublickey.getText().toString().trim();//节点公钥
                walletkey = et_walletkey.getText().toString().trim();//钱包公钥
                net = et_net.getText().toString().trim();//网络地址
                area = tvArea.getText().toString().trim();//国家地址
                url = etUrl.getText().toString().trim();//官网

                if (TextUtils.isEmpty(name)) {
                    ToastUtils.showShort(getString(R.string.inputdotname));
                    return;
                }
                if (TextUtils.isEmpty(publickey)) {
                    ToastUtils.showShort(getString(R.string.inputdotpublickey));
                    return;
                }

                if (TextUtils.isEmpty(datakey)) {
                    ToastUtils.showShort(getString(R.string.walletkey));
                    return;
                }

                /*if (TextUtils.isEmpty(net)) {
                    ToastUtils.showShort(getString(R.string.network_address));
                    return;
                }*/
                if (TextUtils.isEmpty(area)) {
                    ToastUtils.showShort(getString(R.string.countryregion_cannot_be_empty));
                    return;
                }
                if (TextUtils.isEmpty(url)) {
                    ToastUtils.showShort(getString(R.string.the_official_website_cannot_be_empty));
                    return;
                }
               /* if (!AppUtlis.urlReg(url)) {
                    ToastUtils.showShort(getString(R.string.please_enter_the_correct_url));
                    return;
                }*/

                //查询余额
                new CommonGetBalancePresenter().getBalance(wallet.getWalletId(), MyWallet.ELA, 2,this);
                break;
            case R.id.ll_area:
                //选择国家和地区
                start(new AreaCodeFragment());
                break;
        }
    }

    long code;

    @Subscribe(threadMode = ThreadMode.MAIN)
    public void Event(BusEvent result) {
        int integer = result.getCode();
        if (integer == RxEnum.AREA.ordinal()) {
            Area area = (Area) result.getObj();
            code = area.getCode();
            int Language = new SPUtil(getContext()).getLanguage();
            String name;
            if (Language == 0) {
                name = area.getZh();
            } else {
                name = area.getEn();
            }
            tvArea.setText(name);
        } else if (integer == RxEnum.TRANSFERSUCESS.ordinal()) {
            new DialogUtil().showTransferSucess(getBaseActivity(), new WarmPromptListener() {
                @Override
                public void affireBtnClick(View view) {
                    popBackFragment();
                }
            });
        }
    }

    String attributes, datakey;

    @Override
    public void onGetCommonData(String methodname, String data) {

        switch (methodname) {
            //获取钱包公钥
            case "getPublicKeyForVote":
                //KLog.a(data);
                this.datakey = data;
                et_walletkey.setText(data);
                etPublickey.setText(data);
                break;
            //验证密码
            case "exportWalletWithMnemonic":

                presenter.getGenerateProducerPayload(wallet.getWalletId(), MyWallet.ELA, datakey, publickey, name, url, net, code, pwd, this);
                break;
            //验证交易
            case "payload":
                KLog.a(data);
                presenter.createRegisterProducerTransaction(wallet.getWalletId(), MyWallet.ELA, "", data, MyWallet.RATE * 5000, "", false, this);
                break;
            //创建交易
            case "createRegisterProducerTransaction":
                attributes = data;
                //计算手续费
                transferpresenter.calculateTransactionFee(wallet.getWalletId(), MyWallet.ELA, data, MyWallet.feePerKb, this);
                KLog.a(data);
                dialogUtil.dialogDismiss();
                break;
        }
    }


    @Override
    public void affireBtnClick(View view) {
        pwd = ((EditText) view).getText().toString().trim();
        if (TextUtils.isEmpty(pwd)) {
            showToastMessage(getString(R.string.pwdnoempty));
            return;
        }

        presenter.exportWalletWithMnemonic(wallet.getWalletId(), pwd, this);
    }

    @Override
    public void onBalance(BalanceEntity data) {
        if (Long.parseLong(data.getBalance()) / MyWallet.RATE <= 5000) {
            ToastUtils.showShort(R.string._5000);
        } else {
            dialogUtil.showWarmPromptInput(getBaseActivity(), null, null, this);
        }
    }

    @Override
    public void onGetCommonData(long fee) {
        Intent intent = new Intent(getActivity(), VoteTransferActivity.class);
        intent.putExtra("wallet", wallet);
        intent.putExtra("type", Constant.TRANFER);
        intent.putExtra("amount", "5000");
        intent.putExtra("chainId", MyWallet.ELA);
        intent.putExtra("toAddress", publickey);
        intent.putExtra("pwd", pwd);
        intent.putExtra("fee", fee);
        intent.putExtra("attributes", attributes);
        startActivity(intent);
    }

}
