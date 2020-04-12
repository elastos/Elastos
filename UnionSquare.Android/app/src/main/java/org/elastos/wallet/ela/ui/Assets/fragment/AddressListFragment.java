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

import com.alibaba.fastjson.JSON;
import com.scwang.smartrefresh.layout.SmartRefreshLayout;
import com.scwang.smartrefresh.layout.api.RefreshLayout;
import com.scwang.smartrefresh.layout.listener.OnLoadMoreListener;

import org.elastos.wallet.R;
import org.elastos.wallet.ela.base.BaseFragment;
import org.elastos.wallet.ela.db.table.Wallet;
import org.elastos.wallet.ela.ui.Assets.adapter.AddressListRecAdapetr;
import org.elastos.wallet.ela.ui.Assets.bean.AddressListEntity;
import org.elastos.wallet.ela.ui.Assets.presenter.AddressListPresenter;
import org.elastos.wallet.ela.ui.common.listener.CommonRvListener;
import org.elastos.wallet.ela.ui.common.viewdata.CommmonStringViewData;
import org.elastos.wallet.ela.utils.ClipboardUtil;

import java.util.ArrayList;
import java.util.List;

import butterknife.BindView;
import butterknife.Unbinder;

public class AddressListFragment extends BaseFragment implements CommonRvListener, CommmonStringViewData, OnLoadMoreListener {
    @BindView(R.id.tv_title)
    TextView tvTitle;
    @BindView(R.id.rv)
    RecyclerView rv;
    @BindView(R.id.srl)
    SmartRefreshLayout srl;
    Unbinder unbinder;
    private AddressListRecAdapetr adapter;
    private ArrayList<String> list;
    private Wallet wallet;
    private String chainId;
    private AddressListPresenter presenter;
    private int startCount = 0;
    private final int pageCount = 20;

    @Override
    protected int getLayoutId() {
        return R.layout.fragment_address_list;
    }

    @Override
    protected void setExtraData(Bundle data) {
        wallet = data.getParcelable("wallet");
        chainId = data.getString("ChainId", "ELA");


    }

    @Override
    protected void initView(View view) {
        tvTitle.setText(getString(R.string.addresslist));
        presenter = new AddressListPresenter();
        presenter.getAllAddress(wallet.getWalletId(), chainId, startCount, pageCount, this);
        srl.setOnLoadMoreListener(this);
        initClassicsFooter();

    }

    private void setRecycleView(List<String> data) {

        if (list == null) {
            list = new ArrayList<>();

        }
        if (data == null || data.size() == 0)

        {
            showToastMessage(getString(R.string.loadall));
            return;
        }
        list.addAll(data);
        if (adapter == null)

        {
            adapter = new AddressListRecAdapetr(getContext(), list);
            rv.setLayoutManager(new LinearLayoutManager(getContext(), LinearLayoutManager.VERTICAL, false));
            rv.setAdapter(adapter);
            adapter.setCommonRvListener(this);

        } else

        {
            adapter.notifyDataSetChanged();
        }

        startCount += data.size();
    }

    @Override
    public void onRvItemClick(int position, Object o) {
        ClipboardUtil.copyClipboar(getBaseActivity(), (String) o);
    }

    @Override
    public void onGetCommonData(String data) {

        srl.finishLoadMore();
        AddressListEntity addressListEntity = JSON.parseObject(data, AddressListEntity.class);
        List<String> addressList = addressListEntity.getAddresses();
        setRecycleView(addressList);
    }

    @Override
    public void onLoadMore(RefreshLayout refreshLayout) {
        onErrorRefreshLayout(srl);
        presenter.getAllAddress(wallet.getWalletId(), chainId, startCount, pageCount, this);
    }
}
