package org.elastos.wallet.ela.ui.did.fragment;

import android.os.Bundle;
import android.support.v7.widget.LinearLayoutManager;
import android.support.v7.widget.RecyclerView;
import android.view.View;
import android.widget.ImageView;
import android.widget.TextView;

import com.alibaba.fastjson.JSON;

import org.elastos.wallet.R;
import org.elastos.wallet.ela.ElaWallet.MyWallet;
import org.elastos.wallet.ela.base.BaseFragment;
import org.elastos.wallet.ela.db.RealmUtil;
import org.elastos.wallet.ela.db.table.SubWallet;
import org.elastos.wallet.ela.db.table.Wallet;
import org.elastos.wallet.ela.rxjavahelp.BaseEntity;
import org.elastos.wallet.ela.rxjavahelp.NewBaseViewData;
import org.elastos.wallet.ela.ui.common.bean.CommmonStringEntity;
import org.elastos.wallet.ela.ui.common.bean.ISubWalletListEntity;
import org.elastos.wallet.ela.ui.common.listener.CommonRvListener;
import org.elastos.wallet.ela.ui.did.adapter.DIDNetRecordRecAdapetr;
import org.elastos.wallet.ela.ui.did.adapter.DIDRecordRecAdapetr;
import org.elastos.wallet.ela.ui.did.entity.DIDInfoEntity;
import org.elastos.wallet.ela.ui.did.entity.DIDListEntity;
import org.elastos.wallet.ela.ui.did.presenter.AddDIDPresenter;
import org.elastos.wallet.ela.ui.did.presenter.DIDListPresenter;
import org.elastos.wallet.ela.utils.CacheUtil;

import java.util.ArrayList;
import java.util.List;

import butterknife.BindView;
import butterknife.OnClick;
import butterknife.Unbinder;

public class DIDListFragment extends BaseFragment implements NewBaseViewData, CommonRvListener {

    @BindView(R.id.tv_title)
    TextView tvTitle;
    @BindView(R.id.line_tab1)
    View lineTab1;
    @BindView(R.id.tv_tab1)
    TextView tvTab1;

    @BindView(R.id.line_tab2)
    View lineTab2;
    @BindView(R.id.tv_tab2)
    TextView tvTab2;
    @BindView(R.id.rv)
    RecyclerView rv;
    @BindView(R.id.rv1)
    RecyclerView rv1;
    @BindView(R.id.iv_title_right)
    ImageView ivTitleRight;
    private DIDRecordRecAdapetr adapter1;
    private DIDNetRecordRecAdapetr adapter;
    ArrayList<DIDInfoEntity> draftList;
    ArrayList<DIDListEntity.DIDBean> netList;

    @Override
    protected int getLayoutId() {
        return R.layout.fragment_did_list;
    }

    @Override
    protected void setExtraData(Bundle data) {

        draftList = data.getParcelableArrayList("draftInfo");
        netList = data.getParcelableArrayList("netList");
        if (netList == null) {
            //重新获取数据
            draftList = CacheUtil.getDIDInfoList();
            List<Wallet> wallets = new RealmUtil().queryUserAllWallet();
            netList = new ArrayList<>();
            for (Wallet wallet : wallets) {
                new AddDIDPresenter().getAllSubWallets(wallet.getWalletId(), this);

            }
        }
    }

    @Override
    protected void initView(View view) {
        tvTitle.setText("DID");
        ivTitleRight.setVisibility(View.VISIBLE);
        ivTitleRight.setImageResource(R.mipmap.mine_did_add);
    }


    @OnClick({R.id.ll_tab1, R.id.ll_tab2, R.id.iv_title_right})
    public void onViewClicked(View view) {
        switch (view.getId()) {
            case R.id.iv_title_right:
                start(AddDIDFragment.class);
                break;
            case R.id.ll_tab1:
                lineTab1.setVisibility(View.VISIBLE);
                lineTab2.setVisibility(View.GONE);
                tvTab1.setTextColor(getResources().getColor(R.color.whiter));
                tvTab2.setTextColor(getResources().getColor(R.color.whiter50));
                rv.setVisibility(View.VISIBLE);
                rv1.setVisibility(View.GONE);
                setNetRecycleView();
                break;
            case R.id.ll_tab2:
                lineTab1.setVisibility(View.GONE);
                lineTab2.setVisibility(View.VISIBLE);
                tvTab1.setTextColor(getResources().getColor(R.color.whiter50));
                tvTab2.setTextColor(getResources().getColor(R.color.whiter));
                rv.setVisibility(View.GONE);
                rv1.setVisibility(View.VISIBLE);
                setRecycleView();
                break;
        }
    }


    @Override
    public void onGetData(String methodName, BaseEntity baseEntity, Object o) {
        switch (methodName) {
            case "getAllSubWallets":
                ISubWalletListEntity subWalletListEntity = (ISubWalletListEntity) baseEntity;
                for (SubWallet subWallet : subWalletListEntity.getData()) {
                    if (subWallet.getChainId().equals(MyWallet.IDChain)) {
                        new DIDListPresenter().getResolveDIDInfo((String) o, 0, 1, "", this);
                        break;
                    }
                }
                break;
            case "getResolveDIDInfo":

                DIDListEntity didListEntity = JSON.parseObject(((CommmonStringEntity) baseEntity).getData(), DIDListEntity.class);
                if (didListEntity != null && didListEntity.getDID() != null && didListEntity.getDID().size() > 0) {
                    netList.addAll(didListEntity.getDID());
                }
                break;

        }

    }

    private void setNetRecycleView() {
        if (adapter == null) {
            adapter = new DIDNetRecordRecAdapetr(getContext(), netList);
            rv.setLayoutManager(new LinearLayoutManager(getContext(), LinearLayoutManager.VERTICAL, false));
            rv.setAdapter(adapter);
            adapter.setCommonRvListener(this);

        } else {
            adapter.notifyDataSetChanged();
        }

    }

    private void setRecycleView() {
        if (adapter1 == null) {
            adapter1 = new DIDRecordRecAdapetr(getContext(), draftList);
            rv1.setLayoutManager(new LinearLayoutManager(getContext(), LinearLayoutManager.VERTICAL, false));
            rv1.setAdapter(adapter1);
            adapter1.setCommonRvListener(this);

        } else {
            adapter1.notifyDataSetChanged();
        }

    }

    @Override
    public void onRvItemClick(int position, Object o) {
        DIDInfoEntity didInfoEntity = (DIDInfoEntity) o;
        if ("Unpublished".equals(didInfoEntity.getStatus())) {
            //草稿
            Wallet wallet = new RealmUtil().queryUserWallet(didInfoEntity.getWalletId());
            if (wallet == null) {

            }
            Bundle bundle = new Bundle();
            bundle.putParcelable("didInfo", didInfoEntity);
            bundle.putBoolean("useDraft", true);

            bundle.putParcelable("wallet", wallet);
            start(AddDIDFragment.class, bundle);
        }


    }


}
