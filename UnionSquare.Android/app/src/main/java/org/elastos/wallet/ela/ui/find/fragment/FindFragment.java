package org.elastos.wallet.ela.ui.find.fragment;

import android.os.Bundle;
import android.support.v7.widget.LinearLayoutManager;
import android.support.v7.widget.RecyclerView;
import android.view.View;
import android.widget.ImageView;
import android.widget.TextView;
import android.widget.Toast;

import org.elastos.wallet.R;
import org.elastos.wallet.ela.base.BaseFragment;
import org.elastos.wallet.ela.ui.common.listener.CommonRvListener;
import org.elastos.wallet.ela.ui.crvote.CRListFragment;
import org.elastos.wallet.ela.ui.find.FindListRecAdapter;
import org.elastos.wallet.ela.ui.vote.SuperNodeList.SuperNodeListFragment;
import org.elastos.wallet.ela.utils.SPUtil;

import java.util.ArrayList;
import java.util.List;

import butterknife.BindView;

public class FindFragment extends BaseFragment implements CommonRvListener {
    @BindView(R.id.iv_title_left)
    ImageView ivTitleLeft;
    @BindView(R.id.tv_title)
    TextView tvTitle;
    @BindView(R.id.rv)
    RecyclerView rv;
    private FindListRecAdapter adapter;
    private List<Integer> list;
    private SPUtil sp;

    @Override
    protected int getLayoutId() {
        return R.layout.fragment_find;
    }

    @Override
    protected void initInjector() {

    }

    @Override
    protected void initView(View view) {
        tvTitle.setText(getString(R.string.social));
        sp = new SPUtil(getContext());
        ivTitleLeft.setVisibility(View.GONE);
        list = new ArrayList<>();
        if (sp.getLanguage() == 0) {
            list.add(R.mipmap.found_card_vote);
            list.add(R.mipmap.found_card_vote);
        } else {
            list.add(R.mipmap.eg_found_card_vote);
            list.add(R.mipmap.eg_found_card_vote);
        }
        //  list.add(R.mipmap.found_card_id);
        //list.add(R.mipmap.found_card_paradrop);
//        presenter = new FindPresenter();
//        presenter.getSupportedChains(wallet.getWalletId(), MyWallet.ELA, this);
        setRecycleView();
    }

    public static FindFragment newInstance() {
        Bundle args = new Bundle();
        FindFragment fragment = new FindFragment();
        fragment.setArguments(args);
        return fragment;
    }

    private void setRecycleView() {
        if (adapter == null) {
            adapter = new FindListRecAdapter(getContext(), list);
            rv.setLayoutManager(new LinearLayoutManager(getContext(), LinearLayoutManager.VERTICAL, false));
            rv.setAdapter(adapter);
            adapter.setCommonRvListener(this);

        } else {
            adapter.notifyDataSetChanged();
        }
    }

    @Override
    public void onRvItemClick(int position, Object o) {
        if (position == 0) {
            ((BaseFragment) getParentFragment()).start(SuperNodeListFragment.class);
        } else if (position == 1) {
            ((BaseFragment) getParentFragment()).start(CRListFragment.class);
        }
    }

    /**
     * 处理回退事件
     *
     * @return
     */
    @Override
    public boolean onBackPressedSupport() {
        return closeApp();
    }


}
