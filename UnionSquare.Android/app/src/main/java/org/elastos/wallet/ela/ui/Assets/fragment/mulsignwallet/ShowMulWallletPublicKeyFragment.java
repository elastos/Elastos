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
import org.elastos.wallet.ela.ui.Assets.adapter.PublicKeyRecAdapter;
import org.elastos.wallet.ela.ui.Assets.presenter.WallletManagePresenter;
import org.elastos.wallet.ela.ui.common.viewdata.CommmonStringWithMethNameViewData;
import org.elastos.wallet.ela.utils.DividerItemDecoration;
import org.elastos.wallet.ela.utils.ScreenUtil;

import butterknife.BindView;
import butterknife.Unbinder;

public class ShowMulWallletPublicKeyFragment extends BaseFragment implements CommmonStringWithMethNameViewData {
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
        new WallletManagePresenter().exportReadonlyWallet(wallet.getWalletId(), this);
    }


    @Override
    public void onGetCommonData(String methodname, String data) {
        //{"CoinInfoList":[{"ChainID":"ELA","EarliestPeerTime":1561716528,"FeePerKB":10000,"VisibleAssets":["a3d0eaa466df74983b5d7c543de6904f4c9418ead5ffd6d25814234a96db37b0"]}],"OwnerPubKey":"03d916c2072fd8fb57224e9747e0f1e36a2c117689cedf39e0132f3cb4f8ee673d","SingleAddress":false,"m":1,"mnemonicHasPassphrase":false,"n":1,"network":"","publicKeyRing":[{"requestPubKey":"0370a77a257aa81f46629865eb8f3ca9cb052fcfd874e8648cfbea1fbf071b0280","xPubKey":"xpub6D5r16bFTY3FfNht7kobqQzkAHsUxzfKingYXXYUoTfNDSqCW2yjhHdt9yWRwtxx4zWoJ1m3pEo6hzQTswEA2UeEB16jEnYiHoDFwGH9c9z"}],"requestPubKey":"0370a77a257aa81f46629865eb8f3ca9cb052fcfd874e8648cfbea1fbf071b0280","xPubKey":"xpub6D5r16bFTY3FfNht7kobqQzkAHsUxzfKingYXXYUoTfNDSqCW2yjhHdt9yWRwtxx4zWoJ1m3pEo6hzQTswEA2UeEB16jEnYiHoDFwGH9c9z"}
        try {
            JsonObject jsonData = new JsonParser().parse(data).getAsJsonObject();
            String requestPubKey = jsonData.get("requestPubKey").getAsString();
            if (!TextUtils.isEmpty(requestPubKey)) {
                llCurrentpk.setVisibility(View.VISIBLE);
                tvCurrentpk.setText(requestPubKey);
            }
            String n = jsonData.get("n").getAsString();
            tvPknum.setText(n);
            String m = jsonData.get("m").getAsString();
            tvSignnum.setText(m);
            JsonArray publicKeyRing = jsonData.get("publicKeyRing").getAsJsonArray();
            setRecycleView(publicKeyRing);
        } catch (Exception e) {
            showToast(getString(R.string.error_30000));
        }

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


}

