package org.elastos.wallet.ela.ui.crvote.fragment;


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
import org.elastos.wallet.ela.rxjavahelp.BaseEntity;
import org.elastos.wallet.ela.rxjavahelp.NewBaseViewData;
import org.elastos.wallet.ela.ui.Assets.presenter.TransferPresenter;
import org.elastos.wallet.ela.ui.common.bean.CommmonStringEntity;
import org.elastos.wallet.ela.ui.common.viewdata.CommmonStringViewData;
import org.elastos.wallet.ela.ui.crvote.bean.CRListBean;
import org.elastos.wallet.ela.ui.crvote.presenter.CRSignUpPresenter;
import org.elastos.wallet.ela.ui.vote.activity.VoteTransferActivity;
import org.elastos.wallet.ela.ui.vote.bean.Area;
import org.elastos.wallet.ela.ui.vote.fragment.AreaCodeFragment;
import org.elastos.wallet.ela.utils.Arith;
import org.elastos.wallet.ela.utils.Constant;
import org.elastos.wallet.ela.utils.DialogUtil;
import org.elastos.wallet.ela.utils.RxEnum;
import org.elastos.wallet.ela.utils.SPUtil;
import org.elastos.wallet.ela.utils.listener.WarmPromptListener;
import org.greenrobot.eventbus.Subscribe;
import org.greenrobot.eventbus.ThreadMode;
import org.json.JSONException;
import org.json.JSONObject;

import java.util.ArrayList;

import butterknife.BindView;
import butterknife.OnClick;

/**
 * 报名参选
 */
public class CRSignUpForFragment extends BaseFragment implements NewBaseViewData, CommmonStringViewData {
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
    private ArrayList<CRListBean.DataBean.ResultBean.CrcandidatesinfoBean> netList;


    @Override
    protected int getLayoutId() {
        return R.layout.fragment_cr_signup;
    }

    @Override
    protected void setExtraData(Bundle data) {
        did = data.getString("did");
        tvDid.setText(did);
        netList = (ArrayList<CRListBean.DataBean.ResultBean.CrcandidatesinfoBean>) data.getSerializable("netList");

    }

    @Override
    protected void initView(View view) {
        tvTitle.setText(R.string.sign_up_for);
        presenter = new CRSignUpPresenter();
        presenter.getCROwnerPublicKey(wallet.getWalletId(), MyWallet.ELA, this);
        registReceiver();

    }


    String name, ownerPublicKey, did, area, url = "";

    @OnClick({R.id.tv_sure, R.id.ll_area})
    public void onViewClicked(View view) {
        switch (view.getId()) {
            case R.id.tv_sure:
                name = etDotname.getText().toString().trim();//节点名称
                ownerPublicKey = tvPublickey.getText().toString().trim();//节点公钥
                did = tvDid.getText().toString().trim();//节点did
                area = tvArea.getText().toString().trim();//国家地址
                url = etUrl.getText().toString().trim();//官网

                for (CRListBean.DataBean.ResultBean.CrcandidatesinfoBean bean : netList) {
                    if (name.equals(bean.getNickname())) {
                        showToast(getString(R.string.menbernameexist));
                        return;
                    }
                }
                if (TextUtils.isEmpty(ownerPublicKey)) {
                    return;
                }
                if (TextUtils.isEmpty(did)) {
                    return;
                }
                if (TextUtils.isEmpty(name)) {
                    ToastUtils.showShort(getString(R.string.menbernamenotnull));
                    return;
                }
               /* if (TextUtils.isEmpty(area)) {
                    ToastUtils.showShort(getString(R.string.countryregion_cannot_be_empty));
                    return;
                }*/
              /*  if (TextUtils.isEmpty(url)) {
                    ToastUtils.showShort(getString(R.string.the_official_website_cannot_be_empty));
                    return;
                }*/

                //模拟交易获得手续费
                new TransferPresenter().createTransaction(wallet.getWalletId(), MyWallet.ELA, "", "8USqenwzA5bSAvj1mG4SGTABykE9n5RzJQ", Arith.mul("5000", MyWallet.RATE_S).toPlainString(), "", true, this);
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

        // switch (methodName) {

        //  case "getCROwnerPublicKey":
        tvPublickey.setText(((CommmonStringEntity) baseEntity).getData());
        //    break;
        //  }
    }

    @Override
    public void onGetCommonData(String data) {
        //获得createTransaction
        //这里获得手续费
        long fee = MyWallet.feePerKb;
        try {
            JSONObject jsonObject = new JSONObject(data);

            if (jsonObject.has("Fee")) {
                fee = jsonObject.getLong("Fee");
            }

        } catch (JSONException e) {
            e.printStackTrace();
        }
        //获得手续费后
        Intent intent = new Intent(getActivity(), VoteTransferActivity.class);
        intent.putExtra("wallet", wallet);
        intent.putExtra("type", Constant.CRSIGNUP);
        intent.putExtra("amount", "5000");
        intent.putExtra("chainId", MyWallet.ELA);
        intent.putExtra("ownerPublicKey", ownerPublicKey);
        intent.putExtra("fee", fee);
        intent.putExtra("name", name);
        intent.putExtra("url", url);
        intent.putExtra("code", code);

        startActivity(intent);
    }
}
