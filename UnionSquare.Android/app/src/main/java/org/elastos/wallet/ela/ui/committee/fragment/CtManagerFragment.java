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

package org.elastos.wallet.ela.ui.committee.fragment;

import android.content.Intent;
import android.os.Bundle;
import android.support.v7.widget.Toolbar;
import android.view.View;
import android.widget.TextView;

import com.alibaba.fastjson.JSON;

import org.elastos.wallet.R;
import org.elastos.wallet.ela.ElaWallet.MyWallet;
import org.elastos.wallet.ela.base.BaseFragment;
import org.elastos.wallet.ela.bean.BusEvent;
import org.elastos.wallet.ela.db.RealmUtil;
import org.elastos.wallet.ela.db.table.Wallet;
import org.elastos.wallet.ela.rxjavahelp.BaseEntity;
import org.elastos.wallet.ela.rxjavahelp.NewBaseViewData;
import org.elastos.wallet.ela.ui.Assets.activity.TransferActivity;
import org.elastos.wallet.ela.ui.committee.presenter.CtManagePresenter;
import org.elastos.wallet.ela.ui.common.bean.CommmonStringEntity;
import org.elastos.wallet.ela.ui.crvote.bean.CRDePositcoinBean;
import org.elastos.wallet.ela.ui.crvote.bean.CrStatusBean;
import org.elastos.wallet.ela.ui.did.fragment.AuthorizationFragment;
import org.elastos.wallet.ela.utils.AppUtlis;
import org.elastos.wallet.ela.utils.Arith;
import org.elastos.wallet.ela.utils.Constant;
import org.elastos.wallet.ela.utils.DialogUtil;
import org.elastos.wallet.ela.utils.RxEnum;
import org.greenrobot.eventbus.Subscribe;
import org.greenrobot.eventbus.ThreadMode;

import butterknife.BindView;
import butterknife.OnClick;

/**
 * committee manage fragment(in office, expiration office, impeachment)
 */
public class CtManagerFragment extends BaseFragment implements NewBaseViewData {
    @BindView(R.id.toolbar_title)
    TextView toolbarTitle;
    @BindView(R.id.toolbar)
    Toolbar toolbar;
    @BindView(R.id.name)
    TextView nameTv;
    @BindView(R.id.description)
    TextView description;
    @BindView(R.id.refresh_ct_info)
    TextView refreshInfo;
    @BindView(R.id.refresh_ct_did)
    TextView refreshDid;
    @BindView(R.id.deposit)
    TextView deposit;
    @BindView(R.id.first_layout)
    View firstLayout;
    @BindView(R.id.second_layout)
    View secondLayout;
    @BindView(R.id.tv_prompt)
    TextView promptTv;

    private RealmUtil realmUtil = new RealmUtil();
    private Wallet wallet = realmUtil.queryDefauleWallet();
    CtManagePresenter presenter;

    @Override
    protected int getLayoutId() {
        return R.layout.fragment_ct_manager;
    }

    @Override
    protected void initView(View view) {
        setToobar(toolbar, toolbarTitle, (!AppUtlis.isNullOrEmpty(type) && type.equalsIgnoreCase("VOTING"))?getContext().getString(R.string.votemanager):getContext().getString(R.string.ctmanager));
        registReceiver();
        presenter = new CtManagePresenter();
        presenter.getRegisteredCRInfo(wallet.getWalletId(), MyWallet.ELA, this);
    }

    String type;
    String depositAmount;
    String status;
    String did;
    String name;

    @Override
    protected void setExtraData(Bundle data) {
        super.setExtraData(data);
        did = data.getString("did");
        status = data.getString("status");
        name = data.getString("name");
        type = data.getString("type");
        depositAmount = data.getString("depositAmount");

        //当届
        if(!AppUtlis.isNullOrEmpty(type) && type.equalsIgnoreCase("CURRENT")) {
            //任职不正常
            if(!AppUtlis.isNullOrEmpty(status)
                    && !AppUtlis.isNullOrEmpty(depositAmount)
                    && !depositAmount.trim().equalsIgnoreCase("0")
                   && (status.equalsIgnoreCase("Terminated")
                            || status.equalsIgnoreCase("Impeached")
                            || status.equalsIgnoreCase("Returned"))) {
                showFirstLayout();
            } else {
                showSecondLayout();
                showRefreshView();
            }
        } else if(!AppUtlis.isNullOrEmpty(type) && type.equalsIgnoreCase("VOTING")) {
            showSecondLayout();
            showDepositView();
        } else { // from FindFragment
            showFirstLayout();
        }
    }

    private void showRefreshView() {
        refreshInfo.setVisibility(View.VISIBLE);
        refreshDid.setVisibility(View.VISIBLE);
        deposit.setVisibility(View.GONE);
    }

