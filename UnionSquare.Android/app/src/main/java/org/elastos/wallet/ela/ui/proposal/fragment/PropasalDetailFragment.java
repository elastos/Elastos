package org.elastos.wallet.ela.ui.proposal.fragment;

import android.content.Intent;
import android.graphics.Paint;
import android.os.Bundle;
import android.support.v7.widget.LinearLayoutManager;
import android.support.v7.widget.RecyclerView;
import android.text.TextUtils;
import android.view.View;
import android.widget.ImageView;
import android.widget.LinearLayout;
import android.widget.RelativeLayout;
import android.widget.TextView;

import org.elastos.wallet.R;
import org.elastos.wallet.ela.ElaWallet.MyWallet;
import org.elastos.wallet.ela.base.BaseFragment;
import org.elastos.wallet.ela.bean.BusEvent;
import org.elastos.wallet.ela.db.RealmUtil;
import org.elastos.wallet.ela.db.table.Wallet;
import org.elastos.wallet.ela.rxjavahelp.BaseEntity;
import org.elastos.wallet.ela.rxjavahelp.NewBaseViewData;
import org.elastos.wallet.ela.ui.Assets.activity.TransferActivity;
import org.elastos.wallet.ela.ui.Assets.bean.BalanceEntity;
import org.elastos.wallet.ela.ui.Assets.presenter.CommonGetBalancePresenter;
import org.elastos.wallet.ela.ui.Assets.viewdata.CommonBalanceViewData;
import org.elastos.wallet.ela.ui.common.bean.CommmonStringEntity;
import org.elastos.wallet.ela.ui.common.fragment.WebViewFragment;
import org.elastos.wallet.ela.ui.crvote.bean.CRListBean;
import org.elastos.wallet.ela.ui.crvote.presenter.CRlistPresenter;
import org.elastos.wallet.ela.ui.proposal.adapter.ProcessRecAdapetr;
import org.elastos.wallet.ela.ui.proposal.adapter.VoteRecAdapetr;
import org.elastos.wallet.ela.ui.proposal.bean.ProposalSearchEntity;
import org.elastos.wallet.ela.ui.proposal.presenter.ProposalDetailPresenter;
import org.elastos.wallet.ela.ui.vote.ElectoralAffairs.VoteListPresenter;
import org.elastos.wallet.ela.ui.vote.activity.VoteActivity;
import org.elastos.wallet.ela.ui.vote.bean.VoteListBean;
import org.elastos.wallet.ela.utils.Arith;
import org.elastos.wallet.ela.utils.ClipboardUtil;
import org.elastos.wallet.ela.utils.Constant;
import org.elastos.wallet.ela.utils.DialogUtil;
import org.elastos.wallet.ela.utils.Log;
import org.elastos.wallet.ela.utils.NumberiUtil;
import org.elastos.wallet.ela.utils.RxEnum;
import org.elastos.wallet.ela.utils.SPUtil;
import org.elastos.wallet.ela.utils.ScanQRcodeUtil;
import org.elastos.wallet.ela.utils.ScreenUtil;
import org.elastos.wallet.ela.utils.listener.WarmPromptListener;
import org.elastos.wallet.ela.utils.view.CircleProgressView;
import org.greenrobot.eventbus.Subscribe;
import org.greenrobot.eventbus.ThreadMode;
import org.json.JSONArray;
import org.json.JSONObject;

import java.math.BigDecimal;
import java.util.ArrayList;
import java.util.Iterator;
import java.util.List;

import butterknife.BindView;
import butterknife.OnClick;
import butterknife.Unbinder;

public class PropasalDetailFragment extends BaseFragment implements NewBaseViewData, CommonBalanceViewData {
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
    ProposalSearchEntity.DataBean.ListBean searchBean;
    int tag = 1;
    private String scanResult;
    private Wallet wallet;
    //private ProposalPresenter proposalPresenter;
    private String maxBalance;
    private ProposalDetailPresenter presenter;
    ArrayList<ProposalSearchEntity.DataBean.ListBean> searchBeanList;

