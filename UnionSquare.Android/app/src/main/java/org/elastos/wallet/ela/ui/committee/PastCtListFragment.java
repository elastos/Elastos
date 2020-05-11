package org.elastos.wallet.ela.ui.committee;

import android.support.v7.widget.LinearLayoutManager;
import android.support.v7.widget.RecyclerView;
import android.support.v7.widget.Toolbar;
import android.view.View;
import android.widget.ImageView;
import android.widget.TextView;

import com.scwang.smartrefresh.layout.api.RefreshLayout;
import com.scwang.smartrefresh.layout.listener.OnLoadMoreListener;

import org.elastos.wallet.R;
import org.elastos.wallet.ela.base.BaseFragment;
import org.elastos.wallet.ela.rxjavahelp.BaseEntity;
import org.elastos.wallet.ela.rxjavahelp.NewBaseViewData;
import org.elastos.wallet.ela.ui.committee.adaper.PastCtRecAdapter;
import org.elastos.wallet.ela.ui.committee.bean.PastCtBean;
import org.elastos.wallet.ela.ui.committee.presenter.PastCtPresenter;
import org.elastos.wallet.ela.ui.common.listener.CommonRvListener;

import java.util.ArrayList;
import java.util.List;

import butterknife.BindView;

/**
 * List of past members
 */
public class PastCtListFragment extends BaseFragment implements NewBaseViewData, CommonRvListener, OnLoadMoreListener {
    @BindView(R.id.tv_title)
    TextView tvTitle;
    @BindView(R.id.iv_title_right)
    ImageView ivTitleRight;
    @BindView(R.id.recyclerview)
    RecyclerView recyclerview;
    PastCtRecAdapter adapter;
    List<PastCtBean> list;
    PastCtPresenter presenter;
    private int pageNum = 1;
    private final int pageSize = 1000;

    @Override
    protected int getLayoutId() {
        return R.layout.fragment_ct_past_list;
    }

    @Override
    protected void initView(View view) {
        ivTitleRight.setVisibility(View.VISIBLE);
        ivTitleRight.setImageResource(R.mipmap.found_ct_secretary_entrance);
        tvTitle.setText(mContext.getString(R.string.pastcttitle));
        presenter = new PastCtPresenter();
        presenter.getPastCtList(pageNum, pageSize, "all", this, true);
    }

    private void setRecycleView() {
        if (adapter == null) {
            adapter = new PastCtRecAdapter(getContext(), list);
            recyclerview.setLayoutManager(new LinearLayoutManager(getContext(), LinearLayoutManager.VERTICAL, false));
            recyclerview.setAdapter(adapter);
            adapter.setCommonRvListener(this);

        } else {
            adapter.notifyDataSetChanged();
        }
    }

    @Override
    public void onGetData(String methodName, BaseEntity baseEntity, Object o) {
        switch (methodName) {
            case "":
                list = new ArrayList<>();
                setRecycleView();
                break;
        }
    }

    @Override
    public void onRvItemClick(int position, Object o) {

    }

    @Override
    public void onLoadMore(RefreshLayout refreshLayout) {

    }
}
