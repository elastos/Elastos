package org.elastos.wallet.ela.ui.crvote.fragment;


import android.os.Bundle;
import android.support.v7.widget.Toolbar;
import android.text.TextUtils;
import android.view.View;
import android.widget.EditText;
import android.widget.TextView;

import com.alibaba.fastjson.JSON;
import com.blankj.utilcode.util.ToastUtils;

import org.elastos.wallet.R;
import org.elastos.wallet.ela.ElaWallet.MyWallet;
import org.elastos.wallet.ela.base.BaseFragment;
import org.elastos.wallet.ela.bean.BusEvent;
import org.elastos.wallet.ela.db.RealmUtil;
import org.elastos.wallet.ela.db.table.Wallet;
import org.elastos.wallet.ela.rxjavahelp.BaseEntity;
import org.elastos.wallet.ela.rxjavahelp.NewBaseViewData;
import org.elastos.wallet.ela.ui.Assets.presenter.PwdPresenter;
import org.elastos.wallet.ela.ui.Assets.presenter.WallletManagePresenter;
import org.elastos.wallet.ela.ui.common.bean.CommmonBooleanEntity;
import org.elastos.wallet.ela.ui.common.bean.CommmonStringEntity;
import org.elastos.wallet.ela.ui.common.viewdata.CommmonStringWithMethNameViewData;
import org.elastos.wallet.ela.ui.crvote.bean.CRMenberInfoBean;
import org.elastos.wallet.ela.ui.crvote.presenter.CRSignUpPresenter;
import org.elastos.wallet.ela.ui.vote.bean.Area;
import org.elastos.wallet.ela.ui.vote.fragment.AreaCodeFragment;
import org.elastos.wallet.ela.utils.AppUtlis;
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
 * 更新信息
 */
public class UpdateCRInformationFragment extends BaseFragment implements WarmPromptListener, CommmonStringWithMethNameViewData, NewBaseViewData {


    @BindView(R.id.toolbar_title)
    TextView toolbarTitle;
    @BindView(R.id.toolbar)
    Toolbar toolbar;
    @BindView(R.id.et_dotname)
    EditText etDotname;

    @BindView(R.id.tv_area)
    TextView tvArea;
    @BindView(R.id.et_url)
    EditText etUrl;
    DialogUtil dialogUtil = new DialogUtil();
    CRSignUpPresenter presenter = new CRSignUpPresenter();
    private RealmUtil realmUtil = new RealmUtil();
    private Wallet wallet = realmUtil.queryDefauleWallet();
    private PwdPresenter pwdPresenter = new PwdPresenter();
    private String ownerPublicKey;

    @Override
    protected int getLayoutId() {
        return R.layout.fragment_update_crinformation;
    }

    @Override
    protected void initView(View view) {
        setToobar(toolbar, toolbarTitle, getString(R.string.update_information));
        EventBus.getDefault().register(this);
    }

    @Override
    protected void setExtraData(Bundle data) {
        String i = data.getString("info");
        CRMenberInfoBean bean = JSON.parseObject(i, CRMenberInfoBean.class);
        etDotname.setText(bean.getNickName());
        etDotname.setEnabled(false);
        code = bean.getLocation();
        tvArea.setText(AppUtlis.getLoc(getContext(), bean.getLocation() + ""));
        etUrl.setText(bean.getURL());
        ownerPublicKey = bean.getCROwnerPublicKey();
        super.setExtraData(data);

    }

    String name, area, url, pwd;

    @OnClick({R.id.sb_up, R.id.ll_area})
    public void onViewClicked(View view) {
        switch (view.getId()) {
            case R.id.sb_up:
                name = etDotname.getText().toString().trim();//节点名称
                area = tvArea.getText().toString().trim();//国家地址
                url = etUrl.getText().toString().trim();//官网

                if (TextUtils.isEmpty(name)) {
                    ToastUtils.showShort(getString(R.string.inputdotname));
                    return;
                }
                if (TextUtils.isEmpty(ownerPublicKey)) {
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
        new WallletManagePresenter().verifyPayPassword(wallet.getWalletId(), pwd, this);
    }


    @Override
    public void onGetCommonData(String methodname, String data) {

        switch (methodname) {
            //验证交易
            case "signTransaction":
                pwdPresenter.publishTransaction(wallet.getWalletId(), MyWallet.ELA, data, this);
                break;
            case "publishTransaction":
                ToastUtils.showShort(R.string.update_successful);
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

    @Override
    public void onGetData(String methodName, BaseEntity baseEntity, Object o) {

        switch (methodName) {
            //验证密码
            case "verifyPayPassword":
                boolean result = ((CommmonBooleanEntity) baseEntity).getData();
                if (result) {
                    presenter.generateCRInfoPayload(wallet.getWalletId(), MyWallet.ELA, ownerPublicKey, name, url, code, pwd, this);
                } else {
                    showToastMessage(getString(R.string.error_20003));
                }
                break;
            case "generateCRInfoPayload":
                presenter.createUpdateCRTransaction(wallet.getWalletId(), MyWallet.ELA, "", ((CommmonStringEntity) baseEntity).getData(), "", false, this);
                break;
            //创建交易
            case "createUpdateCRTransaction":
                pwdPresenter.signTransaction(wallet.getWalletId(), MyWallet.ELA, ((CommmonStringEntity) baseEntity).getData(), pwd, this);
                dialogUtil.dialogDismiss();
                break;
        }
    }
}
