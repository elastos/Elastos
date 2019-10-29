package org.elastos.wallet.ela.ui.did.fragment;

import android.os.Bundle;
import android.support.v7.widget.LinearLayoutManager;
import android.support.v7.widget.RecyclerView;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.ImageView;
import android.widget.TextView;

import org.elastos.wallet.R;
import org.elastos.wallet.ela.base.BaseFragment;
import org.elastos.wallet.ela.rxjavahelp.BaseEntity;
import org.elastos.wallet.ela.rxjavahelp.NewBaseViewData;
import org.elastos.wallet.ela.ui.Assets.fragment.TransferDetailFragment;
import org.elastos.wallet.ela.ui.common.listener.CommonRvListener;
import org.elastos.wallet.ela.ui.did.adapter.DIDRecordRecAdapetr;
import org.elastos.wallet.ela.ui.did.entity.DIDInfoEntity;
import org.elastos.wallet.ela.utils.CacheUtil;

import java.util.ArrayList;

import butterknife.BindView;
import butterknife.ButterKnife;
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
    Unbinder unbinder;
    private DIDRecordRecAdapetr adapter1;
    private DIDRecordRecAdapetr adapter;
    ArrayList<DIDInfoEntity> draftList;
    ArrayList<DIDInfoEntity> netList;

    @Override
    protected int getLayoutId() {
        return R.layout.fragment_did_list;
    }

    @Override
    protected void setExtraData(Bundle data) {

        draftList = data.getParcelableArrayList("draftInfo");
        netList = data.getParcelableArrayList("netList");
        if (draftList == null) {
            draftList = CacheUtil.getDIDInfoList();
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
                break;
            case R.id.ll_tab2:
                lineTab1.setVisibility(View.GONE);
                lineTab2.setVisibility(View.VISIBLE);
                tvTab1.setTextColor(getResources().getColor(R.color.whiter50));
                tvTab2.setTextColor(getResources().getColor(R.color.whiter));
                rv.setVisibility(View.GONE);
                rv1.setVisibility(View.VISIBLE);
                setRecycleView1();
                break;
        }
    }


    @Override
    public void onGetData(String methodName, BaseEntity baseEntity, Object o) {
        switch (methodName) {
            case "getAllSubWallets":

                break;

        }

    }

  /*  private void setRecycleView(TransferRecordEntity entity) {
        List<TransferRecordEntity.TransactionsBean> data = entity.getTransactions();
        if (startCount == 0 && (data == null || data.size() == 0)) {
            rv.setVisibility(View.GONE);
            tvRecordBg.setVisibility(View.VISIBLE);
            return;
        } else {
            rv.setVisibility(View.VISIBLE);
            tvRecordBg.setVisibility(View.GONE);
        }

        if (startCount == 0) {
            if (list == null) {
                list = new ArrayList<>();
            } else {
                list.clear();
            }
        } else if (data == null || data.size() == 0) {
            showToastMessage(getString(R.string.loadall));
            return;
        }
        list.addAll(data);
        if (adapter == null) {
            adapter = new DIDRecordRecAdapetr(getContext(), list, chainId);
            rv.setLayoutManager(new LinearLayoutManager(getContext(), LinearLayoutManager.VERTICAL, false));
            rv.setAdapter(adapter);
            adapter.setCommonRvListener(this);

        } else {
            adapter.notifyDataSetChanged();
        }
        startCount += data.size();
    }*/

    private void setRecycleView1() {
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
        Bundle bundle = new Bundle();

        if (rv1.getVisibility() == View.VISIBLE) {
            bundle.putInt("recordType", 1);//发布记录
        } else {
            bundle.putInt("recordType", 0);//草稿记录
        }

        start(TransferDetailFragment.class, bundle);
    }

    @Override
    public View onCreateView(LayoutInflater inflater, ViewGroup container, Bundle savedInstanceState) {
        // TODO: inflate a fragment view
        View rootView = super.onCreateView(inflater, container, savedInstanceState);
        unbinder = ButterKnife.bind(this, rootView);
        return rootView;
    }

    @Override
    public void onDestroyView() {
        super.onDestroyView();
        unbinder.unbind();
    }
}
