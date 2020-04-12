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

package org.elastos.wallet.ela.ui.vote.NodeCart;

import android.graphics.Bitmap;
import android.support.v7.widget.AppCompatCheckBox;
import android.support.v7.widget.AppCompatImageView;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.BaseAdapter;
import android.widget.CheckBox;
import android.widget.ImageView;
import android.widget.TextView;

import org.elastos.wallet.R;
import org.elastos.wallet.ela.base.BaseFragment;
import org.elastos.wallet.ela.ui.vote.SuperNodeList.NodeDotJsonViewData;
import org.elastos.wallet.ela.ui.vote.SuperNodeList.NodeInfoBean;
import org.elastos.wallet.ela.ui.vote.SuperNodeList.SuperNodeListPresenter;
import org.elastos.wallet.ela.ui.vote.bean.VoteListBean;
import org.elastos.wallet.ela.utils.AppUtlis;
import org.elastos.wallet.ela.utils.Arith;
import org.elastos.wallet.ela.utils.NumberiUtil;
import org.elastos.wallet.ela.utils.svg.GlideApp;
import org.elastos.wallet.ela.utils.svg.GlideRequest;

import java.math.BigDecimal;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

/**
 * 通过每个条目的点击事件确定是否选中
 */
public class MyAdapter extends BaseAdapter {
    // 填充数据的list
    private List<VoteListBean.DataBean.ResultBean.ProducersBean> list;
    // 用来控制CheckBox的选中状况
    private HashMap<Integer, Boolean> dataMap;
    // 上下文
    private BaseFragment context;

    private Map<String, String> map;

    // 构造器
    MyAdapter(List<VoteListBean.DataBean.ResultBean.ProducersBean> list, BaseFragment context) {
        this.context = context;
        this.list = list;

        dataMap = new HashMap<Integer, Boolean>();
        // 初始化数据
        initDateStaus(false);
        if (map == null) {
            map = new HashMap<>();
        } else {
            map.clear();
        }
    }

    // 初始化dataMap的数据
    void initDateStaus(boolean status) {
        if (list != null) {
            for (int i = 0; i < list.size(); i++) {
                dataMap.put(i, status);
            }
        }
    }

    void setDateStaus(int size, boolean status) {
        if (list != null) {
            for (int i = 0; i < size; i++) {
                dataMap.put(i, status);
            }
        }
    }

    int getCheckNum() {
        int sum = 0;
        if (list != null) {
            for (int i = 0; i < list.size(); i++) {
                if (dataMap.get(i) != null && dataMap.get(i)) {
                    sum++;
                }
            }
        }
        return sum;
    }

    @Override
    public int getCount() {
        return list.size();
    }

    @Override
    public Object getItem(int position) {
        return list.get(position);
    }

    @Override
    public long getItemId(int position) {
        return position;
    }

    @Override
    public View getView(int position, View convertView, ViewGroup parent) {
        ViewHolder holder = null;
        if (convertView == null) {
            holder = new ViewHolder();
            convertView = LayoutInflater.from(context.getContext()).inflate(R.layout.item_node_car, null);
            //holder.tv = (TextView) convertView.findViewById(R.id.item_tv);
            holder.cb = (AppCompatCheckBox) convertView.findViewById(R.id.checkbox);
            holder.tv_name = (TextView) convertView.findViewById(R.id.tv_name);
            holder.tv_address = (TextView) convertView.findViewById(R.id.tv_address);
            holder.tv_id = (TextView) convertView.findViewById(R.id.tv_id);
            holder.tv_zb = (TextView) convertView.findViewById(R.id.tv_zb);
            holder.ivIcon = convertView.findViewById(R.id.iv_icon);
            // 为view设置标签
            convertView.setTag(holder);
        } else {
            // 取出holder
            holder = (ViewHolder) convertView.getTag();
        }
        // 设置list中TextView的显示
        //   holder.tv.setText(list.get(position));
        // 根据isSelected来设置checkbox的选中状况
        if (dataMap.get(position) != null) {
            holder.cb.setChecked(dataMap.get(position));
        }
        VoteListBean.DataBean.ResultBean.ProducersBean producersBean = list.get(position);
        BigDecimal voterateDecimal = new BigDecimal(producersBean.getVoterate());
        if (voterateDecimal.compareTo(new BigDecimal(0.01)) < 0) {
            holder.tv_zb.setText("< 1%");
        } else {
            String voterate = NumberiUtil.numberFormat(Arith.mul(voterateDecimal, 100), 2);
            holder.tv_zb.setText(voterate + "%");
        }
        holder.tv_name.setText(producersBean.getNickname());
        holder.tv_address.setText(AppUtlis.getLoc(context.getBaseActivity(), producersBean.getLocation() + ""));
        int id = producersBean.getIndex() + 1;
        holder.tv_id.setText("NO." + id);//12
        AppCompatImageView iv = holder.ivIcon;
        iv.setImageResource(R.mipmap.found_vote_initial_circle);
        String baseUrl = producersBean.getUrl();
        iv.setTag(R.id.error_tag_empty, baseUrl);
        GlideApp.with(context).clear(iv);
        if (baseUrl == null) {
            return convertView;
        }
        if (map.get(baseUrl) != null) {
            if ("".equals(map.get(baseUrl))) {
                return convertView;
            }
            GlideApp.with(context).load(map.get(baseUrl)).error(R.mipmap.found_vote_initial_circle).circleCrop().into(iv);
            return convertView;
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
                GlideApp.with(context).load(imgUrl).error(R.mipmap.found_vote_initial_circle).circleCrop().into(iv1);
            }
        });


        return convertView;
    }

    public List<VoteListBean.DataBean.ResultBean.ProducersBean> getAllSelectList() {
        List<VoteListBean.DataBean.ResultBean.ProducersBean> selectList = new ArrayList();
        if (list != null) {
            for (int i = 0; i < list.size(); i++) {
                if (dataMap.get(i) != null && dataMap.get(i)) {
                    selectList.add(list.get(i));
                }
            }
        }
        return selectList;
    }

    public HashMap<Integer, Boolean> getDataMap() {
        return dataMap;
    }

    public void setDataMap(HashMap<Integer, Boolean> dataMap) {
        this.dataMap = dataMap;
    }

    public List<VoteListBean.DataBean.ResultBean.ProducersBean> getList() {
        return list;
    }

    public void setList(List<VoteListBean.DataBean.ResultBean.ProducersBean> list) {
        this.list = list;
    }

    public class ViewHolder {
        TextView tv_name;
        TextView tv_address;
        TextView tv_id;
        AppCompatImageView ivIcon;


        CheckBox cb;
        TextView tv_zb;

        public CheckBox getCb() {
            return cb;
        }

        public void setCb(CheckBox cb) {
            this.cb = cb;
        }

    }

}
