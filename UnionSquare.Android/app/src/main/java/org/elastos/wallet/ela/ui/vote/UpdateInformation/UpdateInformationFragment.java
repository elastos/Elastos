package org.elastos.wallet.ela.ui.vote.UpdateInformation;


import android.os.Bundle;
import android.support.v7.widget.Toolbar;
import android.text.TextUtils;
import android.view.View;
import android.widget.EditText;
import android.widget.TextView;

import com.alibaba.fastjson.JSON;
import com.allen.library.SuperButton;
import com.blankj.utilcode.util.ToastUtils;

import org.elastos.wallet.R;
import org.elastos.wallet.ela.ElaWallet.MyWallet;
import org.elastos.wallet.ela.base.BaseFragment;
import org.elastos.wallet.ela.bean.BusEvent;
import org.elastos.wallet.ela.db.RealmUtil;
import org.elastos.wallet.ela.db.table.Wallet;
import org.elastos.wallet.ela.ui.Assets.presenter.PwdPresenter;
import org.elastos.wallet.ela.ui.Assets.presenter.WallletManagePresenter;
import org.elastos.wallet.ela.ui.common.viewdata.CommmonStringWithMethNameViewData;
import org.elastos.wallet.ela.ui.vote.bean.Area;
import org.elastos.wallet.ela.ui.vote.bean.ElectoralAffairsBean;
import org.elastos.wallet.ela.ui.vote.fragment.AreaCodeFragment;
import org.elastos.wallet.ela.ui.vote.signupfor.SignUpPresenter;
import org.elastos.wallet.ela.utils.AppUtlis;
import org.elastos.wallet.ela.utils.ClearEditText;
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
 * 更新信息
 */
public class UpdateInformationFragment extends BaseFragment implements WarmPromptListener, CommmonStringWithMethNameViewData {


    @BindView(R.id.statusbarutil_fake_status_bar_view)
    View statusbarutilFakeStatusBarView;
    @BindView(R.id.toolbar_title)
    TextView toolbarTitle;
    @BindView(R.id.toolbar)
    Toolbar toolbar;
    @BindView(R.id.et_dotname)
    EditText etDotname;
    @BindView(R.id.et_publickey)
    ClearEditText etPublickey;
    @BindView(R.id.tv_area)
    TextView tvArea;
    @BindView(R.id.et_url)
    EditText etUrl;

    @BindView(R.id.sb_up)
    SuperButton sbUp;

    DialogUtil dialogUtil = new DialogUtil();
    SignUpPresenter presenter = new SignUpPresenter();
    private RealmUtil realmUtil = new RealmUtil();
    private Wallet wallet = realmUtil.queryDefauleWallet();
    private PwdPresenter pwdPresenter = new PwdPresenter();
    private String ownerPublicKey;

    @Override
    protected int getLayoutId() {
        return R.layout.fragment_update_information;
    }

    @Override
    protected void initView(View view) {
        setToobar(toolbar, toolbarTitle, getString(R.string.update_information));
        EventBus.getDefault().register(this);
    }

    @Override
    protected void setExtraData(Bundle data) {
        String i = data.getString("info");
        ElectoralAffairsBean bean = JSON.parseObject(i, ElectoralAffairsBean.class);
        etDotname.setText(bean.getNickName());
        etDotname.setEnabled(false);
        code = bean.getLocation();
        tvArea.setText(AppUtlis.getLoc(getContext(), bean.getLocation() + ""));
        etUrl.setText(bean.getURL());
        ownerPublicKey = bean.getOwnerPublicKey();
        etPublickey.setText(bean.getNodePublicKey());
        super.setExtraData(data);

    }

    String name, nodePublicKey, net, area, url, pwd;

    @OnClick({R.id.sb_up, R.id.ll_area})
    public void onViewClicked(View view) {
        switch (view.getId()) {
            case R.id.sb_up:
                name = etDotname.getText().toString().trim();//节点名称
                nodePublicKey = etPublickey.getText().toString().trim();//节点公钥
                area = tvArea.getText().toString().trim();//国家地址
                url = etUrl.getText().toString().trim();//官网

                if (TextUtils.isEmpty(name)) {
                    ToastUtils.showShort(getString(R.string.inputdotname));
                    return;
                }
                if (TextUtils.isEmpty(nodePublicKey)) {
                    ToastUtils.showShort(getString(R.string.inputdotpublickey));
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
                /*if (!AppUtlis.urlReg(url)) {
                    ToastUtils.showShort(getString(R.string.please_enter_the_correct_url));
                    return;
                }*/
                dialogUtil.showWarmPrompt1(getBaseActivity(), getString(R.string.charge) + ":0.0001ELA", new WarmPromptListener() {
                    @Override
                    public void affireBtnClick(View view) {
                        showWarmPromptInput();
                    }
                });

                break;
            case R.id.ll_area:
                //选择国家和地区
                start(new AreaCodeFragment());
                break;
        }
    }

    private void showWarmPromptInput() {
        dialogUtil.showWarmPromptInput(getBaseActivity(), getString(R.string.securitycertificate), getString(R.string.inputWalletPwd), this);
    }

    @Override
    public void affireBtnClick(View view) {
        pwd = ((EditText) view).getText().toString().trim();
        if (TextUtils.isEmpty(pwd)) {
            showToastMessage(getString(R.string.pwdnoempty));
            return;
        }
        new WallletManagePresenter().exportWalletWithMnemonic(wallet.getWalletId(), pwd, this);
    }


    @Override
    public void onGetCommonData(String methodname, String data) {

        switch (methodname) {

            //验证密码
            case "exportWalletWithMnemonic":

                presenter.generateProducerPayload(wallet.getWalletId(), MyWallet.ELA, ownerPublicKey, nodePublicKey, name, url, net, code, pwd, this);
                break;
            //验证交易
            case "generateProducerPayload":
                KLog.a(data);
                presenter.createUpdateProducerTransaction(wallet.getWalletId(), MyWallet.ELA, "", data, "", false, this);
                break;
            //创建交易
            case "createUpdateProducerTransaction":
                pwdPresenter.signTransaction(wallet.getWalletId(), MyWallet.ELA, data, pwd, this);
                dialogUtil.dialogDismiss();
                break;

            case "signTransaction":
                pwdPresenter.publishTransaction(wallet.getWalletId(), MyWallet.ELA, data, this);
                break;
            case "publishTransaction":
                ToastUtils.showShort(R.string.update_successful);
                //post(RxEnum.TRANSFERSUCESS.ordinal(), getString(R.string.for_successful), null);
                _mActivity.onBackPressed();
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
}
