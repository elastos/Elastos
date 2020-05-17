package org.elastos.wallet.ela.ui.proposal.fragment;

import android.content.Intent;
import android.graphics.Paint;
import android.os.Bundle;
import android.support.v7.widget.LinearLayoutManager;
import android.support.v7.widget.RecyclerView;
import android.text.TextUtils;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.ImageView;
import android.widget.LinearLayout;
import android.widget.RelativeLayout;
import android.widget.TextView;

import com.alibaba.fastjson.JSON;

import org.elastos.wallet.R;
import org.elastos.wallet.ela.base.BaseFragment;
import org.elastos.wallet.ela.ui.common.fragment.WebViewFragment;
import org.elastos.wallet.ela.ui.proposal.adapter.ProcessRecAdapetr;
import org.elastos.wallet.ela.ui.proposal.adapter.VoteRecAdapetr;
import org.elastos.wallet.ela.utils.ClipboardUtil;
import org.elastos.wallet.ela.utils.Constant;
import org.elastos.wallet.ela.utils.Log;
import org.elastos.wallet.ela.utils.QrBean;
import org.elastos.wallet.ela.utils.SPUtil;
import org.elastos.wallet.ela.utils.ScanQRcodeUtil;
import org.elastos.wallet.ela.utils.ScreenUtil;
import org.elastos.wallet.ela.utils.view.CircleProgressView;

import java.util.ArrayList;

import butterknife.BindView;
import butterknife.ButterKnife;
import butterknife.OnClick;
import butterknife.Unbinder;

public class PropasalReviewFragment extends BaseFragment {
    @BindView(R.id.tv_title)
    TextView tvTitle;
    @BindView(R.id.tv_propasal_tile)
    TextView tvPropasalTile;
    @BindView(R.id.tv_num)
    TextView tvNum;
    @BindView(R.id.tv_time)
    TextView tvTime;
    @BindView(R.id.tv_people)
    TextView tvPeople;
    @BindView(R.id.tv_status)
    TextView tvStatus;
    @BindView(R.id.iv_info)
    ImageView ivInfo;
    @BindView(R.id.tv_resttime)
    TextView tvResttime;
    @BindView(R.id.tv_hash)
    TextView tvHash;
    @BindView(R.id.tv_web)
    TextView tvWeb;
    @BindView(R.id.ll_info)
    LinearLayout llInfo;
    @BindView(R.id.abstract1)
    TextView abstract1;
    @BindView(R.id.iv_vote)
    ImageView ivVote;
    @BindView(R.id.tv_agree)
    TextView tvAgree;
    @BindView(R.id.tv_disagree)
    TextView tvDisagree;
    @BindView(R.id.tv_abstention)
    TextView tvAbstention;
    @BindView(R.id.rv_vote)
    RecyclerView rvVote;
    @BindView(R.id.rv_process)
    RecyclerView rvProcess;
    @BindView(R.id.ll_vote)
    LinearLayout llVote;
    @BindView(R.id.rl_vote)
    RelativeLayout rlVote;
    Unbinder unbinder;
    @BindView(R.id.ll_disagreeprogress)
    LinearLayout llDisagreeprogress;
    @BindView(R.id.tv_novote)
    TextView tvNovote;
    @BindView(R.id.circleIndicator)
    CircleProgressView circleIndicator;
    @BindView(R.id.currentvote)
    TextView currentvote;
    @BindView(R.id.needvote)
    TextView needvote;
    @BindView(R.id.tv_vote)
    TextView tvVote;
    @BindView(R.id.tv_votestatus)
    TextView tvVotestatus;
    @BindView(R.id.statusbarutil_fake_status_bar_view)
    View statusbarutilFakeStatusBarView;
    @BindView(R.id.iv_title_left)
    ImageView ivTitleLeft;
    @BindView(R.id.iv_title_right)
    ImageView ivTitleRight;
    @BindView(R.id.tv_title_right)
    TextView tvTitleRight;
    @BindView(R.id.ll_process)
    LinearLayout llProcess;
    private int tag;

    @Override
    protected int getLayoutId() {
        return R.layout.fragment_proposal_review;
    }

    @Override
    protected void setExtraData(Bundle data) {
        super.setExtraData(data);
        tag = data.getInt("TAG");
        int id = data.getInt("id");//提案编号
    }

    @Override
    protected void initView(View view) {
        setVoteRecycleView();
        setProcessRecycleView();
        if (tag == 2) {
            //公示期
            tvVote.setText(R.string.votedisagree);
            setInfoStatue(false);
            setDisagreeProgress(30);

        } else if (tag == 3) {
            //执行期
            llProcess.setVisibility(View.VISIBLE);
            setInfoStatue(false);
            setVoteStatue(false);

        } else if (tag == 4) {
            //已完结
            llProcess.setVisibility(View.VISIBLE);
            setInfoStatue(false);
            setVoteStatue(false);

        } else if (tag == 5) {
            //已废止
            setDisagreeProgress(100f);
            tvPropasalTile.getPaint().setFlags(Paint.STRIKE_THRU_TEXT_FLAG);//删除线


        }
    }

    private void setDisagreeProgress(float progress) {
        llDisagreeprogress.setVisibility(View.VISIBLE);
        circleIndicator.setProgress(progress);
        circleIndicator.invalidate();
        setInfoStatue(false);
    }

    private void setInfoStatue(boolean show) {
        if (!show) {
            llInfo.setVisibility(View.GONE);
            ivInfo.setImageResource(R.mipmap.cr_arrow_right);
        } else {

            llInfo.setVisibility(View.VISIBLE);
            ivInfo.setImageResource(R.mipmap.cr_arrow_down);
        }


    }

