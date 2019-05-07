package org.elastos.wallet.ela.ui.Assets.fragment;

import android.os.Bundle;
import android.support.v7.widget.LinearLayoutManager;
import android.support.v7.widget.RecyclerView;
import android.view.View;
import android.widget.TextView;

import org.elastos.wallet.R;
import org.elastos.wallet.ela.base.BaseFragment;
import org.elastos.wallet.ela.db.RealmUtil;
import org.elastos.wallet.ela.db.listener.RealmTransactionAbs;
import org.elastos.wallet.ela.db.table.Wallet;
import org.elastos.wallet.ela.ui.Assets.adapter.WalletListRecAdapetr;
import org.elastos.wallet.ela.ui.common.listener.CommonRvListener;
import org.elastos.wallet.ela.utils.Constant;
import org.elastos.wallet.ela.utils.RxEnum;

import java.util.List;

import butterknife.BindView;
import butterknife.OnClick;

public class WalletListFragment extends BaseFragment implements CommonRvListener {
    @BindView(R.id.tv_title)
    TextView tvTitle;
    @BindView(R.id.rv)
    RecyclerView rv;
    private RealmUtil realmUtil;


    @Override
    protected int getLayoutId() {
        return R.layout.fragment_wallet_list;
    }

    @Override
    protected void initInjector() {

    }

    @Override
    protected void initView(View view) {
        realmUtil = new RealmUtil();
        List<Wallet> wallets = realmUtil.queryUserAllWallet();
        tvTitle.setText(R.string.walletlist);
        setRecycleView(wallets);
    }

    private void setRecycleView(List<Wallet> list) {
        WalletListRecAdapetr adapter = new WalletListRecAdapetr(getContext(), list);
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

    @Override
    public void onRvItemClick(int position, Object o) {
        //条目的点击事件
        realmUtil.updateWalletDefault(((Wallet) o).getWalletId(), new RealmTransactionAbs() {
            @Override
            public void onSuccess() {
                post(RxEnum.WALLETUPDATE.ordinal(), "", o);
                popBackFragment();
            }
        });

    }
}
