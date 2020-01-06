package org.elastos.wallet.ela.ui.crvote.fragment;


import android.content.Intent;
import android.os.Bundle;
import android.support.v7.widget.Toolbar;
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
import org.elastos.wallet.ela.ui.common.bean.CommmonLongEntity;
import org.elastos.wallet.ela.ui.crvote.bean.CrStatusBean;
import org.elastos.wallet.ela.ui.crvote.presenter.CRSignUpPresenter;
import org.elastos.wallet.ela.ui.vote.activity.VoteTransferActivity;
import org.elastos.wallet.ela.ui.vote.bean.Area;
import org.elastos.wallet.ela.ui.vote.fragment.AreaCodeFragment;
import org.elastos.wallet.ela.utils.AppUtlis;
import org.elastos.wallet.ela.utils.Constant;
import org.elastos.wallet.ela.utils.DialogUtil;
import org.elastos.wallet.ela.utils.MatcherUtil;
import org.elastos.wallet.ela.utils.RxEnum;
import org.elastos.wallet.ela.utils.SPUtil;
import org.elastos.wallet.ela.utils.listener.WarmPromptListener;
import org.greenrobot.eventbus.Subscribe;
import org.greenrobot.eventbus.ThreadMode;

import butterknife.BindView;
import butterknife.OnClick;

/**
 * 更新信息
 */
public class UpdateCRInformationFragment extends BaseFragment implements NewBaseViewData {


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
    CRSignUpPresenter presenter = new CRSignUpPresenter();
    private RealmUtil realmUtil = new RealmUtil();
    private Wallet wallet = realmUtil.queryDefauleWallet();
    private String ownerPublicKey;
    private String did;

    @Override
    protected int getLayoutId() {
        return R.layout.fragment_update_crinformation;
    }

    @Override
    protected void initView(View view) {
        setToobar(toolbar, toolbarTitle, getString(R.string.update_information));
        registReceiver();
        MatcherUtil.editTextFormat(etUrl, 100);
    }

    @Override
    protected void setExtraData(Bundle data) {
        CrStatusBean.InfoBean bean = data.getParcelable("info");
        etDotname.setText(bean.getNickName());
        etDotname.setEnabled(false);
        code = bean.getLocation();
        tvArea.setText(AppUtlis.getLoc(getContext(), bean.getLocation() + ""));
        etUrl.setText(bean.getURL());
        ownerPublicKey = bean.getCROwnerPublicKey();
        did = bean.getCROwnerDID();
        super.setExtraData(data);

    }

    String name, area, url;

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
               /* if (TextUtils.isEmpty(area)) {
                    ToastUtils.showShort(getString(R.string.countryregion_cannot_be_empty));
                    return;
                }
                if (TextUtils.isEmpty(url)) {
                    ToastUtils.showShort(getString(R.string.cr_website_cannot_be_empty));
                    return;
                }*/


                presenter.getFee(wallet.getWalletId(), MyWallet.ELA, "", "8USqenwzA5bSAvj1mG4SGTABykE9n5RzJQ", "0", this);

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
    public void onGetData(String methodName, BaseEntity baseEntity, Object o) {

        switch (methodName) {
            //验证密码
            case "getFee":
                Intent intent = new Intent(getActivity(), VoteTransferActivity.class);
                intent.putExtra("wallet", wallet);
                intent.putExtra("type", Constant.CRUPDATE);
                intent.putExtra("chainId", MyWallet.ELA);
                intent.putExtra("ownerPublicKey", ownerPublicKey);
                intent.putExtra("fee", ((CommmonLongEntity) baseEntity).getData());
                intent.putExtra("name", name);
                intent.putExtra("did", did);
                intent.putExtra("url", url);
                intent.putExtra("code", code);
                startActivity(intent);
                break;

        }
    }
}
