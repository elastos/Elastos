package org.elastos.wallet.ela.ui.crvote.adapter;

import android.graphics.Bitmap;
import android.support.annotation.Nullable;
import android.widget.ImageView;

import com.chad.library.adapter.base.BaseViewHolder;

import org.elastos.wallet.R;
import org.elastos.wallet.ela.MyApplication;
import org.elastos.wallet.ela.base.BaseFragment;
import org.elastos.wallet.ela.bean.ImageBean;
import org.elastos.wallet.ela.ui.crvote.bean.CRListBean;
import org.elastos.wallet.ela.ui.vote.SuperNodeList.NodeDotJsonViewData;
import org.elastos.wallet.ela.ui.vote.SuperNodeList.NodeInfoBean;
import org.elastos.wallet.ela.ui.vote.SuperNodeList.SuperNodeListPresenter;
import org.elastos.wallet.ela.utils.GlideApp;
import org.elastos.wallet.ela.utils.GlideRequest;

import java.util.List;

public class CRListAdapter1 extends CRListAdapterFather {

    private final GlideRequest<Bitmap> glideRequest;
    private SuperNodeListPresenter presenter;


    public CRListAdapter1(BaseFragment context, @Nullable List<CRListBean.DataBean.ResultBean.CrcandidatesinfoBean> data, boolean is) {
        super(R.layout.item_super_node_list1, context, data, is);

        glideRequest = GlideApp.with(context).asBitmap().error(R.mipmap.found_vote_initial_circle)
                .placeholder(R.mipmap.found_vote_initial_circle).circleCrop();
    }


    @Override
    protected void convert(BaseViewHolder helper, CRListBean.DataBean.ResultBean.CrcandidatesinfoBean bean) {
        helper.setBackgroundColor(R.id.ll, mContext.getResources().getColor(R.color.black));
        super.convert(helper, bean);
        int no = helper.getLayoutPosition();

        if (is && 0 == helper.getLayoutPosition()) {
            no = pos;
        } else if (is) {
            no = (bean.getIndex() < data.get(0).getIndex()) ? no - 1 : no;
        }
        helper.setText(R.id.tv_rank, "" + (no + 1));
        ImageView iv = helper.getView(R.id.iv_icon);
        iv.setImageResource(R.mipmap.found_vote_initial_circle);
        String baseUrl = bean.getUrl();
        iv.setTag(R.id.error_tag_empty, baseUrl);
        GlideApp.with(mContext).clear(iv);
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
        presenter.getCRUrlJson(iv, baseUrl, context, new NodeDotJsonViewData() {
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
                //   glideRequest.load(imgUrl).into(iv1);
                presenter.getImage(iv1, url, imgUrl, context, new NodeDotJsonViewData() {
                    @Override
                    public void onError(String url) {
                        map.put(url, "");
                    }

                    @Override
                    public void onGetImage(ImageView iv1, String url, ImageBean imageBean) {
                        if (iv1.getTag(R.id.error_tag_empty) == null || !(iv1.getTag(R.id.error_tag_empty).toString()).equals(url)) {
                            GlideApp.with(mContext).clear(iv1);
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

    public boolean isShowCheckbox() {
        return showCheckbox;
    }

    public void setShowCheckbox(boolean showCheckbox) {
        this.showCheckbox = showCheckbox;
        notifyDataSetChanged();
    }
}

