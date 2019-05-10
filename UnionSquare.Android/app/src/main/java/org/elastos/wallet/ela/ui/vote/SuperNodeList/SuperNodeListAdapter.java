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
import org.elastos.wallet.ela.utils.GetDynanicUrl;
import org.elastos.wallet.ela.utils.GlideApp;
import org.elastos.wallet.ela.utils.GlideRequest;

import java.math.BigDecimal;
import java.util.List;

public class SuperNodeListAdapter extends BaseQuickAdapter<VoteListBean.DataBean.ResultBean.ProducersBean, BaseViewHolder> {

    private final GlideRequest<Bitmap> glideRequest;
    private Context context;


    private int pos;
    // private boolean is;

    public SuperNodeListAdapter(Context context, @Nullable List<VoteListBean.DataBean.ResultBean.ProducersBean> data, int pos, boolean is) {
        super(R.layout.item_super_node_list, data);
        this.context = context;
        this.pos = pos;
        //  this.is = is;
        glideRequest = GlideApp.with(context).asBitmap().error(R.mipmap.found_vote_initial).placeholder(R.mipmap.found_vote_initial)
        ;
    }

    @Override
    protected void convert(BaseViewHolder helper, VoteListBean.DataBean.ResultBean.ProducersBean bean) {
        helper.setBackgroundColor(R.id.ll, context.getResources().getColor(R.color.transparent));
        if (pos == helper.getLayoutPosition()) {
            helper.setBackgroundColor(R.id.ll, Color.parseColor("#307CA2"));
        }
        helper.setText(R.id.tv_name, bean.getNickname());
        helper.setText(R.id.tv_num, new BigDecimal(bean.getVotes()).intValue() + " " +context.getString(R.string.ticket));
        ImageView iv = helper.getView(R.id.iv_icon);
        iv.setImageResource(R.mipmap.found_vote_initial);
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
                        iv.setImageResource(R.mipmap.found_vote_initial);
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
