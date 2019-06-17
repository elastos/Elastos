package org.elastos.wallet.ela.ui.vote.SuperNodeList;

import android.content.Context;
import android.graphics.Bitmap;
import android.graphics.Color;
import android.support.annotation.Nullable;
import android.widget.ImageView;

import com.chad.library.adapter.base.BaseQuickAdapter;
import com.chad.library.adapter.base.BaseViewHolder;

import org.elastos.wallet.R;
import org.elastos.wallet.ela.ui.vote.bean.VoteListBean;
import org.elastos.wallet.ela.utils.AppUtlis;
import org.elastos.wallet.ela.utils.GetDynanicUrl;
import org.elastos.wallet.ela.utils.GlideApp;
import org.elastos.wallet.ela.utils.GlideRequest;
import org.elastos.wallet.ela.utils.Log;
import org.elastos.wallet.ela.utils.NumberiUtil;

import java.math.BigDecimal;
import java.util.List;

public class SuperNodeListAdapter1 extends BaseQuickAdapter<VoteListBean.DataBean.ResultBean.ProducersBean, BaseViewHolder> {

    private final GlideRequest<Bitmap> glideRequest;
    private Context context;


    private int pos;
    // private boolean is;

    public SuperNodeListAdapter1(Context context, @Nullable List<VoteListBean.DataBean.ResultBean.ProducersBean> data, int pos, boolean is) {
        super(R.layout.item_super_node_list1, data);
        this.context = context;
        this.pos = pos;
        //  this.is = is;
        glideRequest = GlideApp.with(context).asBitmap().error(R.mipmap.found_vote_initial)
                .placeholder(R.mipmap.found_vote_initial).circleCrop();
    }

    @Override
    protected void convert(BaseViewHolder helper, VoteListBean.DataBean.ResultBean.ProducersBean bean) {
        Log.d("????c",helper.getLayoutPosition()+"");
        helper.setBackgroundColor(R.id.ll, context.getResources().getColor(R.color.black));
        if (pos == helper.getLayoutPosition()) {
            helper.setBackgroundColor(R.id.ll, Color.parseColor("#307CA2"));
        }
        helper.setText(R.id.tv_rank, "No." + (helper.getLayoutPosition() + 1));
        helper.setText(R.id.tv_name, bean.getNickname());
        helper.setText(R.id.tv_address, AppUtlis.getLoc(context, bean.getLocation() + ""));
        helper.setText(R.id.tv_zb, NumberiUtil.numberFormat(Double.parseDouble(bean.getVoterate()) * 100 + "", 5) + "%");
        helper.setText(R.id.tv_num, new BigDecimal(bean.getVotes()).intValue() + " " + context.getString(R.string.ticket));
       ImageView iv = helper.getView(R.id.iv_icon);
        glideRequest.load(R.mipmap.found_vote_initial).into(iv);
        iv.setTag(R.string.error_tag_empty, helper.getLayoutPosition());
        if (iv.getTag(R.string.error_tag_empty) != null && iv.getTag(R.string.error_tag_empty).equals(helper.getLayoutPosition())) {
            String url = bean.getUrl();
            GetDynanicUrl.getData(url, context, new NodeDotJsonViewData() {
                @Override
                public void onGetNodeDotJsonData(NodeInfoBean t) {
                    if (t == null || t.getOrg() == null || t.getOrg().getBranding() == null) {
                        return;
                    }
                    String imgUrl = t.getOrg().getBranding().getLogo_256();
                    iv.setTag(R.string.ownerpublicKey, imgUrl);

                    if (iv.getTag(R.string.ownerpublicKey) != null && iv.getTag(R.string.ownerpublicKey).equals(imgUrl)) {
                        glideRequest.load(imgUrl).into(iv);
                    } else {
                        GlideApp.with(context).clear(iv);
                        glideRequest.load(R.mipmap.found_vote_initial).into(iv);
                    }


                }
            });
        }

    /*    if (isImagePosition(position)) {
            String url = urls.get(position);
            Glide.with(fragment)
                    .load(url)
                    .into(holder.imageView);
        } else {
            Glide.with(fragment).clear(holder.imageView);
            holder.imageView.setImageDrawable(specialDrawable);
        }*/
    }
}
