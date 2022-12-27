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

package org.elastos.wallet.ela.ui.Assets.activity;

import android.content.Intent;
import android.graphics.Color;
import android.graphics.drawable.ColorDrawable;
import android.support.v7.widget.LinearLayoutManager;
import android.support.v7.widget.RecyclerView;
import android.text.TextUtils;
import android.view.View;
import android.view.ViewGroup;

import org.elastos.wallet.R;
import org.elastos.wallet.ela.ElaWallet.MyWallet;
import org.elastos.wallet.ela.ElaWallet.WalletNet;
import org.elastos.wallet.ela.MyApplication;
import org.elastos.wallet.ela.base.BaseActivity;
import org.elastos.wallet.ela.db.table.SubWallet;
import org.elastos.wallet.ela.rxjavahelp.BaseEntity;
import org.elastos.wallet.ela.rxjavahelp.NewBaseViewData;
import org.elastos.wallet.ela.ui.Assets.bean.IPEntity;
import org.elastos.wallet.ela.ui.Assets.presenter.IPPresenter;
import org.elastos.wallet.ela.ui.common.bean.CommmonBooleanEntity;
import org.elastos.wallet.ela.utils.AndroidWorkaround;
import org.elastos.wallet.ela.utils.AppUtlis;
import org.elastos.wallet.ela.utils.CacheUtil;
import org.elastos.wallet.ela.utils.ClearEditText;
import org.elastos.wallet.ela.utils.Log;
import org.elastos.wallet.ela.utils.RxEnum;
import org.elastos.wallet.ela.utils.adpter.TextAdapter;

import java.util.List;

import butterknife.BindView;
import butterknife.OnClick;

public class IPAddressActivity extends BaseActivity implements NewBaseViewData, TextAdapter.OnItemClickListner, View.OnClickListener {
    @BindView(R.id.et_port)
    ClearEditText etPort;
    @BindView(R.id.et_ip)
    ClearEditText etIp;
    @BindView(R.id.rv)
    RecyclerView recyclerView;
    private SubWallet subWallet;
    private List<IPEntity> ips;

    @Override
    protected int getLayoutId() {
        if (AndroidWorkaround.checkDeviceHasNavigationBar(this)) {
            AndroidWorkaround.assistActivity(findViewById(android.R.id.content));
        }
        return R.layout.activity_ip;
    }

    @Override
    protected void initView() {

        getWindow().setBackgroundDrawable(new ColorDrawable(Color.TRANSPARENT));
        //一定要在setContentView之后调用，否则无效
        getWindow().setLayout(ViewGroup.LayoutParams.MATCH_PARENT, ViewGroup.LayoutParams.MATCH_PARENT);
        ips = CacheUtil.getIps();
        if (ips.size() > 0) {
            recyclerView.setLayoutManager(new LinearLayoutManager(this, LinearLayoutManager.VERTICAL, false));
            TextAdapter adapter = new TextAdapter(ips, this);
            adapter.setOnItemOnclickListner(this);
            recyclerView.setAdapter(adapter);
            etIp.setOnClickListener(this);
        } else {
            recyclerView.setVisibility(View.INVISIBLE);
        }
    }


    @Override
    protected void setExtraData(Intent data) {
        subWallet = data.getParcelableExtra("subWallet");
    }

    @OnClick({R.id.tv_sure})
    public void onViewClicked(View view) {
        switch (view.getId()) {
            case R.id.tv_sure:
                //确定
                String ip = etIp.getText().toString().trim();
                if (TextUtils.isEmpty(ip)) {
                    showToastMessage(getString(R.string.plzinputnodeaddress));
                    return;
                }
                //验证是否是ip或者域名格式
                if (!AppUtlis.isURL(ip)) {
                    showToastMessage(getString(R.string.addressformarterro));
                    return;
                }
                int port = getDefaultPort();
                if (!TextUtils.isEmpty(etPort.getText())) {
                    try {
                        port = Integer.parseInt(etPort.getText().toString().trim());
                    } catch (Exception e) {
                        Log.i(this.getClass().getSimpleName(), "port" + e.getMessage());
                    }

                }


                //ping一下
               // new IPPresenter().ping(new IPEntity(ip, port), this);
                new IPPresenter().setFixedPeer(subWallet.getBelongId(), subWallet.getChainId(),new IPEntity(ip, port), this);

                break;

        }
    }


    @Override
    public void onGetData(String methodName, BaseEntity baseEntity, Object o) {
        CommmonBooleanEntity commmonBooleanEntity = (CommmonBooleanEntity) baseEntity;
        switch (methodName) {
            case "ping":

                if (commmonBooleanEntity.getData()) {
                    new IPPresenter().setFixedPeer(subWallet.getBelongId(), subWallet.getChainId(), (IPEntity) o, this);
                } else {
                    showToastMessage(getString(R.string.currentaddressnotconnect));
                }
                break;
            case "setFixedPeer":
                if (!commmonBooleanEntity.getData()) {
                    showToastMessage(getString(R.string.currentaddressnotconnect));
                } else {
                    if (!ips.contains(o)) {
                        ips.add((IPEntity) o);
                    }
                    post(RxEnum.IPVALID.ordinal(), null, null);
                    finish();
                }
                break;
        }
    }

    @Override
    public void onDestroy() {
        CacheUtil.setIps(ips);
        super.onDestroy();
    }

    @Override
    public void onItemClick(View v, int position, IPEntity ipEntity) {
        etIp.setText(ipEntity.getAddress());
        etPort.setText(ipEntity.getPort() + "");
        recyclerView.setVisibility(View.INVISIBLE);
    }


    @Override
    public void onClick(View v) {
        recyclerView.setVisibility(View.VISIBLE);
    }

    private int getDefaultPort() {
        int port = 0;
        switch (MyApplication.currentWalletNet) {
            case WalletNet.MAINNET:
            case WalletNet.ALPHAMAINNET:
                port = getPort(20338, 20608);
                break;
            case WalletNet.TESTNET:
                port = getPort(21338, 21608);
                break;
            case WalletNet.REGTESTNET:
                port = getPort(22338, 22608);
                break;
            default:
                showToastMessage("please set por" + subWallet.getChainId());
                break;
        }
        return port;
    }

    private int getPort(int elaPort, int IDChainPort) {
        switch (subWallet.getChainId()) {
            case MyWallet.IDChain:
                return elaPort;
            case MyWallet.ELA:
                return IDChainPort;
            default:
                showToastMessage("please set por" + subWallet.getChainId());
                return 0;

        }
    }


}
