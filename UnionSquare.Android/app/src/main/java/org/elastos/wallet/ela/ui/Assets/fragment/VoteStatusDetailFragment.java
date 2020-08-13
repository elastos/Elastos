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
import org.elastos.wallet.ela.ui.committee.bean.CtListBean;
import org.elastos.wallet.ela.ui.crvote.bean.CRListBean;
import org.elastos.wallet.ela.ui.proposal.bean.ProposalSearchEntity;
import org.elastos.wallet.ela.ui.vote.bean.VoteListBean;
import org.elastos.wallet.ela.utils.ScreenUtil;
import org.json.JSONArray;
import org.json.JSONException;
import org.json.JSONObject;

import java.math.BigDecimal;
import java.util.ArrayList;
import java.util.Iterator;
import java.util.List;

import butterknife.BindView;
import butterknife.Unbinder;

public class VoteStatusDetailFragment extends BaseFragment {
    @BindView(R.id.tv_title)
    TextView tvTitle;
    @BindView(R.id.viewpage)
    ViewPager viewpage;
    @BindView(R.id.ll_node)
    LinearLayout llNode;
    Unbinder unbinder;
    private ArrayList<ProposalSearchEntity.DataBean.ListBean> searchBeanList;
    private ArrayList<CtListBean.Council> councilList;
    private ArrayList<CRListBean.DataBean.ResultBean.CrcandidatesinfoBean> crList;
    private ArrayList<VoteListBean.DataBean.ResultBean.ProducersBean> depositList;
    private ArrayList<VoteStatus> listVoteStatus;
    private long currentStartTime;
    private String voteInfo;
    private List<BaseFragment> fragmentList = new ArrayList();

    @Override
    protected int getLayoutId() {
        return R.layout.fragment_votestatus_detail;
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
        conversUnactiveVote(currentStartTime, voteInfo, depositList, crList, searchBeanList, councilList);

        fragmentList.add(new VoteStatusDetailItemFragment());
        fragmentList.add(new VoteStatusDetailItemFragment());
        fragmentList.add(new VoteStatusDetailItemFragment());
        viewpage.setAdapter(new CtListPagerAdapter(getFragmentManager(), fragmentList));
        viewpage.setPageMargin(ScreenUtil.dp2px(getContext(), 15));
    }

    @Override
    protected void initView(View view) {
        tvTitle.setText(R.string.votedetail);
    }

    public void conversUnactiveVote(long currentStartTime, String voteInfo, List<VoteListBean.DataBean.ResultBean.ProducersBean> depositList,
                                    List<CRListBean.DataBean.ResultBean.CrcandidatesinfoBean> crcList, List<ProposalSearchEntity.DataBean.ListBean> voteList, List<CtListBean.Council> councilList) {


        try {
            JSONArray lastVoteInfo = new JSONArray(voteInfo);
            for (int i = 0; i < lastVoteInfo.length(); i++) {
                JSONObject jsonObject = lastVoteInfo.getJSONObject(i);
                String type = jsonObject.getString("Type");

                long timestamp = jsonObject.getLong("Timestamp");

                JSONObject votes = jsonObject.getJSONObject("Votes");
                Iterator it = votes.keys();
                JSONArray candidates = new JSONArray();

                BigDecimal count = new BigDecimal(0);
                switch (type) {
                    case "Delegate":
                        ArrayList<VoteListBean.DataBean.ResultBean.ProducersBean> resultList =new ArrayList<>();
                        while (it.hasNext()) {
                            String key = (String) it.next();
                            String value = votes.getString(key);
                            if (depositList == null || depositList.size() == 0) {
                                VoteListBean.DataBean.ResultBean.ProducersBean producersBean=new VoteListBean.DataBean.ResultBean.ProducersBean();
                                producersBean.setNickname("");
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


            }
            //   tvVoteTime.setText(getString(R.string.lastvotetime) + DateUtil.time(lastTime, getContext()));
            //tvVoteCount.setText(getString(R.string.allcount) + Arith.div(maxCount, MyWallet.RATE_S, 8).longValue() + " ELA");
        } catch (JSONException e) {
            e.printStackTrace();
        }


    }

    private class Recorder implements Comparable<Recorder> {
        int no;
        String name;

        @Override
        public int compareTo(Recorder o) {
            return this.no - o.no;
        }
    }

    //获取名字
    private Recorder getRecord(String publickey) {
        Recorder recorder = new Recorder();
        for (int i = 0; i < depositList.size(); i++) {
            if (depositList.get(i).getOwnerpublickey().equals(publickey)) {
                recorder.no = (depositList.get(i).getIndex() + 1);
                recorder.name = depositList.get(i).getNickname();
                return recorder;
            }
        }
        recorder.no = Integer.MAX_VALUE;
        recorder.name = getString(R.string.invalidcr);
        return recorder;
    }
}
