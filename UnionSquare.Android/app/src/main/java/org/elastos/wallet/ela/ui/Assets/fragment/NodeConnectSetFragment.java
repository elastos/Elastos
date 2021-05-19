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
import org.elastos.wallet.ela.ui.Assets.adapter.NodeConnectSetRecAdapetr;
import org.elastos.wallet.ela.ui.Assets.presenter.AssetsPresenter;
import org.elastos.wallet.ela.ui.Assets.viewdata.AssetsViewData;
import org.elastos.wallet.ela.ui.common.listener.CommonRvListener;

import java.util.List;

import butterknife.BindView;
import butterknife.Unbinder;

public class NodeConnectSetFragment extends BaseFragment implements  CommonRvListener {
    @BindView(R.id.tv_title)
    TextView tvTitle;
    @BindView(R.id.rv)
    RecyclerView rv;
    Unbinder unbinder;

private List<SubWallet> subWallets;

    @Override
    protected int getLayoutId() {
        return R.layout.fragment_nodeconnectset;
    }

    @Override
    protected void setExtraData(Bundle data) {

        subWallets=data.getParcelableArrayList("subWallets");
        setRecycleView(subWallets);
    }

    @Override
    protected void initView(View view) {
        tvTitle.setText(R.string.nodeconnectsetting);

    }



    private void setRecycleView(List<SubWallet> assetList) {


        if (assetList == null || assetList.size() == 0) {
            return;
        }
        NodeConnectSetRecAdapetr assetskAdapter = new NodeConnectSetRecAdapetr(getContext(), assetList);
        rv.setAdapter(assetskAdapter);
        rv.setLayoutManager(new LinearLayoutManager(getContext()));
        rv.setHasFixedSize(true);
        rv.setNestedScrollingEnabled(false);
        rv.setFocusableInTouchMode(false);
        assetskAdapter.setCommonRvListener(this);

    }

    @Override
    public void onRvItemClick(int position, Object o) {
        //SubWallet
        Bundle bundle = new Bundle();
        bundle.putParcelable("subWallet", (SubWallet) o);
        start(NodeConnectSetDetailFragment.class, bundle);
    }
}