    private void setVoteStatue(boolean show) {
        if (!show) {
            tvVotestatus.setVisibility(View.GONE);
            llVote.setVisibility(View.GONE);
            ivVote.setImageResource(R.mipmap.cr_arrow_right);
        } else {
            tvVotestatus.setVisibility(View.VISIBLE);
            llVote.setVisibility(View.VISIBLE);
            ivVote.setImageResource(R.mipmap.cr_arrow_down);
        }


    }

    @OnClick({R.id.tv_agree, R.id.iv_info, R.id.iv_vote, R.id.tv_disagree, R.id.tv_abstention, R.id.tv_hash, R.id.tv_web, R.id.tv_vote})
    public void onViewClicked(View view) {
        switch (view.getId()) {
            case R.id.tv_agree:
                view.setAlpha(1);
                tvDisagree.setAlpha(0.5f);
                tvAbstention.setAlpha(0.5f);
                setMargin(view, 0, 0, 0, 0);
                setMargin(tvDisagree, ScreenUtil.dp2px(getContext(), 5), 0, 0, ScreenUtil.dp2px(getContext(), 5));
                setMargin(tvAbstention, ScreenUtil.dp2px(getContext(), 5), 0, 0, ScreenUtil.dp2px(getContext(), 5));
                break;
            case R.id.tv_disagree:
                view.setAlpha(1);
                tvAgree.setAlpha(0.5f);
                tvAbstention.setAlpha(0.5f);
                setMargin(view, ScreenUtil.dp2px(getContext(), 5), 0, 0, 0);
                setMargin(tvAgree, 0, 0, 0, ScreenUtil.dp2px(getContext(), 5));
                setMargin(tvAbstention, ScreenUtil.dp2px(getContext(), 5), 0, 0, ScreenUtil.dp2px(getContext(), 5));
                break;
            case R.id.tv_abstention:
                view.setAlpha(1);
                tvAgree.setAlpha(0.5f);
                tvDisagree.setAlpha(0.5f);
                setMargin(view, ScreenUtil.dp2px(getContext(), 5), 0, 0, 0);
                setMargin(tvDisagree, ScreenUtil.dp2px(getContext(), 5), 0, 0, ScreenUtil.dp2px(getContext(), 5));
                setMargin(tvAgree, 0, 0, 0, ScreenUtil.dp2px(getContext(), 5));
                break;
            case R.id.tv_hash:
                ClipboardUtil.copyClipboar(getBaseActivity(), tvHash.getText().toString().trim());
                break;
            case R.id.tv_web:
                String web = tvWeb.getText().toString();
                if (!TextUtils.isEmpty(web)) {
                    Bundle bundle = new Bundle();
                    bundle.putString(Constant.FRAGMENTTAG, web + "?langua=" + (new SPUtil(getContext()).getLanguage() == 0 ? "ch" : "en"));
                    start(WebViewFragment.class, bundle);
                }
                break;
            case R.id.tv_vote:
                if (tag == 1) {
                    //评议扫码投票
                    requstManifestPermission(getString(R.string.needpermission));
                } else if (tag == 2) {
                    //公示期 投票反对

                }
                break;
            case R.id.iv_info:
                if (llInfo.getVisibility() == View.VISIBLE) {
                    setInfoStatue(false);
                } else {
                    setInfoStatue(true);
                }
            case R.id.iv_vote:
                if (llVote.getVisibility() == View.VISIBLE) {
                    setVoteStatue(false);
                } else {
                    setVoteStatue(true);
                }

                break;


        }
    }

    private void setVoteRecycleView() {
        Log.i("????", tag + "");
        ArrayList<String> list = new ArrayList<String>();
        list.add(tag + "");
        list.add(tag + "");
        list.add(tag + "");
        VoteRecAdapetr adapter = new VoteRecAdapetr(getContext(), list);
        rvVote.setLayoutManager(new LinearLayoutManager(getContext(), LinearLayoutManager.VERTICAL, false));
        rvVote.setAdapter(adapter);
    }

    private void setProcessRecycleView() {
        Log.i("????", tag + "");
        ArrayList<String> list = new ArrayList<String>();
        list.add(tag + "");
        list.add(tag + "");
        list.add(tag + "");
        ProcessRecAdapetr adapter = new ProcessRecAdapetr(getContext(), list);
        rvProcess.setLayoutManager(new LinearLayoutManager(getContext(), LinearLayoutManager.VERTICAL, false));
        rvProcess.setAdapter(adapter);
    }

    @Override
    protected void requstPermissionOk() {
        new ScanQRcodeUtil().scanQRcode(this);
    }

    @Override
    public void onActivityResult(int requestCode, int resultCode, Intent data) {
        super.onActivityResult(requestCode, resultCode, data);
        //处理扫描结果（在界面上显示）
        if (resultCode == RESULT_OK && requestCode == ScanQRcodeUtil.SCAN_QR_REQUEST_CODE && data != null) {
            String result = data.getStringExtra("result");//&& matcherUtil.isMatcherAddr(result)
            if (!TextUtils.isEmpty(result) /*&& matcherUtil.isMatcherAddr(result)*/) {
                if (result.startsWith("elastos:")) {
                    return;
                }
                try {
                    QrBean qrBean = JSON.parseObject(result, QrBean.class);
                    int type = qrBean.getExtra().getType();

                } catch (Exception e) {
                }
            }
        }

    }


    private void setMargin(View view, int left, int top, int right, int bottom) {
        LinearLayout.LayoutParams lp = (LinearLayout.LayoutParams) view.getLayoutParams();
        lp.setMargins(left, top, right, bottom);
        view.setLayoutParams(lp);

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
