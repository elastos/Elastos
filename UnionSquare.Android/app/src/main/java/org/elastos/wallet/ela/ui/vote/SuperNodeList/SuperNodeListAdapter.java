package org.elastos.wallet.ela.ui.vote.SuperNodeList;

import android.graphics.Bitmap;
import android.support.annotation.Nullable;
import android.widget.ImageView;

import com.bumptech.glide.RequestBuilder;
import com.chad.library.adapter.base.BaseQuickAdapter;
import com.chad.library.adapter.base.BaseViewHolder;

import org.elastos.wallet.R;
import org.elastos.wallet.ela.MyApplication;
import org.elastos.wallet.ela.base.BaseFragment;
import org.elastos.wallet.ela.bean.ImageBean;
import org.elastos.wallet.ela.ui.vote.bean.VoteListBean;
import org.elastos.wallet.ela.utils.GlideApp;
import org.elastos.wallet.ela.utils.GlideRequest;
import org.elastos.wallet.ela.utils.Log;

import java.math.BigDecimal;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

public class SuperNodeListAdapter extends BaseQuickAdapter<VoteListBean.DataBean.ResultBean.ProducersBean, BaseViewHolder> {


    private final GlideRequest<Bitmap> glideRequest;
    private BaseFragment context;
    private Map<String, String> map;


    private boolean is;
    private SuperNodeListPresenter presenter;

    public SuperNodeListAdapter(BaseFragment context, @Nullable List<VoteListBean.DataBean.ResultBean.ProducersBean> data, boolean is) {
        super(R.layout.item_super_node_list, data);
        this.context = context;
        //this.mContext = context.getContext();
        this.is = is;
        glideRequest = GlideApp.with(context).asBitmap().error(R.mipmap.found_vote_initial).placeholder(R.mipmap.found_vote_initial);
        if (map == null) {
            map = new HashMap<>();
        } else {
            map.clear();
        }

    }

    @Override
    protected void convert(BaseViewHolder helper, VoteListBean.DataBean.ResultBean.ProducersBean bean) {
        helper.setBackgroundColor(R.id.ll, context.getResources().getColor(R.color.transparent));
        if (is && helper.getLayoutPosition() == 0) {
            helper.setBackgroundColor(R.id.ll, context.getResources().getColor(R.color.blue1));
        }
        helper.setText(R.id.tv_name, bean.getNickname());
        helper.setText(R.id.tv_num, new BigDecimal(bean.getVotes()).intValue() + " " + context.getString(R.string.ticket));
        ImageView iv = helper.getView(R.id.iv_icon);
        iv.setImageResource(R.mipmap.found_vote_initial);
        String baseUrl = bean.getUrl();
        iv.setTag(R.id.error_tag_empty, null);
        GlideApp.with(context).clear(iv);
        if (baseUrl == null) {
            return;
        }
        iv.setTag(R.id.error_tag_empty, baseUrl);
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
                map.put(url, imgUrl);
                 glideRequest.load(imgUrl).into(iv1);
                //获得url 上传url

             /*   presenter.getImage(iv1, url, imgUrl, context, new NodeDotJsonViewData() {
                    @Override
                    public void onError(String url) {
                        map.put(url, "");
                    }
                    @Override
                    public void onGetImage(ImageView iv1, String url, ImageBean imageBean) {
                        String newimgUrl = MyApplication.REQUEST_BASE_URL + "/" + imageBean.getData();
                        Log.d("???????", imgUrl);
                        Log.d("???????", newimgUrl);
                        map.put(url, newimgUrl);
                        glideRequest.load(newimgUrl).into(iv1);
                    }
                });*/


            }
        });
    }

}
