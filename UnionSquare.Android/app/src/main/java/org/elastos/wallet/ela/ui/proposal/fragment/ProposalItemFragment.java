package org.elastos.wallet.ela.ui.proposal.fragment;

import android.os.Bundle;
import android.support.v7.widget.LinearLayoutManager;
import android.support.v7.widget.RecyclerView;
import android.view.View;

import org.elastos.wallet.R;
import org.elastos.wallet.ela.base.BaseFragment;
import org.elastos.wallet.ela.ui.common.listener.CommonRvListener;
import org.elastos.wallet.ela.ui.proposal.ProposalFragment;
import org.elastos.wallet.ela.ui.proposal.adapter.ProposalRecAdapetr;
import org.elastos.wallet.ela.utils.Log;

import java.util.ArrayList;

import butterknife.BindView;

;

//承载提案首页viewpager控制的各个item的Fragment
public class ProposalItemFragment extends BaseFragment implements CommonRvListener {
    @BindView(R.id.rv)
    RecyclerView rv;
    private int tag;

    @Override
    protected int getLayoutId() {
        return R.layout.fragment_project_item;
    }

    @Override
    protected void setExtraData(Bundle data) {
        super.setExtraData(data);
        tag = data.getInt("TAG");
    }

    @Override
    protected void initView(View view) {
        mRootView.setBackgroundResource(0);
        setRecycleView();
    }


    private void setRecycleView() {
        Log.i("????", tag + "");
        ArrayList<String> list = new ArrayList<String>();
        list.add(tag + "");
        list.add(tag + "");
        list.add(tag + "");
        ProposalRecAdapetr adapter = new ProposalRecAdapetr(getContext(), list);
        adapter.setCommonRvListener(this);
        rv.setLayoutManager(new LinearLayoutManager(getContext(), LinearLayoutManager.VERTICAL, false));
        rv.setAdapter(adapter);
    }

    @Override
    public void onRvItemClick(int position, Object o) {
        //  start(PropasalReviewFragment.class);
        // loadRootFragment(R.id.mhoneframeLayout, new PropasalReviewFragment());
        Bundle bundle=new Bundle();
        bundle.putInt("TAG",tag);
        findFragment(ProposalFragment.class).start(PropasalReviewFragment.class,bundle);

    }

    public static ProposalItemFragment getInstance(int tag) {
        ProposalItemFragment proposalItemFragment = new ProposalItemFragment();
        Bundle bundle = new Bundle();
        bundle.putInt("TAG", tag);
        proposalItemFragment.setExtraData(bundle);
        return proposalItemFragment;
    }
}
