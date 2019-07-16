package org.elastos.wallet.ela.ui.vote.SuperNodeList;

import android.graphics.Bitmap;
import android.graphics.Color;
import android.graphics.drawable.Drawable;
import android.support.annotation.NonNull;
import android.support.annotation.Nullable;
import android.widget.ImageView;

import com.bumptech.glide.request.target.CustomViewTarget;
import com.bumptech.glide.request.transition.Transition;
import com.chad.library.adapter.base.BaseQuickAdapter;
import com.chad.library.adapter.base.BaseViewHolder;

import org.elastos.wallet.R;
import org.elastos.wallet.ela.base.BaseFragment;
import org.elastos.wallet.ela.ui.vote.bean.VoteListBean;
import org.elastos.wallet.ela.utils.AppUtlis;
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
    private Map<String, Bitmap> map;

    private int pos;
    // private boolean is;

    public SuperNodeListAdapter1(BaseFragment context, @Nullable List<VoteListBean.DataBean.ResultBean.ProducersBean> data, int pos, boolean is) {
        super(R.layout.item_super_node_list1, data);
        this.context = context;
        this.pos = pos;
        //  this.is = is;
        glideRequest = GlideApp.with(context).asBitmap().error(R.mipmap.found_vote_initial_circle)
                .placeholder(R.mipmap.found_vote_initial_circle).circleCrop();
        if (map == null) {
            map = new HashMap<>();
        } else {
            map.clear();
        }
    }

    @Override
    protected void convert(BaseViewHolder helper, VoteListBean.DataBean.ResultBean.ProducersBean bean) {

        helper.setBackgroundColor(R.id.ll, context.getResources().getColor(R.color.black));
        if (pos == helper.getLayoutPosition()) {
            helper.setBackgroundColor(R.id.ll, Color.parseColor("#307CA2"));
        }
        helper.setText(R.id.tv_rank, "" + (helper.getLayoutPosition() + 1));
        helper.setText(R.id.tv_name, bean.getNickname());
        helper.setText(R.id.tv_address, AppUtlis.getLoc(context.getContext(), bean.getLocation() + ""));
        helper.setText(R.id.tv_zb, NumberiUtil.numberFormat(Double.parseDouble(bean.getVoterate()) * 100 + "", 5) + "%");
        helper.setText(R.id.tv_num, new BigDecimal(bean.getVotes()).intValue() + " " + context.getString(R.string.ticket));
        ImageView iv = helper.getView(R.id.iv_icon);
        iv.setImageResource(R.mipmap.found_vote_initial_circle);
        GlideApp.with(context).clear(iv);
        String baseUrl = bean.getUrl();
        if (baseUrl == null) {
            return;
        }
       /* if (map.get(baseUrl) != null) {
             iv.setImageBitmap(map.get(baseUrl));
            // glideRequest.load(map.get(baseUrl)).into(iv);
            return;
        }*/
        iv.setTag(R.string.error_tag_empty, baseUrl);
        new SuperNodeListPresenter().getUrlJson(baseUrl, context, new NodeDotJsonViewData() {
            @Override
            public void onGetNodeDotJsonData(NodeInfoBean t, String url) {
                if (iv.getTag(R.string.error_tag_empty) == null || !(iv.getTag(R.string.error_tag_empty)).equals(url)) {
                    return;
                }
                if (t == null || t.getOrg() == null || t.getOrg().getBranding() == null || t.getOrg().getBranding().getLogo_256() == null) {
                    return;
                }
                String imgUrl = t.getOrg().getBranding().getLogo_256();

                glideRequest.load(imgUrl).into(new CustomViewTarget<ImageView, Bitmap>(iv) {
                    @Override
                    public void onLoadFailed(@Nullable Drawable errorDrawable) {
                        //GlideApp.with(context).clear(iv);
                    }

                    @Override
                    public void onResourceReady(@NonNull Bitmap resource, @Nullable Transition<? super Bitmap> transition) {
                        if (iv.getTag(R.string.error_tag_empty) != null && (iv.getTag(R.string.error_tag_empty)).equals(url)) {
                            glideRequest.load(resource).into(iv);
                            map.put(url, resource);
                        } else {
                            GlideApp.with(context).clear(iv);
                        }
                    }

                    @Override
                    protected void onResourceCleared(@Nullable Drawable placeholder) {
                        // glideRequest.load(placeholder).into(iv);
                    }
                });


            }
        });
    }
}

