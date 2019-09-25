package org.elastos.wallet.ela.ui.crvote.adapter;

import android.graphics.Bitmap;
import android.support.annotation.Nullable;
import android.widget.ImageView;

import com.chad.library.adapter.base.BaseViewHolder;

import org.elastos.wallet.R;
import org.elastos.wallet.ela.base.BaseFragment;
import org.elastos.wallet.ela.ui.crvote.bean.CRListBean;
import org.elastos.wallet.ela.ui.vote.SuperNodeList.NodeDotJsonViewData;
import org.elastos.wallet.ela.ui.vote.SuperNodeList.NodeInfoBean;
import org.elastos.wallet.ela.ui.vote.SuperNodeList.SuperNodeListPresenter;
import org.elastos.wallet.ela.utils.AppUtlis;
import org.elastos.wallet.ela.utils.GlideApp;
import org.elastos.wallet.ela.utils.GlideRequest;
import org.elastos.wallet.ela.utils.NumberiUtil;

import java.math.BigDecimal;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

public class CRListAdapter1 extends CRListAdapterFather {

    private final GlideRequest<Bitmap> glideRequest;


    public CRListAdapter1(BaseFragment context, @Nullable List<CRListBean.DataBean.ResultBean.CrcandidatesinfoBean> data, boolean is,String totalvotes) {
        super(R.layout.item_super_node_list1, context, data, is,totalvotes);

        glideRequest = GlideApp.with(context).asBitmap().error(R.mipmap.found_vote_initial_circle)
                .placeholder(R.mipmap.found_vote_initial_circle).circleCrop();
    }


    @Override
    protected void convert(BaseViewHolder helper, CRListBean.DataBean.ResultBean.CrcandidatesinfoBean bean) {
        super.convert(helper, bean);
        helper.setBackgroundColor(R.id.ll, context.getResources().getColor(R.color.black));
        helper.setText(R.id.tv_rank, "" + (bean.getIndex() + 1));
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

        new SuperNodeListPresenter().getUrlJson(iv, baseUrl, context, new NodeDotJsonViewData() {
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
                map.put(url, imgUrl);
                glideRequest.load(imgUrl).into(iv1);
            }
        });
    }

    public boolean isShowCheckbox() {
        return showCheckbox;
    }

    public void setShowCheckbox(boolean showCheckbox) {
        this.showCheckbox = showCheckbox;
        notifyDataSetChanged();
    }
}

