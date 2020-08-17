package org.elastos.wallet.ela.ui.Assets.fragment;

import android.os.Bundle;
import android.support.v4.view.ViewPager;
import android.view.View;
import android.widget.LinearLayout;
import android.widget.TextView;

import org.elastos.wallet.R;
import org.elastos.wallet.ela.base.BaseFragment;
import org.elastos.wallet.ela.ui.Assets.bean.VoteStatus;
import org.elastos.wallet.ela.ui.committee.adaper.CtListPagerAdapter;
import org.elastos.wallet.ela.utils.ScreenUtil;

import java.util.ArrayList;
import java.util.List;

import butterknife.BindView;

public class VoteStatusDetailFragment extends BaseFragment {
    @BindView(R.id.tv_title)
    TextView tvTitle;
    @BindView(R.id.viewpage)
    ViewPager viewpage;
    @BindView(R.id.ll_node)
    LinearLayout llNode;
    private ArrayList<VoteStatus> listVoteStatus;

    @Override
    protected int getLayoutId() {
        return R.layout.fragment_votestatus_detail;
    }

    @Override
    protected void setExtraData(Bundle data) {

        listVoteStatus = data.getParcelableArrayList("listVoteStatus");
        if (listVoteStatus != null && listVoteStatus.size() > 0) {
            List<BaseFragment> fragmentList = new ArrayList();
            for (VoteStatus voteStatus : listVoteStatus) {
                if (voteStatus.getStatus() != 0)
                    //0没有投票   1 有投票部分失效 2 有投票完全失效 3有投票无失效
                    fragmentList.add(VoteStatusDetailItemFragment.getInstance(voteStatus));
            }
            viewpage.setAdapter(new CtListPagerAdapter(getFragmentManager(), fragmentList));
            viewpage.setPageMargin(ScreenUtil.dp2px(getContext(), 15));
        }

    }

    @Override
    protected void initView(View view) {
        tvTitle.setText(R.string.votedetail);
    }


}
