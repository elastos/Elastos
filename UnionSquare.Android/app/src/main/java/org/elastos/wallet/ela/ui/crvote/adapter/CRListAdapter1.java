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
import android.text.TextUtils;
import android.util.Base64;
import android.widget.ImageView;

import com.alibaba.fastjson.JSON;
import com.chad.library.adapter.base.BaseViewHolder;

import org.elastos.wallet.R;
import org.elastos.wallet.ela.base.BaseFragment;
import org.elastos.wallet.ela.rxjavahelp.BaseEntity;
import org.elastos.wallet.ela.ui.crvote.bean.CRListBean;
import org.elastos.wallet.ela.ui.did.entity.CredentialSubjectBean;
import org.elastos.wallet.ela.ui.did.entity.GetJwtRespondBean;
import org.elastos.wallet.ela.ui.vote.SuperNodeList.NodeDotJsonViewData;
import org.elastos.wallet.ela.ui.vote.SuperNodeList.SuperNodeListPresenter;
import org.elastos.wallet.ela.utils.JwtUtils;
import org.elastos.wallet.ela.utils.svg.GlideApp;

import java.util.List;

public class CRListAdapter1 extends CRListAdapterFather {

    private SuperNodeListPresenter presenter;


    public CRListAdapter1(BaseFragment context, @Nullable List<CRListBean.DataBean.ResultBean.CrcandidatesinfoBean> data, boolean is) {
        super(R.layout.item_super_node_list1, context, data, is);


    }


    @Override
    protected void convert(BaseViewHolder helper, CRListBean.DataBean.ResultBean.CrcandidatesinfoBean bean) {
        helper.setBackgroundColor(R.id.ll, mContext.getResources().getColor(R.color.black));
        super.convert(helper, bean);
      /*  int no = helper.getLayoutPosition();

        if (is && 0 == helper.getLayoutPosition()) {
            no = pos;
        } else if (is) {
            no = (bean.getIndex() < data.get(0).getIndex()) ? no - 1 : no;
        }*/
        helper.setText(R.id.tv_rank, "" + (bean.getIndex() + 1));
        ImageView iv = helper.getView(R.id.iv_icon);
        iv.setImageResource(R.mipmap.found_vote_initial_circle);
        String baseUrl = bean.getCid();
        iv.setTag(R.id.error_tag_empty, baseUrl);
        GlideApp.with(mContext).clear(iv);
        if (baseUrl == null) {
            return;
        }
        if (map.get(baseUrl) != null) {
            if ("".equals(map.get(baseUrl))) {
                return;
            }
            GlideApp.with(context).load(map.get(baseUrl)).error(R.mipmap.found_vote_initial_circle)
                  .circleCrop().into(iv);
            return;
        }
        if (presenter == null) {
            presenter = new SuperNodeListPresenter();
        }
        presenter.getCRUrlJson(iv, baseUrl, context, new NodeDotJsonViewData() {
            @Override
            public void onError(String url) {
                GlideApp.with(mContext).clear(iv);
                iv.setImageResource(R.mipmap.found_vote_initial_circle);
                map.put(url, "");
            }

            @Override
            public void onGetNodeDotJsonData(ImageView iv1, BaseEntity baseEntity, String url) {
                //这个时候的iv已经不是那个iv了  所有传递iv试试
                if (iv1.getTag(R.id.error_tag_empty) == null || !(iv1.getTag(R.id.error_tag_empty).toString()).equals(url)) {
                    GlideApp.with(mContext).clear(iv1);
                    iv1.setImageResource(R.mipmap.found_vote_initial_circle);
                    return;
                }
                GetJwtRespondBean getJwtRespondBean = (GetJwtRespondBean) baseEntity;
                String jwt = getJwtRespondBean.getData().getJwt();
                if (!TextUtils.isEmpty(jwt)) {
                    String payload = JwtUtils.getJwtPayload(jwt);
                    String pro = context.getMyDID().getCredentialProFromJson(payload);
                    CredentialSubjectBean credentialSubjectBean = JSON.parseObject(pro, CredentialSubjectBean.class);
                    if (credentialSubjectBean == null || credentialSubjectBean.getAvatar() == null) {
                        map.put(url, "");
                        return;
                    }
                    String imgUrl = credentialSubjectBean.getAvatar();
                    map.put(url, imgUrl);
                    GlideApp.with(context).load(imgUrl).error(R.mipmap.found_vote_initial_circle).circleCrop().into(iv1);
                }

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

