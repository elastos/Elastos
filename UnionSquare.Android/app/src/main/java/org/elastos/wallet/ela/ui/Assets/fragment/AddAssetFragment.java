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
import org.elastos.wallet.ela.db.table.SubWallet;
import org.elastos.wallet.ela.ui.Assets.adapter.AddAssetRecAdapetr;
import org.elastos.wallet.ela.ui.Assets.presenter.AddAssetPresenter;
import org.elastos.wallet.ela.ui.Assets.presenter.CommonCreateSubWalletPresenter;
import org.elastos.wallet.ela.ui.Assets.presenter.CommonDestorySubWalletPresenter;
import org.elastos.wallet.ela.ui.Assets.viewdata.AddAssetViewData;
import org.elastos.wallet.ela.ui.Assets.viewdata.CommonCreateSubWalletViewData;
import org.elastos.wallet.ela.ui.Assets.viewdata.CommonDestorySubWalletViewData;
import org.elastos.wallet.ela.ui.common.listener.CommonRvListener1;
import org.elastos.wallet.ela.ui.did.entity.DIDInfoEntity;
import org.elastos.wallet.ela.utils.CacheUtil;
import org.elastos.wallet.ela.utils.DialogUtil;
import org.elastos.wallet.ela.utils.RxEnum;
import org.elastos.wallet.ela.utils.listener.WarmPromptListener;

import java.util.ArrayList;

import butterknife.BindView;

public class AddAssetFragment extends BaseFragment implements CommonRvListener1, AddAssetViewData, CommonCreateSubWalletViewData, CommonDestorySubWalletViewData {

    @BindView(R.id.tv_title)
    TextView tvTitle;
    @BindView(R.id.rv)
    RecyclerView rv;
    private ArrayList<String> chainIds;
    private CommonCreateSubWalletPresenter commonCreateSubWalletPresenter;
    private CommonDestorySubWalletPresenter commonDestorySubWalletPresenter;
    private String walletId;

    @Override
    protected int getLayoutId() {
        return R.layout.fragment_asset_add;
    }

    @Override
    protected void setExtraData(Bundle data) {
        //    bundle.putStringArrayList("chainIds", chainIds);
        walletId = data.getString("walletId");
        chainIds = data.getStringArrayList("chainIds");

    }

    @Override
    protected void initView(View view) {
        tvTitle.setText(R.string.addassset);
        new AddAssetPresenter().getSupportedChains(walletId, this);
        commonCreateSubWalletPresenter = new CommonCreateSubWalletPresenter();
        commonDestorySubWalletPresenter = new CommonDestorySubWalletPresenter();

    }

    private void setRecycleView(String[] data, ArrayList<String> contains) {

        AddAssetRecAdapetr adapter = new AddAssetRecAdapetr(getContext(), data, contains);
        rv.setLayoutManager(new LinearLayoutManager(getContext(), LinearLayoutManager.VERTICAL, false));
        rv.setAdapter(adapter);
        adapter.setCommonRvListener(this);

    }

    @Override
    public void onRvItemClick(View v, int position, Object o) {
        //条目的点击事件
        if (v.isSelected()) {
            //删除子
            showOpenDraftWarm((String) o, v);

        } else {
            //添加子
            v.setSelected(!v.isSelected());
            commonCreateSubWalletPresenter.createSubWallet(walletId, (String) o, this);
        }
    }

    private void showOpenDraftWarm(String o, View v) {
        new DialogUtil().showCommonWarmPrompt(getBaseActivity(), getString(R.string.closeidchainornot),
                getString(R.string.sure), getString(R.string.cancel), false, new WarmPromptListener() {
                    @Override
                    public void affireBtnClick(View view) {
                        v.setSelected(!v.isSelected());
                        commonDestorySubWalletPresenter.destroySubWallet(walletId, (String) o, AddAssetFragment.this);
                    }
                });
    }

    @Override
    public void onGetSupportedChains(String[] data) {
        //获得支持的币种
        setRecycleView(data, chainIds);
    }

    @Override
    public void onCreateSubWallet(String data) {
        new RealmUtil().updateSubWalletDetial(walletId, data, new RealmTransactionAbs() {
            @Override
            public void onSuccess() {
                post(RxEnum.UPDATAPROPERTY.ordinal(), null, walletId);
                popBackFragment();
            }
        });

    }

    @Override
    public void onDestorySubWallet(String data) {

        SubWallet subWallet = new SubWallet();
        subWallet.setBelongId(walletId);
        new RealmUtil().deleteSubWallet(walletId, data);
        //删除did草稿
        ArrayList<DIDInfoEntity> info = CacheUtil.getDIDInfoList();
        for (DIDInfoEntity entity : info) {
            if (entity.getWalletId().equals(walletId)) {
                info.remove(entity);
            }
        }
        CacheUtil.setDIDInfoList(info);
        post(RxEnum.UPDATAPROPERTY.ordinal(), null, walletId);
        popBackFragment();
    }
}
