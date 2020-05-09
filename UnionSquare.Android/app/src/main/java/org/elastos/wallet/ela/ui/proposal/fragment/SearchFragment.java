package org.elastos.wallet.ela.ui.proposal.fragment;

import android.support.v7.widget.LinearLayoutManager;
import android.support.v7.widget.RecyclerView;
import android.text.TextUtils;
import android.view.View;
import android.widget.EditText;
import android.widget.TextView;

import org.elastos.wallet.R;
import org.elastos.wallet.ela.base.BaseFragment;
import org.elastos.wallet.ela.ui.common.listener.CommonRvListener;
import org.elastos.wallet.ela.ui.proposal.adapter.ProposalRecAdapetr;

import java.util.ArrayList;

import butterknife.BindView;
import butterknife.OnClick;

public class SearchFragment extends BaseFragment implements CommonRvListener {

    @BindView(R.id.et_search)
    EditText etSearch;
    @BindView(R.id.tv_search)
    TextView tvSearch;
    @BindView(R.id.tv_nosearch)
    TextView tvNosearch;
    @BindView(R.id.rv)
    RecyclerView rv;


    @Override
    protected int getLayoutId() {
        return R.layout.fragment_proposal_search;
    }

    @Override
    protected void initView(View view) {

    }

    @OnClick({R.id.tv_search})
    public void onViewClicked(View view) {
        switch (view.getId()) {
            case R.id.tv_search:
                //查找

                String searchInput = etSearch.getText().toString();
                if (TextUtils.isEmpty(searchInput)) {
                    return;
                }

                setRecycleView();
                break;


        }
    }

    private void setRecycleView() {

        ArrayList<String> list = new ArrayList<String>();
        list.add("serch");
        list.add("serch");
        list.add("serch");
        tvNosearch.setVisibility(View.GONE);
        ProposalRecAdapetr adapter = new ProposalRecAdapetr(getContext(), list);
        rv.setLayoutManager(new LinearLayoutManager(getContext(), LinearLayoutManager.VERTICAL, false));
        rv.setAdapter(adapter);
        adapter.setCommonRvListener(this);


    }

    @Override
    public void onRvItemClick(int position, Object o) {

    }
}
