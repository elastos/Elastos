package org.elastos.wallet.ela.ui.crvote.fragment;


import android.content.Intent;
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
import org.elastos.wallet.ela.rxjavahelp.BaseEntity;
import org.elastos.wallet.ela.rxjavahelp.NewBaseViewData;
import org.elastos.wallet.ela.ui.Assets.bean.BalanceEntity;
import org.elastos.wallet.ela.ui.Assets.presenter.WallletManagePresenter;
import org.elastos.wallet.ela.ui.Assets.viewdata.CommonBalanceViewData;
import org.elastos.wallet.ela.ui.common.bean.CommmonStringEntity;
import org.elastos.wallet.ela.ui.common.bean.CommmonStringWithiMethNameEntity;
import org.elastos.wallet.ela.ui.crvote.presenter.CRSignUpPresenter;
import org.elastos.wallet.ela.ui.crvote.presenter.CRlistPresenter;
import org.elastos.wallet.ela.ui.vote.activity.VoteTransferActivity;
import org.elastos.wallet.ela.ui.vote.bean.Area;
import org.elastos.wallet.ela.ui.vote.fragment.AreaCodeFragment;
import org.elastos.wallet.ela.utils.Arith;
import org.elastos.wallet.ela.utils.Constant;
import org.elastos.wallet.ela.utils.DialogUtil;
import org.elastos.wallet.ela.utils.RxEnum;
import org.elastos.wallet.ela.utils.SPUtil;
import org.elastos.wallet.ela.utils.listener.WarmPromptListener;
import org.greenrobot.eventbus.EventBus;
import org.greenrobot.eventbus.Subscribe;
import org.greenrobot.eventbus.ThreadMode;

import butterknife.BindView;
import butterknife.OnClick;

/**
 * 报名参选
 */
public class CRSignUpForFragment extends BaseFragment implements NewBaseViewData, WarmPromptListener, CommonBalanceViewData {
    @BindView(R.id.tv_title)
    TextView tvTitle;
    @BindView(R.id.tv_publickey)
    TextView tvPublickey;
    @BindView(R.id.tv_did)
    TextView tvDid;
    @BindView(R.id.et_dotname)
    EditText etDotname;
    @BindView(R.id.tv_area)
    TextView tvArea;
    @BindView(R.id.et_url)
    EditText etUrl;
    private RealmUtil realmUtil = new RealmUtil();
    private Wallet wallet = realmUtil.queryDefauleWallet();
    CRSignUpPresenter presenter;

    private DialogUtil dialogUtil = new DialogUtil();


    @Override
    protected int getLayoutId() {
        return R.layout.fragment_cr_signup;
    }

    @Override
    protected void initInjector() {

    }

    @Override
    protected void initView(View view) {
        tvTitle.setText(R.string.sign_up_for);
        presenter = new CRSignUpPresenter();
        //获取公钥
        new CRlistPresenter().getCROwnerPublicKey(wallet.getWalletId(), MyWallet.ELA, this);
        presenter.getCROwnerDID(wallet.getWalletId(), MyWallet.ELA, this);
        EventBus.getDefault().register(this);
    }


    String name, publickey, did, area, url, pwd;

    @OnClick({R.id.tv_sure, R.id.ll_area})
    public void onViewClicked(View view) {
        switch (view.getId()) {
            case R.id.tv_sure:
                name = etDotname.getText().toString().trim();//节点名称
                publickey = tvPublickey.getText().toString().trim();//节点公钥
                did = tvDid.getText().toString().trim();//节点did
                area = tvArea.getText().toString().trim();//国家地址
                url = etUrl.getText().toString().trim();//官网


                if (TextUtils.isEmpty(publickey)) {
                    return;
                }
                if (TextUtils.isEmpty(did)) {
                    return;
                }
                if (TextUtils.isEmpty(name)) {
                    ToastUtils.showShort(getString(R.string.inputdotname));
                    return;
                }
                if (TextUtils.isEmpty(area)) {
                    ToastUtils.showShort(getString(R.string.countryregion_cannot_be_empty));
                    return;
                }
                if (TextUtils.isEmpty(url)) {
                    ToastUtils.showShort(getString(R.string.the_official_website_cannot_be_empty));
                    return;
                }

                dialogUtil.showWarmPromptInput(getBaseActivity(), null, null, this);
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
    public void onGetData(String methodName, BaseEntity baseEntity, Object o) {

        switch (methodName) {
            //获取钱包公钥
            case "getCROwnerPublicKey":
                tvPublickey.setText(((CommmonStringEntity) baseEntity).getData());
                break;
            case "getCROwnerDID":
                tvDid.setText(((CommmonStringEntity) baseEntity).getData());
                break;
            //验证密码
            case "exportWalletWithMnemonic":
                presenter.generateCRInfoPayload(wallet.getWalletId(), MyWallet.ELA, publickey, name, url, code, pwd, this);
                break;
            //验证交易
            case "generateCRInfoPayload":

                presenter.createRegisterCRTransaction(wallet.getWalletId(), MyWallet.ELA, "", ((CommmonStringEntity) baseEntity).getData(), Arith.mul("5000", MyWallet.RATE_S).toPlainString(), "", true, this);
                break;
            //创建交易
            case "createRegisterCRTransaction":
                Intent intent = new Intent(getActivity(), VoteTransferActivity.class);
                intent.putExtra("wallet", wallet);
                intent.putExtra("type", Constant.TRANFER);
                intent.putExtra("amount", "5000");
                intent.putExtra("chainId", MyWallet.ELA);
                intent.putExtra("toAddress", publickey);
                intent.putExtra("pwd", pwd);
                intent.putExtra("attributes", ((CommmonStringEntity) baseEntity).getData());
                startActivity(intent);
                dialogUtil.dialogDismiss();
                break;
        }
    }
}
