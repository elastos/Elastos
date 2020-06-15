/*
 * Copyright (c) 2019 Elastos Foundation
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

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

import com.alibaba.fastjson.JSON;

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
import org.elastos.wallet.ela.ui.Assets.bean.qr.proposal.RecieveReviewJwtEntity;
import org.elastos.wallet.ela.ui.Assets.presenter.CommonGetBalancePresenter;
import org.elastos.wallet.ela.ui.Assets.viewdata.CommonBalanceViewData;
import org.elastos.wallet.ela.ui.committee.bean.CtDetailBean;
import org.elastos.wallet.ela.ui.committee.bean.CtListBean;
import org.elastos.wallet.ela.ui.committee.presenter.CtListPresenter;
import org.elastos.wallet.ela.ui.common.bean.CommmonStringEntity;
import org.elastos.wallet.ela.ui.common.fragment.WebViewFragment;
import org.elastos.wallet.ela.ui.crvote.bean.CRListBean;
import org.elastos.wallet.ela.ui.crvote.presenter.CRlistPresenter;
import org.elastos.wallet.ela.ui.proposal.adapter.ProcessRecAdapetr;
import org.elastos.wallet.ela.ui.proposal.adapter.VoteRecAdapetr;
import org.elastos.wallet.ela.ui.proposal.bean.ProposalDetailEntity;
import org.elastos.wallet.ela.ui.proposal.bean.ProposalSearchEntity;
import org.elastos.wallet.ela.ui.proposal.presenter.ProposalDetailPresenter;
import org.elastos.wallet.ela.ui.proposal.presenter.ProposalPresenter;
import org.elastos.wallet.ela.ui.vote.ElectoralAffairs.VoteListPresenter;
import org.elastos.wallet.ela.ui.vote.activity.VoteActivity;
import org.elastos.wallet.ela.ui.vote.bean.VoteListBean;
import org.elastos.wallet.ela.utils.Arith;
import org.elastos.wallet.ela.utils.ClipboardUtil;
import org.elastos.wallet.ela.utils.Constant;
import org.elastos.wallet.ela.utils.DateUtil;
import org.elastos.wallet.ela.utils.DialogUtil;
import org.elastos.wallet.ela.utils.JwtUtils;
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
    @BindView(R.id.tv_abstract1)
    TextView tvAbstract1;
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
    @BindView(R.id.ll_hasvote)
    LinearLayout llHasvote;
    @BindView(R.id.ll_restTime)
    LinearLayout llRestTime;
    @BindView(R.id.rl_vote_button)
    RelativeLayout rlVoteButton;
    Unbinder unbinder;
    @BindView(R.id.ll_disagreeprogress)
    LinearLayout llDisagreeprogress;
    @BindView(R.id.tv_novote)
    TextView tvNovote;
    @BindView(R.id.circleIndicator)
    CircleProgressView circleIndicator;
    @BindView(R.id.tv_currentvote)
    TextView tvCurrentvote;
    @BindView(R.id.tv_needvote)
    TextView tvNeedvote;
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
    private String scanResult;
    private Wallet wallet;
    private ProposalPresenter proposalPresenter;
    private ProposalDetailPresenter presenter;
    private List<ProposalSearchEntity.DataBean.ListBean> searchBeanList;
    private List<CRListBean.DataBean.ResultBean.CrcandidatesinfoBean> crList;
    private List<VoteListBean.DataBean.ResultBean.ProducersBean> depositList;
    private List<CtListBean.Council> councilList;

    @Override
    protected int getLayoutId() {
        return R.layout.fragment_proposal_review;
    }

    @Override
    protected void setExtraData(Bundle data) {
        super.setExtraData(data);

        searchBean = data.getParcelable("ProposalSearchDate");
    }

    @Override
    protected void initView(View view) {
        tvAgree.setEnabled(false);
        tvDisagree.setEnabled(false);
        tvAbstention.setEnabled(false);
        wallet = new RealmUtil().queryDefauleWallet();
        presenter = new ProposalDetailPresenter();
        proposalPresenter = new ProposalPresenter();

        presenter.proposalDetail(searchBean.getId(), this);
        registReceiver();
        switch (searchBean.getStatus()) {
            case "VOTING":
                //委员评议
                llRestTime.setVisibility(View.VISIBLE);
                proposalPresenter.getCurrentCouncilInfo(wallet.getDid().replace("did:elastos:", ""), this);
                tvTitle.setText(R.string.proposalcomments);
                tvStatus.setText(R.string.menberreview);
                setInfoStatue(true);
                setVoteStatue(true);
                break;
            case "NOTIFICATION":
                //公示期
                llRestTime.setVisibility(View.VISIBLE);
                tvTitle.setText(R.string.proposalpublished);
                tvVote.setText(R.string.votedisagree);
                tvStatus.setText(R.string.publishing);
                setVoteStatue(true);
                rlVoteButton.setVisibility(View.VISIBLE);
                llDisagreeprogress.setVisibility(View.VISIBLE);
                break;
            case "ACTIVE":
                //执行期;
                tvTitle.setText(R.string.proposalprocess);
                tvStatus.setText(R.string.executing);
                llProcess.setVisibility(View.VISIBLE);
                break;
            case "FINAL":
                //已完结
                tvTitle.setText(R.string.proposalfinish);
                tvStatus.setText(R.string.hasfinished);
                llProcess.setVisibility(View.VISIBLE);
                break;
            case "REJECTED":
                // 已废止 未通过
                tvTitle.setText(R.string.proposalabandon);
                tvStatus.setText(R.string.nopass);
                tvPropasalTile.getPaint().setFlags(Paint.STRIKE_THRU_TEXT_FLAG);//删除线
                setInfoStatue(true);
                setVoteStatue(true);
                break;
            case "VETOED":
                //已废止 已否决
                llDisagreeprogress.setVisibility(View.VISIBLE);
                tvTitle.setText(R.string.proposalabandon);
                tvStatus.setText(R.string.hasreject);
                tvPropasalTile.getPaint().setFlags(Paint.STRIKE_THRU_TEXT_FLAG);//删除线
                setInfoStatue(true);
                setVoteStatue(true);
                break;
        }
    }

    private void setWebData(ProposalDetailEntity.DataBean data) {

        tvPropasalTile.setText(searchBean.getTitle());
        tvNum.setText("#" + searchBean.getId());
        tvTime.setText(DateUtil.timeNYR(searchBean.getCreatedAt(), getContext(), true));
        tvPeople.setText(searchBean.getProposedBy());
        tvHash.setText(searchBean.getProposalHash());
        tvResttime.setText(setRestDay(data.getDuration()));
        tvWeb.setText(data.getAddress());
        tvAbstract1.setText(data.getAbs());
        initVote(data.getVoteResult());
        setDisagreeProgress(data.getRejectRatio());
        tvCurrentvote.setText(data.getRejectAmount() != null ? data.getRejectAmount().split("\\.")[0] : "");
        tvNeedvote.setText(data.getRejectThroughAmount() != null ? data.getRejectThroughAmount().split("\\.")[0] : "");

        setProcessRecycleView(data.getTracking());

    }

    private String setRestDay(long time) {
        int hours = (int) (time / 3600);
        if (hours >= 24) {
            return String.format(getString(R.string.aboutday), String.valueOf(hours / 24));
        } else if (hours >= 1) {
            return String.format(getString(R.string.abouthour), String.valueOf(hours));
        } else {
            return getString(R.string.noonehour);
        }

    }

    private void setDisagreeProgress(float progress) {

        circleIndicator.setProgress(progress);
        //setInfoStatue(false);
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
            //同意 6   反对 2   弃权 2
            tvVotestatus.setVisibility(View.VISIBLE);
            llVote.setVisibility(View.GONE);
            ivVote.setImageResource(R.mipmap.cr_arrow_right);
        } else {
            tvVotestatus.setVisibility(View.GONE);
            llVote.setVisibility(View.VISIBLE);
            ivVote.setImageResource(R.mipmap.cr_arrow_down);
        }


    }

    private void voteLeftClickUI(View target, List<ProposalDetailEntity.DataBean.VoteResultBean> list) {
        setVoteRecycleView(list);
        int targetBottom = ScreenUtil.dp2px(getContext(), 5);
        tvAgree.setAlpha(0.5f);
        tvDisagree.setAlpha(0.5f);
        tvAbstention.setAlpha(0.5f);
        setMargin(tvAgree, 0, 0, 0, targetBottom);
        setMargin(tvDisagree, ScreenUtil.dp2px(getContext(), 5), 0, 0, targetBottom);
        setMargin(tvAbstention, ScreenUtil.dp2px(getContext(), 5), 0, 0, targetBottom);

        target.setAlpha(1);
        if (target == tvAgree) {
            setMargin(target, 0, 0, 0, 0);
        }
        if (target == tvDisagree || target == tvAbstention) {
            setMargin(target, ScreenUtil.dp2px(getContext(), 5), 0, 0, 0);

        }

    }


    @OnClick({R.id.tv_agree, R.id.iv_info, R.id.iv_vote, R.id.tv_disagree, R.id.tv_abstention, R.id.tv_hash, R.id.tv_web, R.id.tv_vote})
    public void onViewClicked(View view) {
        switch (view.getId()) {
            case R.id.tv_agree:
                voteLeftClickUI(view, supportList);
                break;
            case R.id.tv_disagree:
                voteLeftClickUI(view, rejectList);
                break;
            case R.id.tv_abstention:
                voteLeftClickUI(view, abstentionList);
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
                    proposalPresenter.proposalSearch(-1, -1, "NOTIFICATION", null, this);
                    new VoteListPresenter().getDepositVoteList("1", "all", this, true);
                    new CRlistPresenter().getCRlist(-1, -1, "all", this, true);
                    new CtListPresenter().getCouncilList(this, String.valueOf(1));
                    new CommonGetBalancePresenter().getBalance(wallet.getWalletId(), MyWallet.ELA, 2, this);
                }
                break;
            case R.id.iv_info:
                if (llInfo.getVisibility() == View.VISIBLE) {
                    setInfoStatue(false);
                } else {
                    setInfoStatue(true);
                }
                break;
            case R.id.iv_vote:
                if (llVote.getVisibility() == View.VISIBLE) {
                    setVoteStatue(false);
                } else {
                    setVoteStatue(true);
                }

                break;


        }
    }

    List<ProposalDetailEntity.DataBean.VoteResultBean> supportList;
    List<ProposalDetailEntity.DataBean.VoteResultBean> rejectList;
    List<ProposalDetailEntity.DataBean.VoteResultBean> abstentionList;

    private void initVote(List<ProposalDetailEntity.DataBean.VoteResultBean> list) {

        if (list == null || list.size() == 0) {
            tvNovote.setVisibility(View.VISIBLE);
            ivVote.setVisibility(View.GONE);
            llHasvote.setVisibility(View.GONE);
            tvVotestatus.setVisibility(View.GONE);
            return;
        }
        tvNovote.setVisibility(View.GONE);
        llHasvote.setVisibility(View.VISIBLE);
        supportList = new ArrayList<>();
        rejectList = new ArrayList<>();
        abstentionList = new ArrayList<>();

        for (ProposalDetailEntity.DataBean.VoteResultBean bean : list) {
            //  投票类型 [赞同: 'support', 反对: 'reject', 弃权: 'abstention']
            if ("support".equals(bean.getValue())) {
                supportList.add(bean);
            } else if ("reject".equals(bean.getValue())) {
                rejectList.add(bean);
            } else if ("abstention".equals(bean.getValue())) {
                abstentionList.add(bean);
            }
        }
        tvAgree.setText(getString(R.string.agree1) + " (" + supportList.size() + ")");
        tvDisagree.setText(getString(R.string.disagree1) + " (" + rejectList.size() + ")");
        tvAbstention.setText(getString(R.string.abstention) + " (" + abstentionList.size() + ")");
        if (abstentionList.size() > 0) {
            tvAbstention.setEnabled(true);
        }
        if (rejectList.size() > 0) {
            tvDisagree.setEnabled(true);
        }
        if (supportList.size() > 0) {
            tvAgree.setEnabled(true);
        }
        if (supportList.size() > 0) {
            voteLeftClickUI(tvAgree, supportList);
        } else if (rejectList.size() > 0) {
            voteLeftClickUI(tvDisagree, supportList);
        } else if (abstentionList.size() > 0) {
            voteLeftClickUI(tvAbstention, supportList);
        }
        tvVotestatus.setText(getString(R.string.agree) + " " + (supportList == null ? 0 : supportList.size())
                + "   " + getString(R.string.disagree1) + " " + (rejectList == null ? 0 : rejectList.size())
                + "   " + getString(R.string.abstention) + " " + (abstentionList == null ? 0 : abstentionList.size()));
    }

    private void setVoteRecycleView(List<ProposalDetailEntity.DataBean.VoteResultBean> list) {

        if (list == null || list.size() == 0) {
            return;
        }
        VoteRecAdapetr adapter = new VoteRecAdapetr(getContext(), list);
        rvVote.setLayoutManager(new LinearLayoutManager(getContext(), LinearLayoutManager.VERTICAL, false));
        rvVote.setAdapter(adapter);
    }

    private void setProcessRecycleView(List<ProposalDetailEntity.DataBean.TrackingBean> list) {
        if (list == null || list.size() == 0) {
            return;
        }
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
                    String result = scanResult.replace("elastos://crproposal/", "");
                    try {
                        String payload = JwtUtils.getJwtPayload(result);
                        RecieveReviewJwtEntity curentJwtEntity = JSON.parseObject(payload, RecieveReviewJwtEntity.class);
                        if (!"reviewproposal".equals(curentJwtEntity.getCommand())) {
                            showToast(getString(R.string.infoformatwrong));
                        } else if (!curentJwtEntity.getData().getProposalhash().equals(searchBean.getProposalHash())) {
                            showToast(getString(R.string.proposalhashnotsame));
                        } else {
                            post(RxEnum.SCANDATATOASSETPAGE.ordinal(), getClass().getSimpleName(), scanResult);//交给首页去处理
                        }

                    } catch (Exception e) {
                        showToast(getString(R.string.infoformatwrong));
                    }
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
            case "getCouncilList":
                councilList = ((CtListBean) baseEntity).getData().getCouncil();
                break;
            case "getCurrentCouncilInfo":
                CtDetailBean ctDetailBean = (CtDetailBean) baseEntity;
                if ("CouncilMember".equals(ctDetailBean.getData().getType())
                        && searchBean.getStatus().equals("VOTING")) {
                    rlVoteButton.setVisibility(View.VISIBLE);
                }
                break;

            case "getVoteInfo":
                //剔除非公示期的
                String voteInfo = ((CommmonStringEntity) baseEntity).getData();
                JSONArray otherUnActiveVote = presenter.conversUnactiveVote("CRCProposal", voteInfo, depositList, crList, searchBeanList,councilList);
                try {
                    JSONObject voteJson = presenter.conversVote(voteInfo, "CRCProposal");//key value
                    //点击下一步 获得上次的投票后筛选数据
                    String amount = Arith.mulRemoveZero(num, MyWallet.RATE_S).toPlainString();
                    JSONObject newVotes = presenter.getPublishDataFromLastVote(voteJson, amount, searchBeanList);
                    newVotes.put(searchBean.getProposalHash(), amount);
                    presenter.createVoteCRCProposalTransaction(wallet.getWalletId(), newVotes.toString(), otherUnActiveVote.toString(), this);

                } catch (Exception e) {
                    e.printStackTrace();
                }


                break;
            case "proposalSearch":
                searchBeanList = ((ProposalSearchEntity) baseEntity).getData().getList();
                break;
            case "getCRlist":
                crList = ((CRListBean) baseEntity).getData().getResult().getCrcandidatesinfo();

                break;
            case "getDepositVoteList":
                depositList = ((VoteListBean) baseEntity).getData().getResult().getProducers();
                break;
            case "proposalDetail":
                setWebData(((ProposalDetailEntity) baseEntity).getData());
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
        intent.putExtra("extra", searchBean.getProposalHash());
        intent.putExtra("transType", 1004);
        startActivity(intent);
    }

    @Override
    public void onBalance(BalanceEntity data) {

        Intent intent = new Intent(getContext(), VoteActivity.class);
        BigDecimal balance = Arith.div(Arith.sub(data.getBalance(), 1000000), MyWallet.RATE_S, 8);
        String maxBalance = NumberiUtil.removeZero(balance.toPlainString());
        //小于1 huo 0
        if ((balance.compareTo(new BigDecimal(0)) <= 0)) {
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
            num = (String) result.getObj();
            //点击下一步 获得金额后
            presenter.getVoteInfo(wallet.getWalletId(), "", this);
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
