package org.elastos.wallet.ela.ui.council;

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
import org.elastos.wallet.ela.ui.common.listener.CommonRvListener;
import org.elastos.wallet.ela.ui.council.adapter.CouncilListRecAdapetr;
import org.elastos.wallet.ela.ui.proposal.adapter.ProposalRecAdapetr;

import java.util.ArrayList;

import butterknife.BindView;
import butterknife.ButterKnife;
import butterknife.Unbinder;

public class CRCouncilFragment extends BaseFragment implements CommonRvListener {
    @BindView(R.id.tv_title)
    TextView tvTitle;
    @BindView(R.id.iv_title_right)
    ImageView ivTitleRight;
    @BindView(R.id.rv)
    RecyclerView rv;
    Unbinder unbinder;

    @Override
    protected int getLayoutId() {
        return R.layout.fragment_councial_list;
    }

    @Override
    protected void initView(View view) {
        setRecycleView();
    }

    private void setRecycleView() {

        ArrayList<String> list = new ArrayList<String>();
        list.add("1");
        list.add("2");
        list.add("1");
        CouncilListRecAdapetr adapter = new CouncilListRecAdapetr(getContext(), list);
        adapter.setCommonRvListener(this);
        rv.setLayoutManager(new LinearLayoutManager(getContext(), LinearLayoutManager.VERTICAL, false));
        rv.setAdapter(adapter);
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

    @Override
    public void onRvItemClick(int position, Object o) {

    }
}
