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

package org.elastos.wallet.ela.ui.vote.SuperNodeList;

import android.support.annotation.Nullable;
import android.widget.ImageView;

import com.chad.library.adapter.base.BaseQuickAdapter;
import com.chad.library.adapter.base.BaseViewHolder;

import org.elastos.wallet.R;
import org.elastos.wallet.ela.MyApplication;
import org.elastos.wallet.ela.base.BaseFragment;
import org.elastos.wallet.ela.bean.ImageBean;
import org.elastos.wallet.ela.ui.vote.bean.VoteListBean;
import org.elastos.wallet.ela.utils.svg.GlideApp;

import java.math.BigDecimal;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

public class SuperNodeListAdapter extends BaseQuickAdapter<VoteListBean.DataBean.ResultBean.ProducersBean, BaseViewHolder> {


    private BaseFragment context;
    private Map<String, String> map;


    private boolean is;
    private SuperNodeListPresenter presenter;

    public SuperNodeListAdapter(BaseFragment context, @Nullable List<VoteListBean.DataBean.ResultBean.ProducersBean> data, boolean is) {
        super(R.layout.item_super_node_list, data);
        this.context = context;
        //this.mContext = context.getContext();
        this.is = is;
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
        helper.setBackgroundColor(R.id.ll, mContext.getResources().getColor(R.color.transparent));
        if (is && helper.getLayoutPosition() == 0) {
            helper.setBackgroundColor(R.id.ll, mContext.getResources().getColor(R.color.blue1));
        }
        helper.setText(R.id.tv_name, bean.getNickname());
        helper.setText(R.id.tv_num, new BigDecimal(bean.getVotes()).intValue() + " " + mContext.getString(R.string.ticket));
        ImageView iv = helper.getView(R.id.iv_icon);
        iv.setImageResource(R.mipmap.found_vote_initial);
        String baseUrl = bean.getUrl();
        iv.setTag(R.id.error_tag_empty, null);
        GlideApp.with(mContext).clear(iv);
        if (baseUrl == null) {
            return;
        }
        iv.setTag(R.id.error_tag_empty, baseUrl);
        if (map.get(baseUrl) != null) {
            if ("".equals(map.get(baseUrl))) {
                return;
            }
            GlideApp.with(context).load(map.get(baseUrl)).error(R.mipmap.found_vote_initial).into(iv);
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
                //map.put(url, imgUrl);
                //glideRequest.load(imgUrl).into(iv1);
                //获得url 上传url

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
                        GlideApp.with(context).load(newimgUrl).error(R.mipmap.found_vote_initial).into(iv1);
                    }
                });


            }
        });
    }

}
