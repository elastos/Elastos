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
import org.elastos.wallet.ela.ui.Assets.presenter.IPPresenter;
import org.elastos.wallet.ela.ui.common.bean.CommmonBooleanEntity;
import org.elastos.wallet.ela.utils.AndroidWorkaround;
import org.elastos.wallet.ela.utils.AppUtlis;
import org.elastos.wallet.ela.utils.CacheUtil;
import org.elastos.wallet.ela.utils.ClearEditText;
import org.elastos.wallet.ela.utils.RxEnum;
import org.elastos.wallet.ela.utils.adpter.TextAdapter;

import java.util.List;

import butterknife.BindView;
import butterknife.OnClick;

public class IPAddressActivity extends BaseActivity implements NewBaseViewData, TextAdapter.OnItemClickListner, View.OnClickListener {
    @BindView(R.id.et_ip)
    ClearEditText etIp;
    @BindView(R.id.rv)
    RecyclerView recyclerView;
    private SubWallet subWallet;
    private List<String> ips;

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

                //ping一下
                new IPPresenter().ping(ip, this);
                break;

        }
    }


    @Override
    public void onGetData(String methodName, BaseEntity baseEntity, Object o) {
        CommmonBooleanEntity commmonBooleanEntity = (CommmonBooleanEntity) baseEntity;
        switch (methodName) {
            case "ping":

                if (commmonBooleanEntity.getData()) {
                    new IPPresenter().setFixedPeer(subWallet.getBelongId(), subWallet.getChainId(), (String) o, 0, this);
                } else {
                    showToastMessage(getString(R.string.currentaddressnotconnect));
                }
                break;
            case "setFixedPeer":
                if (!commmonBooleanEntity.getData()) {
                    showToastMessage(getString(R.string.currentaddressnotconnect));
                } else {
                    if (!ips.contains(o)) {
                        ips.add((String) o);
                    }
                   /* ips.add("192.168.0.2");
                    ips.add("192.168.0.3");
                    ips.add("192.168.0.4");
                    ips.add("192.168.0.5");
                    ips.add("192.168.0.6");
                    ips.add("192.168.0.7");
                    ips.add("192.168.0.8");
                    ips.add("192.168.0.11");
                    ips.add("192.168.0.111");
                    ips.add("192.168.0.1111");*/

                    post(RxEnum.IPVALID.ordinal(), null, null);
                    finish();
                }
                break;
        }
    }

    @Override
    public void onDestroy() {
        super.onDestroy();
        CacheUtil.setIps(ips);
    }

    @Override
    public void onItemClick(View v, int position, String text) {
        etIp.setText(text);
        recyclerView.setVisibility(View.INVISIBLE);
    }


    @Override
    public void onClick(View v) {
        recyclerView.setVisibility(View.VISIBLE);
    }

    private int getDefaultPort(String chainId) {
        int port = 0;
        switch (MyApplication.currentWalletNet) {
            case WalletNet.MAINNET:
            case WalletNet.ALPHAMAINNET:
                port = getPort(20338, 20608, chainId);
                break;
            case WalletNet.TESTNET:
                port = getPort(21338, 21608, chainId);
                break;
            case WalletNet.REGTESTNET:
                port = getPort(22338, 22608, chainId);
                break;
            default:
                showToastMessage("please set por" + chainId);
                break;
        }
        return port;
    }

    private int getPort(int elaPort, int IDChainPort, String chainId) {
        switch (chainId) {
            case MyWallet.IDChain:
                return elaPort;
            case MyWallet.ELA:
                return IDChainPort;
            default:
                showToastMessage("please set por" + chainId);
                return 0;

        }
    }
}
