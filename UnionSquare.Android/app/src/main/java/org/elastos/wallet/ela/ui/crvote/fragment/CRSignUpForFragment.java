/*
 * Copyright (c) 2019 Elastos Foundation
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

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
import org.elastos.wallet.ela.db.table.Wallet;
import org.elastos.wallet.ela.ui.Assets.presenter.TransferPresenter;
import org.elastos.wallet.ela.ui.common.viewdata.CommmonStringViewData;
import org.elastos.wallet.ela.ui.crvote.bean.CRListBean;
import org.elastos.wallet.ela.ui.crvote.presenter.CRSignUpPresenter;
import org.elastos.wallet.ela.ui.vote.activity.VoteTransferActivity;
import org.elastos.wallet.ela.ui.vote.bean.Area;
import org.elastos.wallet.ela.ui.vote.fragment.AreaCodeFragment;
import org.elastos.wallet.ela.utils.Arith;
import org.elastos.wallet.ela.utils.Constant;
import org.elastos.wallet.ela.utils.DialogUtil;
import org.elastos.wallet.ela.utils.MatcherUtil;
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
public class CRSignUpForFragment extends BaseFragment implements CommmonStringViewData {
    @BindView(R.id.tv_title)
    TextView tvTitle;
    @BindView(R.id.tv_did)
    TextView tvDid;
    @BindView(R.id.et_dotname)
    EditText etDotname;
    @BindView(R.id.tv_area)
    TextView tvArea;
    @BindView(R.id.et_url)
    EditText etUrl;

    private Wallet wallet;
    CRSignUpPresenter presenter;
    private ArrayList<CRListBean.DataBean.ResultBean.CrcandidatesinfoBean> netList;


    @Override
    protected int getLayoutId() {
        return R.layout.fragment_cr_signup;
    }

    @Override
    protected void setExtraData(Bundle data) {
        ownerPublicKey = data.getString("publickey");
        wallet = data.getParcelable("wallet");
        CID = data.getString("CID");
        tvDid.setText(wallet.getDid());//todo 没有did
        netList = (ArrayList<CRListBean.DataBean.ResultBean.CrcandidatesinfoBean>) data.getSerializable("netList");

    }

    @Override
    protected void initView(View view) {
        tvTitle.setText(R.string.sign_up_for);
        presenter = new CRSignUpPresenter();

        registReceiver();
        MatcherUtil.editTextFormat(etDotname, 100);
        MatcherUtil.editTextFormat(etUrl, 100);
    }


    String name, ownerPublicKey, CID, area, url = "";

    @OnClick({R.id.tv_sure, R.id.ll_area})
    public void onViewClicked(View view) {
        switch (view.getId()) {
            case R.id.tv_sure:
                name = etDotname.getText().toString().trim();//节点名称
                area = tvArea.getText().toString().trim();//国家地址
                url = etUrl.getText().toString().trim();//官网
                if (netList != null && netList.size() != 0) {
                    for (CRListBean.DataBean.ResultBean.CrcandidatesinfoBean bean : netList) {
                        if (name.equals(bean.getNickname())) {
                            showToast(getString(R.string.menbernameexist));
                            return;
                        }
                    }
                }
                if (TextUtils.isEmpty(ownerPublicKey)) {
                    return;
                }
                if (TextUtils.isEmpty(wallet.getDid())) {
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
                new TransferPresenter().createTransaction(wallet.getWalletId(), MyWallet.ELA, "", "8USqenwzA5bSAvj1mG4SGTABykE9n5RzJQ", Arith.mulRemoveZero("5000", MyWallet.RATE_S).toPlainString(), "", true, this);
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
        intent.putExtra("CID", CID);
        intent.putExtra("name", name);
        intent.putExtra("url", url);
        intent.putExtra("code", code);
        intent.putExtra("transType", 33);

        startActivity(intent);
    }
}