    private void showDepositView() {
        refreshInfo.setVisibility(View.GONE);
        refreshDid.setVisibility(View.GONE);
        deposit.setVisibility(View.VISIBLE);
    }


    @OnClick({R.id.refresh_ct_info, R.id.refresh_ct_did, R.id.deposit, R.id.tv_close, R.id.tv_deposit})
    public void onClick(View view) {
        Bundle bundle = new Bundle();
        switch (view.getId()) {
            case R.id.refresh_ct_info:
                bundle.putParcelable("crStatusBean", crStatusBean);
                start(UpdateCtInformationFragment.class, bundle);
//                new WalletManagePresenter().DIDResolveWithTip(wallet.getDid().replace("did:elastos:", ""), this, "2");
                break;
            case R.id.refresh_ct_did:
                bundle.putString("type", "authorization");
                bundle.putParcelable("wallet", wallet);
                start(AuthorizationFragment.class, bundle);
                break;
            case R.id.deposit:
//                presenter.getCRDepositcoin(did, this);
                if(!AppUtlis.isNullOrEmpty(depositAmount)) {
                    String ownerPublicKey = crStatusBean.getInfo().getCROwnerPublicKey();
                    presenter.createRetrieveCRDepositTransaction(wallet.getWalletId(), MyWallet.ELA, ownerPublicKey,
                            Arith.mulRemoveZero(depositAmount, MyWallet.RATE_S).toPlainString(), "", this);
                }
                break;
            case R.id.tv_close:
                popBackFragment();
                break;
            case R.id.tv_deposit:
                showSecondLayout();
                break;
        }
    }


    private void showFirstLayout() {
        firstLayout.setVisibility(View.VISIBLE);
        secondLayout.setVisibility(View.GONE);
        if(AppUtlis.isNullOrEmpty(status)) return;
        switch (status) {
            case "Terminated":
                promptTv.setText(getString(R.string.completedialoghint));
                break;
            case "Impeached":
                promptTv.setText(getString(R.string.canceldialoghint));
                break;
            case "Returned":
                promptTv.setText(getString(R.string.dimissdialoghint));
                break;
            default:
        }
    }

    private void showSecondLayout() {
        firstLayout.setVisibility(View.GONE);
        secondLayout.setVisibility(View.VISIBLE);

        if(!AppUtlis.isNullOrEmpty(type) && type.equalsIgnoreCase("VOTING")) {
            nameTv.setText(name);
            description.setText(R.string.votingfinishhint);
            showDepositView();
            return;
        }

        switch (status) {
            case "Terminated":
                description.setText(String.format(getString(R.string.completeofficehint), depositAmount));
                showDepositView();
                break;
            case "Impeached":
                description.setText(String.format(getString(R.string.cancelofficehint), depositAmount));
                showDepositView();
                break;
            case "Returned":
                description.setText(String.format(getString(R.string.dimissofficehint), depositAmount));
                showDepositView();
                break;
            case "Elected":
                description.setText(getString(R.string.inofficehint));
                showRefreshView();
                break;
            default:
                throw new IllegalStateException("Unexpected value: " + status);
        }
    }

    CrStatusBean crStatusBean = null;

    @Override
    public void onGetData(String methodName, BaseEntity baseEntity, Object o) {
        switch (methodName) {
            case "createRetrieveCRDepositTransaction":
                Intent intent = new Intent(getActivity(), TransferActivity.class);
                intent.putExtra("wallet", wallet);
                intent.putExtra("type", Constant.WITHDRAWCR);
                intent.putExtra("amount", depositAmount);
                intent.putExtra("chainId", MyWallet.ELA);
                intent.putExtra("transType", 36);
                intent.putExtra("attributes", ((CommmonStringEntity) baseEntity).getData());
                startActivity(intent);
                break;

            case "getCRDepositcoin":
                CRDePositcoinBean getdePositcoinBean = (CRDePositcoinBean) baseEntity;
                depositAmount = getdePositcoinBean.getData().getResult().getAvailable();
                if (!AppUtlis.isNullOrEmpty(depositAmount))
                    presenter.getRegisteredCRInfo(wallet.getWalletId(), MyWallet.ELA, this);
                break;

            case "getRegisteredCRInfo":
                crStatusBean = JSON.parseObject(((CommmonStringEntity) baseEntity).getData(), CrStatusBean.class);
                break;
        }
    }

    @Subscribe(threadMode = ThreadMode.MAIN)
    public void Event(BusEvent result) {
        int integer = result.getCode();
        if (integer == RxEnum.TRANSFERSUCESS.ordinal()) {
            new DialogUtil().showTransferSucess(getBaseActivity(), view -> popBackFragment());
        }
    }

}
