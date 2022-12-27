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

package org.elastos.wallet.ela.ui.Assets.fragment.mulsignwallet;

import android.os.Bundle;
import android.support.v7.widget.LinearLayoutManager;
import android.support.v7.widget.RecyclerView;
import android.text.TextUtils;
import android.view.View;
import android.widget.LinearLayout;
import android.widget.TextView;

import com.alibaba.fastjson.JSON;
import com.alibaba.fastjson.JSONArray;
import com.alibaba.fastjson.JSONObject;

import org.elastos.wallet.R;
import org.elastos.wallet.ela.base.BaseFragment;
import org.elastos.wallet.ela.db.table.Wallet;
import org.elastos.wallet.ela.rxjavahelp.BaseEntity;
import org.elastos.wallet.ela.rxjavahelp.NewBaseViewData;
import org.elastos.wallet.ela.ui.Assets.adapter.PublicKeyRecAdapter;
import org.elastos.wallet.ela.ui.Assets.presenter.WalletManagePresenter;
import org.elastos.wallet.ela.ui.common.bean.CommmonStringEntity;
import org.elastos.wallet.ela.utils.DividerItemDecoration;
import org.elastos.wallet.ela.utils.ScreenUtil;

import butterknife.BindView;

public class ShowMulWallletPublicKeyFragment extends BaseFragment implements NewBaseViewData {
    @BindView(R.id.tv_title)
    TextView tvTitle;
    @BindView(R.id.tv_signnum)
    TextView tvSignnum;
    @BindView(R.id.tv_pknum)
    TextView tvPknum;
    @BindView(R.id.tv_currentpk)
    TextView tvCurrentpk;
    @BindView(R.id.ll_currentpk)
    LinearLayout llCurrentpk;
    @BindView(R.id.rv)
    RecyclerView rv;

    private Wallet wallet;


    @Override
    protected int getLayoutId() {
        return R.layout.fragment_showmulwalletpublickey;
    }

    @Override
    protected void setExtraData(Bundle data) {
        wallet = data.getParcelable("wallet");
    }

    @Override
    protected void initView(View view) {
        tvTitle.setText(getString(R.string.pklist));
        new WalletManagePresenter().getPubKeyInfo(wallet.getWalletId(), this);
    }


    private void setRecycleView(JSONArray publicKeyRing) {
        rv.setNestedScrollingEnabled(false);
        rv.setFocusableInTouchMode(false);
        PublicKeyRecAdapter adapter = new PublicKeyRecAdapter(getContext(), publicKeyRing);
        DividerItemDecoration decoration = new DividerItemDecoration(getActivity(), DividerItemDecoration.HORIZONTAL_LIST, ScreenUtil.dp2px(getContext(), 0.5), R.color.whiter);
        rv.addItemDecoration(decoration);
        rv.setLayoutManager(new LinearLayoutManager(getContext(), LinearLayoutManager.VERTICAL, false));
        rv.setAdapter(adapter);
    }


    @Override
    public void onGetData(String methodName, BaseEntity baseEntity, Object o) {
        //{
        //	"derivationStrategy":"BIP44",
        //	"m":1,
        //	"n":1,
        //	"publicKeyRing":[
        // "xpub68R18fSmxfwBJ9dEm9SokS1hx5ZTd1nRtbioJ8qrgMJLai9nPpFucvf5Fq5DS1w7qZZs5UKtZDJCDAH3She2vbgpPbdoPrMMmSBFNeDrEPK"],
        //"xPubKey":"xpub6D5r16bFTY3FfNht7kobqQzkAHsUxzfKingYXXYUoTfNDSqCW2yjhHdt9yWRwtxx4zWoJ1m3pEo6hzQTswEA2UeEB16jEnYiHoDFwGH9c9z",
        //"xPubKeyHDPM":"xpub68R18fSmxfwBJ9dEm9SokS1hx5ZTd1nRtbioJ8qrgMJLai9nPpFucvf5Fq5DS1w7qZZs5UKtZDJCDAH3She2vbgpPbdoPrMMmSBFNeDrEPK"
        //}
        String data = ((CommmonStringEntity) baseEntity).getData();
        try {
            JSONObject jsonData = JSON.parseObject(data);
            String derivationStrategy = jsonData.getString("derivationStrategy");
            int n = jsonData.getIntValue("n");
            tvPknum.setText(n + "");
            String m = jsonData.getString("m");
            tvSignnum.setText(m);
            String requestPubKey;
            if ("BIP44".equals(derivationStrategy) && n > 1) {
                requestPubKey = jsonData.getString("xPubKey");

            } else {
                requestPubKey = jsonData.getString("xPubKeyHDPM");
            }
            if (!TextUtils.isEmpty(requestPubKey)) {
                llCurrentpk.setVisibility(View.VISIBLE);
                tvCurrentpk.setText(requestPubKey);
            }


            JSONArray publicKeyRing = jsonData.getJSONArray("publicKeyRing");
            setRecycleView(publicKeyRing);
        } catch (Exception e) {
            showToast(getString(R.string.error_30000));
        }
    }
}

