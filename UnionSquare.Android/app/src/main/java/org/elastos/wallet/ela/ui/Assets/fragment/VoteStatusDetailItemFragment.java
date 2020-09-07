package org.elastos.wallet.ela.ui.Assets.fragment;

import android.os.Bundle;
import android.support.v7.widget.LinearLayoutManager;
import android.support.v7.widget.RecyclerView;
import android.view.View;
import android.widget.LinearLayout;
import android.widget.TextView;

import org.elastos.wallet.R;
import org.elastos.wallet.ela.ElaWallet.MyWallet;
import org.elastos.wallet.ela.base.BaseFragment;
import org.elastos.wallet.ela.ui.Assets.adapter.votestatus.DeposRecAdapetr;
import org.elastos.wallet.ela.ui.Assets.bean.VoteStatus;
import org.elastos.wallet.ela.ui.proposal.presenter.ProposalDetailPresenter;
import org.elastos.wallet.ela.utils.Arith;
import org.elastos.wallet.ela.utils.DateUtil;

import java.util.ArrayList;

import butterknife.BindView;

public class VoteStatusDetailItemFragment extends BaseFragment {


    @BindView(R.id.tv_type)
    TextView tvType;
    @BindView(R.id.tv_votetime)
    TextView tvVotetime;
    @BindView(R.id.tv_resttime)
    TextView tvResttime;
    @BindView(R.id.tv_ticketnum)
    TextView tvTicketnum;
    @BindView(R.id.tv_ticketnum_dec)
    TextView tv_ticketnumDec;
    @BindView(R.id.tv_rvtitle)
    TextView tvRvtitle;
    @BindView(R.id.rv)
    RecyclerView rv;
    @BindView(R.id.ll_restTime)
    LinearLayout llRestTime;
    @BindView(R.id.ll_ticketnum)
    LinearLayout llTicketnum;

    private VoteStatus voteStatus;


    @Override
    protected int getLayoutId() {
        return R.layout.fragment_votestatus_detail_item;
    }

    @Override
    protected void setExtraData(Bundle data) {

        voteStatus = data.getParcelable("voteStatus");

        if (voteStatus == null) return;
        tvType.setText(voteStatus.getName());
        tvVotetime.setText(DateUtil.time(voteStatus.getTime(), getContext()));
        String type = voteStatus.getType();
        int status = voteStatus.getStatus();
        //0没有投票   1 有投票部分失效 2 有投票完全失效 3有投票无失效
        switch (type) {
            case "Delegate":
                tvRvtitle.setText(getString(R.string.nodelist) + " (" + voteStatus.getData().size() + ")");
                rv.setBackgroundResource(R.drawable.sc_10ffffff_cr5);
                tvResttime.setText("--");
                tvTicketnum.setText(Arith.div(voteStatus.getCount(), MyWallet.RATE_S, 8).longValue() + " ELA");
                break;

            case "CRC":
                tvRvtitle.setText(getString(R.string.votelist) + " (" + voteStatus.getData().size() + ")");
                rv.setBackgroundResource(R.drawable.sc_10ffffff_cr5);
                if (status == 2) {
                    tvResttime.setText(R.string.expired);
                    tvResttime.setBackgroundResource(R.drawable.sc_white20_cr2_stc_ffffff);
                } else
                    tvResttime.setText(ProposalDetailPresenter.setRestDay(voteStatus.getExpire(), getContext()));
                tv_ticketnumDec.setText(R.string.ticketcount);
                tvTicketnum.setText(Arith.div(voteStatus.getCount(), MyWallet.RATE_S, 8).longValue() + " ELA");

                break;
            case "CRCImpeachment":
                tvRvtitle.setText(getString(R.string.impeachmentlist));
                if (status == 2) {
                    tvResttime.setText(R.string.expired);
                    tvResttime.setBackgroundResource(R.drawable.sc_white20_cr2_stc_ffffff);
                } else
                    tvResttime.setText(ProposalDetailPresenter.setRestDay(voteStatus.getExpire(), getContext()));
                llTicketnum.setVisibility(View.GONE);

                break;
            case "CRCProposal":
                tvRvtitle.setText(R.string.againstlist);
                llRestTime.setVisibility(View.GONE);
                llTicketnum.setVisibility(View.GONE);
                break;
        }
        setDeposRecyclerView(type, voteStatus.getData());

    }

    private void setDeposRecyclerView(String type, ArrayList<Object> data) {
        if (data == null || data.size() == 0) {
            return;
        }
        DeposRecAdapetr deposRecAdapetr = new DeposRecAdapetr(getContext(), type, data);
        rv.setLayoutManager(new LinearLayoutManager(getContext(), LinearLayoutManager.VERTICAL, false));
        rv.setAdapter(deposRecAdapetr);

    }

    public static VoteStatusDetailItemFragment getInstance(VoteStatus voteStatus) {
        VoteStatusDetailItemFragment detailItemFragment = new VoteStatusDetailItemFragment();
        Bundle bundle = new Bundle();
        bundle.putParcelable("voteStatus", voteStatus);
        detailItemFragment.setArguments(bundle);
        return detailItemFragment;
    }

    @Override
    protected void initView(View view) {
        mRootView.setBackgroundResource(R.drawable.sc_80000000_stc_ffffff);
    }


}
