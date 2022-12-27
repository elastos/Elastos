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

import android.os.Bundle;
import android.support.v7.widget.LinearLayoutManager;
import android.support.v7.widget.RecyclerView;
import android.view.View;
import android.widget.TextView;

import org.elastos.wallet.R;
import org.elastos.wallet.ela.base.BaseFragment;
import org.elastos.wallet.ela.db.table.SubWallet;
import org.elastos.wallet.ela.db.table.Wallet;
import org.elastos.wallet.ela.rxjavahelp.BaseEntity;
import org.elastos.wallet.ela.rxjavahelp.NewBaseViewData;
import org.elastos.wallet.ela.ui.Assets.adapter.SyncReSetRecAdapetr;
import org.elastos.wallet.ela.ui.Assets.presenter.ResetSyncPresenter;
import org.elastos.wallet.ela.ui.common.listener.CommonRvListener;
import org.elastos.wallet.ela.utils.DialogUtil;
import org.elastos.wallet.ela.utils.RxEnum;
import org.elastos.wallet.ela.utils.listener.WarmPromptListener;

import java.util.ArrayList;
import java.util.List;

import butterknife.BindView;
import butterknife.OnClick;

public class SyncResetFragment extends BaseFragment implements CommonRvListener, NewBaseViewData {
    @BindView(R.id.tv_title)
    TextView tvTitle;
    @BindView(R.id.rv)
    RecyclerView rv;
    private List<SubWallet> subWallets;
    private List<SubWallet> checkList;
    private ResetSyncPresenter presenter;
    private Wallet wallet;

    @Override
    protected int getLayoutId() {
        return R.layout.fragment_syncreset;
    }

    @Override
    protected void setExtraData(Bundle data) {
        wallet = data.getParcelable("wallet");
        subWallets = data.getParcelableArrayList("subWallets");
        setRecycleView(subWallets);
    }

    @Override
    protected void initView(View view) {
        tvTitle.setText(R.string.selectreset);
        presenter = new ResetSyncPresenter();

    }

    @OnClick({R.id.tv_reset})
    public void onViewClicked(View view) {
        switch (view.getId()) {
            case R.id.tv_reset:

                if (checkList == null || checkList.size() == 0) {
                    showToast(getString(R.string.plzselectreset));
                    return;
                }

                new DialogUtil().showWarmPrompt1(getBaseActivity(), getString(R.string.deletesyncornot), new WarmPromptListener() {
                    @Override
                    public void affireBtnClick(View view) {
                        for (SubWallet subWallet : checkList) {
                            presenter.resync(wallet.getWalletId(), subWallet.getChainId(), SyncResetFragment.this);
                            subWallet.setFiled1("Connecting");
                            subWallet.setSyncTime("- -");
                            subWallet.setProgress(0);
                            subWallet.setFiled2("false");
                            subWallet.setBytesPerSecond(0);
                            subWallet.setDownloadPeer(null);
                        }
                        post(RxEnum.UPDATAPROGRESS.ordinal(), null, null);
                        popBackFragment();
                    }
                });


                break;

        }
    }

    private void setRecycleView(List<SubWallet> assetList) {


        if (assetList == null || assetList.size() == 0) {
            return;
        }
        SyncReSetRecAdapetr assetskAdapter = new SyncReSetRecAdapetr(getContext(), assetList);
        rv.setAdapter(assetskAdapter);
        rv.setLayoutManager(new LinearLayoutManager(getContext()));
        assetskAdapter.setCommonRvListener(this);

    }

    @Override
    public void onRvItemClick(int position, Object o) {
        SubWallet subWallet = subWallets.get(position);
        boolean cb = (boolean) o;
        if (checkList == null) {
            checkList = new ArrayList<>();
        }
        if (cb) {
            checkList.add(subWallet);
        } else {
            checkList.remove(subWallet);
        }
    }

    @Override
    public void onGetData(String methodName, BaseEntity baseEntity, Object o) {

    }
}
