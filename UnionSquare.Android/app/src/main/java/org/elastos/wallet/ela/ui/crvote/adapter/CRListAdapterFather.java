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

package org.elastos.wallet.ela.ui.crvote.adapter;

import android.support.annotation.Nullable;

import com.chad.library.adapter.base.BaseQuickAdapter;
import com.chad.library.adapter.base.BaseViewHolder;

import org.elastos.wallet.R;
import org.elastos.wallet.ela.base.BaseFragment;
import org.elastos.wallet.ela.ui.crvote.bean.CRListBean;
import org.elastos.wallet.ela.utils.AppUtlis;

import java.math.BigDecimal;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

/**
 * 方案1
 * 点击选中时使用checckPosition记录选中的位置  取消选中时remove
 * 并没有真的加入data   点击批量加入的时遍历checckPosition的到所有点击选中的list
 * 方案2  现用
 * 用CRListBean的check字段 记得在不同状态下重置状态
 */
public class CRListAdapterFather extends BaseQuickAdapter<CRListBean.DataBean.ResultBean.CrcandidatesinfoBean, BaseViewHolder> {
    BaseFragment context;
    boolean showCheckbox;//是否展示加入购物车页面ui
    boolean is;//是否有当前钱包参选的节点
    Map<String, String> map;
    //  Set<Integer> checkPosition = new HashSet<>();//用来记录被check的position
    List<CRListBean.DataBean.ResultBean.CrcandidatesinfoBean> data;


    CRListAdapterFather(int id, BaseFragment context, @Nullable List<CRListBean.DataBean.ResultBean.CrcandidatesinfoBean> data, boolean is) {
        super(id, data);
        this.context = context;

        this.data = data;
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


    public boolean isShowCheckbox() {
        return showCheckbox;
    }

    public void setShowCheckbox(boolean showCheckbox) {
        //checkPosition.clear();
        this.showCheckbox = showCheckbox;
        notifyDataSetChanged();
    }

    /* public Set<Integer> getChecckPosition() {
         return checkPosition;
     }

     public void setChecckPosition(Set<Integer> checckPosition) {
         this.checkPosition = checckPosition;
     }
 */
    @Override
    protected void convert(BaseViewHolder helper, CRListBean.DataBean.ResultBean.CrcandidatesinfoBean bean) {
        helper.setGone(R.id.tv_address, !showCheckbox);
        helper.setGone(R.id.tv_num, !showCheckbox);
        helper.setGone(R.id.tv_zb, !showCheckbox);
        helper.setGone(R.id.checkbox, showCheckbox);
        if (bean.isSelect()) {
            helper.getView(R.id.checkbox).setEnabled(false);
        } else {
            helper.getView(R.id.checkbox).setEnabled(true);
        }
        helper.setChecked(R.id.checkbox, bean.isChecked());

        if (is && 0 == helper.getLayoutPosition()) {
            helper.setBackgroundColor(R.id.ll, mContext.getResources().getColor(R.color.blue1));
        }
        helper.setText(R.id.tv_name, bean.getNickname());
        helper.setText(R.id.tv_address, AppUtlis.getLoc(mContext, bean.getLocation() + ""));

        helper.setText(R.id.tv_zb, bean.getVoterate() + "%");
        helper.setText(R.id.tv_num, new BigDecimal(bean.getVotes()).intValue() + " " + mContext.getString(R.string.ticket));
    }

    /*   @Override
   public void onViewRecycled(BaseViewHolder holder)//这个方法是Adapter里面的
   {
       if (holder != null) {
           GlideApp.with(context).clear((ImageView) holder.getView(R.id.iv_icon));
       }
       super.onViewRecycled(holder);

   }*/
    public void addAllPositionAndNotify() {
        for (int i = 0; i < getItemCount(); i++) {
            if (!data.get(i).isSelect()) {
                //select的保持不动  这样是为了保存购物车数据=isSelect的数据+checkPosition的数据
                //  checkPosition.add(i);
                data.get(i).setChecked(true);
            }


        }
        notifyDataSetChanged();
    }

    public void removeAllPosition() {
        for (CRListBean.DataBean.ResultBean.CrcandidatesinfoBean bean : data) {
            bean.setChecked(false);
        }
    }

    public void removeAllPositionAndNotify() {
        removeAllPosition();
        notifyDataSetChanged();
    }

}
