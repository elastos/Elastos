package org.elastos.wallet.ela.ui.Assets.fragment.mulsignwallet;

import android.os.Bundle;
import android.support.v7.widget.LinearLayoutManager;
import android.support.v7.widget.RecyclerView;
import android.text.TextUtils;
import android.view.View;
import android.widget.LinearLayout;
import android.widget.TextView;

import com.google.gson.JsonArray;
import com.google.gson.JsonObject;
import com.google.gson.JsonParser;

import org.elastos.wallet.R;
import org.elastos.wallet.ela.base.BaseFragment;
import org.elastos.wallet.ela.db.table.Wallet;
import org.elastos.wallet.ela.rxjavahelp.BaseEntity;
import org.elastos.wallet.ela.rxjavahelp.NewBaseViewData;
import org.elastos.wallet.ela.ui.Assets.adapter.PublicKeyRecAdapter;
import org.elastos.wallet.ela.ui.Assets.presenter.WallletManagePresenter;
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
        new WallletManagePresenter().getPubKeyInfo(wallet.getWalletId(), this);
    }


    private void setRecycleView(JsonArray publicKeyRing) {
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
            JsonObject jsonData = new JsonParser().parse(data).getAsJsonObject();
            String derivationStrategy = jsonData.get("derivationStrategy").getAsString();
            int n = jsonData.get("n").getAsInt();
            tvPknum.setText(n + "");
            String m = jsonData.get("m").getAsString();
            tvSignnum.setText(m);
            String requestPubKey;
            if ("BIP44".equals(derivationStrategy) && n > 1) {
                requestPubKey = jsonData.get("xPubKey").getAsString();

            } else {
                requestPubKey = jsonData.get("xPubKeyHDPM").getAsString();
            }
            if (!TextUtils.isEmpty(requestPubKey)) {
                llCurrentpk.setVisibility(View.VISIBLE);
                tvCurrentpk.setText(requestPubKey);
            }


            JsonArray publicKeyRing = jsonData.get("publicKeyRing").getAsJsonArray();
            setRecycleView(publicKeyRing);
        } catch (Exception e) {
            showToast(getString(R.string.error_30000));
        }
    }
}

