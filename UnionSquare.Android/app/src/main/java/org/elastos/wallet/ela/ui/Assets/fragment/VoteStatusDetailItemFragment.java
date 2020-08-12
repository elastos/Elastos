package org.elastos.wallet.ela.ui.Assets.fragment;

import android.os.Bundle;
import android.support.v7.widget.RecyclerView;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.TextView;

import org.elastos.wallet.R;
import org.elastos.wallet.ela.base.BaseFragment;
import org.elastos.wallet.ela.ui.Assets.bean.VoteStatus;
import org.elastos.wallet.ela.ui.committee.bean.CtListBean;
import org.elastos.wallet.ela.ui.crvote.bean.CRListBean;
import org.elastos.wallet.ela.ui.proposal.bean.ProposalSearchEntity;
import org.elastos.wallet.ela.ui.vote.bean.VoteListBean;
import org.json.JSONArray;
import org.json.JSONException;
import org.json.JSONObject;

import java.math.BigDecimal;
import java.util.ArrayList;
import java.util.Iterator;
import java.util.List;

import butterknife.BindView;
import butterknife.ButterKnife;
import butterknife.Unbinder;

public class VoteStatusDetailItemFragment extends BaseFragment {


    Unbinder unbinder;
    @BindView(R.id.tv_type)
    TextView tvType;
    @BindView(R.id.tv_votetime)
    TextView tvVotetime;
    @BindView(R.id.tv_resttime)
    TextView tvResttime;
    @BindView(R.id.tv_ticketnum)
    TextView tvTicketnum;
    @BindView(R.id.tv_rvtitle)
    TextView tvRvtitle;
    @BindView(R.id.rv)
    RecyclerView rv;
    private ArrayList<ProposalSearchEntity.DataBean.ListBean> searchBeanList;
    private ArrayList<CtListBean.Council> councilList;
    private ArrayList<CRListBean.DataBean.ResultBean.CrcandidatesinfoBean> crList;
    private ArrayList<VoteListBean.DataBean.ResultBean.ProducersBean> depositList;
    private ArrayList<VoteStatus> listVoteStatus;
    private long currentStartTime;
    private String voteInfo;

    @Override
    protected int getLayoutId() {
        return R.layout.fragment_votestatus_detail_item;
    }

    @Override
    protected void setExtraData(Bundle data) {
        currentStartTime = data.getLong("currentStartTime");
        voteInfo = data.getString("voteInfo");
        depositList = data.getParcelableArrayList("depositList");
        crList = data.getParcelableArrayList("crList");
        searchBeanList = data.getParcelableArrayList("searchBeanList");
        councilList = data.getParcelableArrayList("councilList");
        listVoteStatus = data.getParcelableArrayList("listVoteStatus");
        // conversUnactiveVote(currentStartTime, voteInfo, depositList, crList, searchBeanList, councilList);

    }

    public static VoteStatusDetailItemFragment getInstance(String type, ArrayList arrayList, String vote) {
        VoteStatusDetailItemFragment detailItemFragment = new VoteStatusDetailItemFragment();
        Bundle bundle = new Bundle();
        bundle.putString("vote", vote);
        bundle.putString("type", type);
        bundle.putParcelableArrayList("arrayList", arrayList);
        detailItemFragment.setExtraData(bundle);
        return detailItemFragment;
    }

    @Override
    protected void initView(View view) {
        mRootView.setBackgroundResource(R.drawable.sc_80000000_stc_ffffff);
    }

