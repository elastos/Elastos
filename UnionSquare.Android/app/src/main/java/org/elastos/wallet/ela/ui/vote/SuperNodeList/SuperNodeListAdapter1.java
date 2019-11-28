package org.elastos.wallet.ela.ui.vote.SuperNodeList;

import android.graphics.Bitmap;
import android.support.annotation.Nullable;
import android.widget.ImageView;

import com.chad.library.adapter.base.BaseQuickAdapter;
import com.chad.library.adapter.base.BaseViewHolder;

import org.elastos.wallet.R;
import org.elastos.wallet.ela.MyApplication;
import org.elastos.wallet.ela.base.BaseFragment;
import org.elastos.wallet.ela.bean.ImageBean;
import org.elastos.wallet.ela.ui.vote.bean.VoteListBean;
import org.elastos.wallet.ela.utils.AppUtlis;
import org.elastos.wallet.ela.utils.Arith;
import org.elastos.wallet.ela.utils.GlideApp;
import org.elastos.wallet.ela.utils.GlideRequest;
import org.elastos.wallet.ela.utils.NumberiUtil;

import java.math.BigDecimal;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

public class SuperNodeListAdapter1 extends BaseQuickAdapter<VoteListBean.DataBean.ResultBean.ProducersBean, BaseViewHolder> {

    private final GlideRequest<Bitmap> glideRequest;
    private BaseFragment context;
    private Map<String, String> map;

    private boolean is;
    private SuperNodeListPresenter presenter;

    public SuperNodeListAdapter1(BaseFragment context, @Nullable List<VoteListBean.DataBean.ResultBean.ProducersBean> data, boolean is) {
        super(R.layout.item_super_node_list1, data);
        this.context = context;

        this.is = is;

        glideRequest = GlideApp.with(context).asBitmap().error(R.mipmap.found_vote_initial_circle)
                .placeholder(R.mipmap.found_vote_initial_circle).circleCrop();
        if (map == null) {
            map = new HashMap<>();
        } else {
            map.clear();
        }
    }

    public void setIs(boolean is) {
        this.is = is;
    }

    @Override
    protected void convert(BaseViewHolder helper, VoteListBean.DataBean.ResultBean.ProducersBean bean) {

        helper.setBackgroundColor(R.id.ll, context.getResources().getColor(R.color.black));
        if (is && 0 == helper.getLayoutPosition()) {
            helper.setBackgroundColor(R.id.ll, context.getResources().getColor(R.color.blue1));
        }
        helper.setText(R.id.tv_rank, "" + (bean.getIndex() + 1));
        helper.setText(R.id.tv_name, bean.getNickname());
        helper.setText(R.id.tv_address, AppUtlis.getLoc(context.getContext(), bean.getLocation() + ""));
        BigDecimal voterateDecimal = new BigDecimal(bean.getVoterate());
        if (voterateDecimal.compareTo(new BigDecimal(0.01)) < 0) {
            helper.setText(R.id.tv_zb, "< 1%");
        } else {
            String voterate = NumberiUtil.numberFormat(Arith.mul(voterateDecimal, 100), 2);
            helper.setText(R.id.tv_zb, voterate + "%");

        }
        helper.setText(R.id.tv_num, new BigDecimal(bean.getVotes()).intValue() + " " + context.getString(R.string.ticket));
        ImageView iv = helper.getView(R.id.iv_icon);
        iv.setImageResource(R.mipmap.found_vote_initial_circle);
        String baseUrl = bean.getUrl();
        iv.setTag(R.id.error_tag_empty, baseUrl);
        GlideApp.with(context).clear(iv);
        if (baseUrl == null) {
            return;
        }
        if (map.get(baseUrl) != null) {
            if ("".equals(map.get(baseUrl))) {
                return;
            }
            glideRequest.load(map.get(baseUrl)).into(iv);
            return;
        }
        if (presenter == null) {
            presenter = new SuperNodeListPresenter();
        }
        presenter.getUrlJson(iv, baseUrl, context, new NodeDotJsonViewData() {
            @Override
            public void onError(String url) {
                map.put(url, "");
            }

            @Override
            public void onGetNodeDotJsonData(ImageView iv1, NodeInfoBean t, String url) {
                //这个时候的iv已经不是那个iv了  所有传递iv试试
                if (iv1.getTag(R.id.error_tag_empty) == null || !(iv1.getTag(R.id.error_tag_empty).toString()).equals(url)) {
                    return;
                }
                if (t == null || t.getOrg() == null || t.getOrg().getBranding() == null || t.getOrg().getBranding().getLogo_256() == null) {
                    map.put(url, "");
                    return;
                }

                String imgUrl = t.getOrg().getBranding().getLogo_256();
                // map.put(url, imgUrl);
                //glideRequest.load(imgUrl).into(iv1);
                presenter.getImage(iv1, url, imgUrl, context, new NodeDotJsonViewData() {
                    @Override
                    public void onError(String url) {
                        map.put(url, "");
                    }

                    @Override
                    public void onGetImage(ImageView iv1, String url, ImageBean imageBean) {
                        if (iv1.getTag(R.id.error_tag_empty) == null || !(iv1.getTag(R.id.error_tag_empty).toString()).equals(url)) {
                            GlideApp.with(context).clear(iv1);
                            iv1.setImageResource(R.mipmap.found_vote_initial);
                            return;
                        }
                        String newimgUrl = MyApplication.REQUEST_BASE_URL + "/" + imageBean.getData();
                        map.put(url, newimgUrl);
                        glideRequest.load(newimgUrl).into(iv1);
                    }
                });
            }
        });
    }
}

