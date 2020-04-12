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

package org.elastos.wallet.ela.ui.Assets.fragment;

import android.app.Dialog;
import android.os.Bundle;
import android.support.v7.widget.LinearLayoutManager;
import android.support.v7.widget.RecyclerView;
import android.text.TextUtils;
import android.view.View;
import android.widget.EditText;
import android.widget.ImageView;
import android.widget.TextView;

import org.elastos.wallet.R;
import org.elastos.wallet.ela.base.BaseFragment;
import org.elastos.wallet.ela.bean.CreateWalletBean;
import org.elastos.wallet.ela.db.RealmUtil;
import org.elastos.wallet.ela.db.listener.RealmTransactionAbs;
import org.elastos.wallet.ela.db.table.Wallet;
import org.elastos.wallet.ela.rxjavahelp.BaseEntity;
import org.elastos.wallet.ela.rxjavahelp.NewBaseViewData;
import org.elastos.wallet.ela.ui.Assets.adapter.WalletListRecAdapetr;
import org.elastos.wallet.ela.ui.Assets.fragment.mulsignwallet.CreateMulWalletFragment;
import org.elastos.wallet.ela.ui.Assets.presenter.mulwallet.CreatMulWalletPresenter;
import org.elastos.wallet.ela.ui.common.bean.CommmonStringEntity;
import org.elastos.wallet.ela.ui.common.listener.CommonRvListener;
import org.elastos.wallet.ela.utils.Constant;
import org.elastos.wallet.ela.utils.DialogUtil;
import org.elastos.wallet.ela.utils.Log;
import org.elastos.wallet.ela.utils.RxEnum;
import org.elastos.wallet.ela.utils.listener.WarmPromptListener;

import java.util.List;

import butterknife.BindView;
import butterknife.OnClick;
import butterknife.Unbinder;

public class WalletListFragment extends BaseFragment implements CommonRvListener, WarmPromptListener, NewBaseViewData {
    @BindView(R.id.tv_title)
    TextView tvTitle;
    @BindView(R.id.rv)
    RecyclerView rv;
    @BindView(R.id.iv_add)
    ImageView ivAdd;
    Unbinder unbinder;
    private RealmUtil realmUtil;
    private String openType;
    private String pwd;
    private Dialog dialog;


    @Override
    protected int getLayoutId() {
        return R.layout.fragment_wallet_list;
    }

    @Override
    protected void initInjector() {

    }

    @Override
    protected void setExtraData(Bundle data) {
        // bundle.putString("openType","showList");
        openType = data.getString("openType", "");

    }

    @Override
    protected void initView(View view) {
        realmUtil = new RealmUtil();

        tvTitle.setText(R.string.walletlist);
        if (openType.equals("showList")) {
            ivAdd.setVisibility(View.GONE);
            setRecycleView(realmUtil.queryTypeUserAllWallet(0), false);
            return;
        }
        setRecycleView(realmUtil.queryUserAllWallet(), true);
    }

    private void setRecycleView(List<Wallet> list, boolean showStatus) {
        WalletListRecAdapetr adapter = new WalletListRecAdapetr(getContext(), list, showStatus);
        rv.setLayoutManager(new LinearLayoutManager(getContext(), LinearLayoutManager.VERTICAL, false));
        rv.setAdapter(adapter);
        adapter.setCommonRvListener(this);


    }

    @OnClick({R.id.iv_add})
    public void onViewClicked(View view) {
        switch (view.getId()) {
            case R.id.iv_add:
                //创建钱包
                Bundle bundle = new Bundle();
                HomeWalletFragment homeWalletFragment = new HomeWalletFragment();
                bundle.putString("type", Constant.INNER);
                homeWalletFragment.setArguments(bundle);
                start(homeWalletFragment);
                break;

        }
    }

    private Wallet curentWallt;

    @Override
    public void onRvItemClick(int position, Object o) {
        //条目的点击事件

        if (openType.equals("showList")) {
            curentWallt = (Wallet) o;
            Log.i("??", curentWallt.getWalletName() + position);
            dialog = new DialogUtil().showWarmPromptInput(getBaseActivity(), null, null, this);
            return;
        }
        realmUtil.updateWalletDefault(((Wallet) o).getWalletId(), new RealmTransactionAbs() {
            @Override
            public void onSuccess() {
                post(RxEnum.WALLETUPDATE.ordinal(), "", o);
                popBackFragment();
            }
        });

    }

    @Override
    public void affireBtnClick(View view) {
        pwd = ((EditText) view).getText().toString().trim();
        if (TextUtils.isEmpty(pwd)) {
            showToastMessage(getString(R.string.pwdnoempty));
            return;
        }
        new CreatMulWalletPresenter().exportxPrivateKey(curentWallt.getWalletId(), pwd, this);
    }



    @Override
    public void onGetData(String methodName, BaseEntity baseEntity, Object o) {
        String data = ((CommmonStringEntity) baseEntity).getData();
        dialog.dismiss();
        CreateWalletBean createWalletBean = new CreateWalletBean();
        createWalletBean.setMasterWalletID(curentWallt.getWalletId());
        createWalletBean.setPayPassword(pwd);
        createWalletBean.setPrivateKey(data);
        post(RxEnum.SELECTRIVATEKEY.ordinal(), "", createWalletBean);
        popTo(CreateMulWalletFragment.class, false);

    }
}