    @Override
    protected int getLayoutId() {
        return R.layout.fragment_proposal_review;
    }

    @Override
    protected void setExtraData(Bundle data) {
        super.setExtraData(data);
        searchBeanList = data.getParcelableArrayList("ProposalSearchDateList");
        searchBean = searchBeanList.get(data.getInt("position"));
    }

    @Override
    protected void initView(View view) {
        setVoteRecycleView();
        setProcessRecycleView();
        wallet = new RealmUtil().queryDefauleWallet();
        presenter = new ProposalDetailPresenter();
        presenter.proposalDetail(searchBean.getId(), this);
        registReceiver();
        switch (searchBean.getStatus()) {
            case "VOTING":
                //委员评议
                tvTitle.setText(R.string.proposalcomments);
                rlVote.setVisibility(View.VISIBLE);
                break;
            case "NOTIFICATION":
                //公示期
                tvTitle.setText(R.string.proposalpublished);
                tvVote.setText(R.string.votedisagree);
                setInfoStatue(false);
                setDisagreeProgress(30);
                rlVote.setVisibility(View.VISIBLE);
                break;
            case "ACTIVE":
                //执行期;
                tvTitle.setText(R.string.proposalprocess);
                llProcess.setVisibility(View.VISIBLE);
                setInfoStatue(false);
                setVoteStatue(false);
                break;
            case "FINAL":
                //已完结
                tvTitle.setText(R.string.proposalfinish);
                llProcess.setVisibility(View.VISIBLE);
                setInfoStatue(false);
                setVoteStatue(false);
                break;
            case "REJECTED":
            case "VETOED":
                //已废止
                tvTitle.setText(R.string.proposalabandon);
                setDisagreeProgress(100f);
                tvPropasalTile.getPaint().setFlags(Paint.STRIKE_THRU_TEXT_FLAG);//删除线
                break;
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

    JSONArray otherUnActiveVote;

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
                if ("VOTING".equals(searchBean.getStatus())) {
                    //评议扫码投票
                    requstManifestPermission(getString(R.string.needpermission));
                } else if ("NOTIFICATION".equals(searchBean.getStatus())) {
                    //公示期 投票反对
                    if (otherUnActiveVote == null) {
                        otherUnActiveVote = new JSONArray();
                    }
                    new VoteListPresenter().getDepositVoteList("1", "all", this, false);
                    new CRlistPresenter().getCRlist(1, 1000, "all", this, false);
                    new CommonGetBalancePresenter().getBalance(wallet.getWalletId(), MyWallet.ELA, 2, this);
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

        ArrayList<String> list = new ArrayList<String>();
        list.add(tag + "");
        list.add(tag + "");
        list.add(tag + "");
        VoteRecAdapetr adapter = new VoteRecAdapetr(getContext(), list);
        rvVote.setLayoutManager(new LinearLayoutManager(getContext(), LinearLayoutManager.VERTICAL, false));
        rvVote.setAdapter(adapter);
    }

    private void setProcessRecycleView() {

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
            scanResult = data.getStringExtra("result");//&& matcherUtil.isMatcherAddr(result)
            if (!TextUtils.isEmpty(scanResult) /*&& matcherUtil.isMatcherAddr(result)*/) {
                if (scanResult.startsWith("elastos://crproposal/")) {
                    //兼容elastos:
                    post(RxEnum.SCANDATATOASSETPAGE.ordinal(), getClass().getSimpleName(), scanResult);//交给首页去处理
                } else {
                    showToast(getString(R.string.infoformatwrong));
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
    public void onGetData(String methodName, BaseEntity baseEntity, Object o) {
        switch (methodName) {
            case "getVoteInfo":
                //剔除非公示期的
                JSONObject newVotes = new JSONObject();
                try {
                    String amount = Arith.mulRemoveZero(num, MyWallet.RATE_S).toPlainString();
                    newVotes.put(searchBean.getProposalHash(), amount);
                    String voteInfo = ((CommmonStringEntity) baseEntity).getData();
                    if (!TextUtils.isEmpty(voteInfo) && !voteInfo.equals("null") && !voteInfo.equals("[]")) {
                        JSONArray lastVoteInfo = new JSONArray(((CommmonStringEntity) baseEntity).getData());
                        if (lastVoteInfo.length() >= 1) {
                            JSONObject lastVote = lastVoteInfo.getJSONObject(0).getJSONObject("Votes");
                            //获得上次的投票后筛选数据
                            Iterator it = lastVote.keys();
                            while (it.hasNext()) {
                                String key = (String) it.next();
                                for (int i = 0; i < searchBeanList.size(); i++) {
                                    if (searchBeanList.get(i).getProposalHash().equals(key)) {
                                        newVotes.put(key, amount);
                                        break;
                                    }
                                }
                            }
                        }
                    }


                } catch (Exception e) {
                    e.printStackTrace();
                }
                Log.i("???", otherUnActiveVote.toString());
                Log.i("???", newVotes.toString());
                presenter.createVoteCRCProposalTransaction(wallet.getWalletId(), newVotes.toString(), otherUnActiveVote.toString(), this);

                break;
            case "getCRlist":
                List<CRListBean.DataBean.ResultBean.CrcandidatesinfoBean> crList = ((CRListBean) baseEntity).getData().getResult().getCrcandidatesinfo();
                if (crList != null || crList.size() > 0) {
                    otherUnActiveVote.put(new ProposalDetailPresenter().getCRUnactiveData(crList));
                }
                break;
            case "getDepositVoteList":
                List<VoteListBean.DataBean.ResultBean.ProducersBean> depositList = ((VoteListBean) baseEntity).getData().getResult().getProducers();
                if (depositList != null || depositList.size() > 0) {
                    otherUnActiveVote.put(new ProposalDetailPresenter().getDepositUnactiveData(depositList));
                }
                break;
            case "proposalDetail":
                break;
            case "createVoteCRCProposalTransaction":
                //签名发交易
                goTransferActivity(((CommmonStringEntity) baseEntity).getData());
                break;
        }
    }

    private void goTransferActivity(String attributesJson) {
        Intent intent = new Intent(getActivity(), TransferActivity.class);
        intent.putExtra("amount", num);
        intent.putExtra("wallet", wallet);
        intent.putExtra("chainId", MyWallet.ELA);
        intent.putExtra("attributes", attributesJson);
        intent.putExtra("type", Constant.PROPOSALPUBLISHED);
        intent.putExtra("extra", searchBean);
        intent.putExtra("transType", 1004);
        startActivity(intent);
    }

    @Override
    public void onBalance(BalanceEntity data) {

        Intent intent = new Intent(getContext(), VoteActivity.class);
        BigDecimal balance = Arith.div(Arith.sub(data.getBalance(), 1000000), MyWallet.RATE_S, 8);
        maxBalance = NumberiUtil.removeZero(balance.toPlainString());
        //小于1 huo 0
        if ((balance.compareTo(new BigDecimal(0)) <= 0)) {
            maxBalance = "0";
            intent.putExtra("maxBalance", "0");
        } else {
            intent.putExtra("maxBalance", maxBalance);
        }
        intent.putExtra("type", Constant.PROPOSALPUBLISHED);
        startActivity(intent);
    }

    String num;//投票金额

    @Subscribe(threadMode = ThreadMode.MAIN)
    public void Event(BusEvent result) {
        int integer = result.getCode();
        if (integer == RxEnum.VOTETRANSFERACTIVITY.ordinal()) {
            num = result.getName();
            //点击下一步 获得金额后
            presenter.getVoteInfo(wallet.getWalletId(), "CRCProposal", this);
        }
        if (integer == RxEnum.TRANSFERSUCESS.ordinal()) {
            new DialogUtil().showTransferSucess(getBaseActivity(), new WarmPromptListener() {
                @Override
                public void affireBtnClick(View view) {
                    popBackFragment();
                }
            });

        }

    }


}