    public void conversUnactiveVote(long currentStartTime, String voteInfo, List<VoteListBean.DataBean.ResultBean.ProducersBean> depositList,
                                    List<CRListBean.DataBean.ResultBean.CrcandidatesinfoBean> crcList, List<ProposalSearchEntity.DataBean.ListBean> voteList, List<CtListBean.Council> councilList) {


        try {
            JSONArray lastVoteInfo = new JSONArray(voteInfo);
            long lastTime = 0;
            BigDecimal maxCount = new BigDecimal(0);
            for (int i = 0; i < lastVoteInfo.length(); i++) {
                JSONObject jsonObject = lastVoteInfo.getJSONObject(i);
                String type = jsonObject.getString("Type");

                long timestamp = jsonObject.getLong("Timestamp");
                if (lastTime < timestamp) {
                    lastTime = timestamp;
                }
                JSONObject votes = jsonObject.getJSONObject("Votes");
                Iterator it = votes.keys();
                JSONArray candidates = new JSONArray();
                VoteStatus voteStatus = null;
                BigDecimal count = new BigDecimal(0);
                switch (type) {
                    case "Delegate":
                        voteStatus = listVoteStatus.get(0);
                        while (it.hasNext()) {
                            String key = (String) it.next();
                            String value = votes.getString(key);
                            count = new BigDecimal(value);
                            if (depositList == null || depositList.size() == 0) {
                                candidates.put(key);
                                continue;
                            }
                            for (VoteListBean.DataBean.ResultBean.ProducersBean bean : depositList) {
                                if (bean.getOwnerpublickey().equals(key) && !bean.getState().equals("Active")) {
                                    candidates.put(key);
                                    break;
                                }
                            }

                        }
                        break;
                    case "CRC":
                        voteStatus = listVoteStatus.get(1);
                        while (it.hasNext()) {
                            String key = (String) it.next();
                            String value = votes.getString(key);
                            count = count.add(new BigDecimal(value));
                            if (timestamp < currentStartTime || crcList == null || crcList.size() == 0) {
                                candidates.put(key);
                                continue;
                            }
                            for (CRListBean.DataBean.ResultBean.CrcandidatesinfoBean bean : crcList) {
                                if (bean.getDid().equals(key) && !bean.getState().equals("Active")) {
                                    candidates.put(key);
                                    break;
                                }
                            }
                        }

                        break;
                    case "CRCImpeachment"://弹劾
                        voteStatus = listVoteStatus.get(2);
                        while (it.hasNext()) {
                            String key = (String) it.next();
                            String value = votes.getString(key);
                            count = count.add(new BigDecimal(value));
                            if (timestamp < currentStartTime || councilList == null || councilList.size() == 0) {
                                candidates.put(key);
                                continue;
                            }
                            for (CtListBean.Council bean : councilList) {
                                if ((bean.getDid().equals(key) && !bean.getStatus().equals("Elected"))) {
                                    candidates.put(key);
                                    break;
                                }
                            }
                        }
                        break;
                    case "CRCProposal":
                        voteStatus = listVoteStatus.get(3);
                        while (it.hasNext()) {
                            String key = (String) it.next();
                            String value = votes.getString(key);
                            count = count.add(new BigDecimal(value));
                            if (voteList == null || voteList.size() == 0) {
                                candidates.put(key);
                                continue;
                            }
                            for (ProposalSearchEntity.DataBean.ListBean bean : voteList) {
                                if (bean.getProposalHash().equals(key) && !bean.getStatus().equals("NOTIFICATION")) {
                                    candidates.put(key);
                                    break;
                                }
                            }
                        }
                        break;


                }
                if (voteStatus != null) {
                    //默认0没有投票   1 有投票部分失效 2 有投票完全失效 3有投票无失效
                    if (candidates.length() == 0) {
                        voteStatus.setStatus(3);
                    } else if (candidates.length() == votes.length()) {
                        voteStatus.setStatus(2);
                    } else {
                        voteStatus.setStatus(1);
                    }
                    voteStatus.setCount(count);
                    if (maxCount.compareTo(count) < 0) {
                        maxCount = count;
                    }
                }


            }
            //   tvVoteTime.setText(getString(R.string.lastvotetime) + DateUtil.time(lastTime, getContext()));
            //tvVoteCount.setText(getString(R.string.allcount) + Arith.div(maxCount, MyWallet.RATE_S, 8).longValue() + " ELA");
        } catch (JSONException e) {
            e.printStackTrace();
        }


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
