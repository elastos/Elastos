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

package org.elastos.wallet.ela.ui.vote.UpdateInformation;


import android.content.Intent;
import android.os.Bundle;
import android.support.v7.widget.Toolbar;
import android.text.TextUtils;
import android.view.View;
import android.widget.EditText;
import android.widget.TextView;

import com.allen.library.SuperButton;
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
import org.elastos.wallet.ela.ui.crvote.presenter.CRSignUpPresenter;
import org.elastos.wallet.ela.ui.vote.activity.VoteTransferActivity;
import org.elastos.wallet.ela.ui.vote.bean.Area;
import org.elastos.wallet.ela.ui.vote.bean.VoteListBean;
import org.elastos.wallet.ela.ui.vote.fragment.AreaCodeFragment;
import org.elastos.wallet.ela.utils.AppUtlis;
import org.elastos.wallet.ela.utils.ClearEditText;
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
public class UpdateInformationFragment extends BaseFragment implements NewBaseViewData {


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

    private RealmUtil realmUtil = new RealmUtil();
    private Wallet wallet = realmUtil.queryDefauleWallet();
    private String ownerPublicKey;

    @Override
    protected int getLayoutId() {
        return R.layout.fragment_update_information;
    }

    @Override
    protected void initView(View view) {
        setToobar(toolbar, toolbarTitle, getString(R.string.update_information));
        registReceiver();
    }

    @Override
    protected void setExtraData(Bundle data) {
        VoteListBean.DataBean.ResultBean.ProducersBean bean = (VoteListBean.DataBean.ResultBean.ProducersBean) data.getSerializable("curentNode");

        etDotname.setText(bean.getNickname());
        etDotname.setEnabled(false);
        code = bean.getLocation();
        tvArea.setText(AppUtlis.getLoc(getContext(), bean.getLocation() + ""));
        etUrl.setText(bean.getUrl());
        ownerPublicKey = bean.getOwnerpublickey();
        etPublickey.setText(bean.getNodepublickey());
        super.setExtraData(data);
        MatcherUtil.editTextFormat(etUrl, 100);

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
                new CRSignUpPresenter().getFee(wallet.getWalletId(), MyWallet.ELA, "", "8USqenwzA5bSAvj1mG4SGTABykE9n5RzJQ", "0", this);


                break;
            case R.id.ll_area:
                //选择国家和地区
                start(new AreaCodeFragment());
                break;
        }
    }


    @Override
    public void onGetData(String methodName, BaseEntity baseEntity, Object o) {

        switch (methodName) {
            //验证密码
            case "getFee":
                Intent intent = new Intent(getActivity(), VoteTransferActivity.class);
                intent.putExtra("wallet", wallet);
                intent.putExtra("type", Constant.UPDATENODEINFO);
                intent.putExtra("chainId", MyWallet.ELA);
                intent.putExtra("ownerPublicKey", ownerPublicKey);
                intent.putExtra("nodePublicKey", nodePublicKey);
                intent.putExtra("fee", ((CommmonLongEntity) baseEntity).getData());
                intent.putExtra("name", name);
                intent.putExtra("url", url);
                intent.putExtra("code", code);
                intent.putExtra("transType", 11);
                startActivity(intent);
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
