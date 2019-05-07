package org.elastos.wallet.ela.ui.Assets.fragment;

import android.support.v7.widget.LinearLayoutManager;
import android.support.v7.widget.RecyclerView;
import android.view.View;
import android.widget.TextView;

import org.elastos.wallet.R;
import org.elastos.wallet.ela.base.BaseFragment;
import org.elastos.wallet.ela.ui.Assets.adapter.ChooseSideAddressRecAdapter;
import org.elastos.wallet.ela.ui.common.listener.CommonRvListener;
import org.elastos.wallet.ela.utils.RxEnum;

import java.util.ArrayList;
import java.util.List;

import butterknife.BindView;
import butterknife.Unbinder;

public class ChooseSideChainFragment extends BaseFragment implements CommonRvListener {
    @BindView(R.id.tv_title)
    TextView tvTitle;
    @BindView(R.id.rv)
    RecyclerView rv;
    Unbinder unbinder;
    private List<String> list;
    private ChooseSideAddressRecAdapter adapter;

    @Override
    protected int getLayoutId() {
        return R.layout.fragment_contact_choose;
    }

    @Override
    protected void initInjector() {

    }

    @Override
    protected void initView(View view) {
        tvTitle.setText(R.string.choosesidechain);
        list = new ArrayList<>();
        list.add("IdChain");
        setRecycleView();
    }

    private void setRecycleView() {
        if (adapter == null) {
            adapter = new ChooseSideAddressRecAdapter(getContext(), list);
            rv.setLayoutManager(new LinearLayoutManager(getContext(), LinearLayoutManager.VERTICAL, false));
            rv.setAdapter(adapter);
            adapter.setCommonRvListener(this);

        } else {
            adapter.notifyDataSetChanged();
        }
    }

    @Override
    public void onRvItemClick(int position, Object o) {
        post(RxEnum.CHOSESIDECHAIN.ordinal(), "选择侧链地址", list.get(position));
        popBackFragment();
    }
}
