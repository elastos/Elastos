package org.elastos.wallet.ela.ui.Assets.fragment.transfer;

import android.content.Intent;
import android.graphics.Bitmap;
import android.os.Bundle;
import android.text.TextUtils;
import android.view.View;
import android.widget.ImageView;
import android.widget.TextView;

import com.google.gson.JsonObject;
import com.google.gson.JsonParser;

import org.elastos.wallet.R;
import org.elastos.wallet.ela.ElaWallet.MyWallet;
import org.elastos.wallet.ela.base.BaseFragment;
import org.elastos.wallet.ela.bean.BusEvent;
import org.elastos.wallet.ela.db.table.Wallet;
import org.elastos.wallet.ela.ui.Assets.activity.PwdActivity;
import org.elastos.wallet.ela.ui.Assets.bean.TransferRecordDetailEntity;
import org.elastos.wallet.ela.ui.Assets.fragment.CreateSignReadOnlyWalletFragment;
import org.elastos.wallet.ela.ui.Assets.fragment.mulsignwallet.CreateMulWalletFragment;
import org.elastos.wallet.ela.ui.Assets.presenter.PwdPresenter;
import org.elastos.wallet.ela.ui.Assets.presenter.TransferPresenter;
import org.elastos.wallet.ela.ui.common.viewdata.CommmonStringWithMethNameViewData;
import org.elastos.wallet.ela.utils.Arith;
import org.elastos.wallet.ela.utils.ClipboardUtil;
import org.elastos.wallet.ela.utils.Constant;
import org.elastos.wallet.ela.utils.Log;
import org.elastos.wallet.ela.utils.NumberiUtil;
import org.elastos.wallet.ela.utils.QRCodeUtils;
import org.elastos.wallet.ela.utils.RxEnum;
import org.elastos.wallet.ela.utils.ScanQRcodeUtil;
import org.elastos.wallet.ela.utils.ScreenUtil;
import org.elastos.wallet.ela.utils.ShareUtil;
import org.greenrobot.eventbus.Subscribe;
import org.greenrobot.eventbus.ThreadMode;

import java.util.HashMap;
import java.util.Map;
import java.util.TreeMap;

import butterknife.BindView;
import butterknife.OnClick;
import butterknife.Unbinder;

public class SignFragment extends BaseFragment implements CommmonStringWithMethNameViewData {
    @BindView(R.id.tv_title)
    TextView tvTitle;
    @BindView(R.id.iv_title_right)
    ImageView ivTitleRight;
    @BindView(R.id.iv_qr)
    ImageView ivQr;
    private Map<Integer, String> dataMap;
    private Wallet wallet;

    @Override
    protected int getLayoutId() {
        return R.layout.fragment_sign;
    }

    @Override
    protected void setExtraData(Bundle data) {
        super.setExtraData(data);
        wallet = data.getParcelable("wallet");
        if (dataMap == null) {
            dataMap = new TreeMap<>();
        }
        JsonObject jsonObject = new JsonParser().parse(data.getString("result")).getAsJsonObject();
        getData(jsonObject);
    }


    @Override
    protected void initView(View view) {
        tvTitle.setText(getString(R.string.waitingsign));
        ivTitleRight.setImageResource(R.mipmap.top_share);
        registReceiver();
    }

    @OnClick({R.id.iv_title_right})
    public void onViewClicked(View view) {
        switch (view.getId()) {
            case R.id.iv_title_right:
                //截图分享
                ShareUtil.fxPic(getBaseActivity(), mRootView);
                break;
        }
    }

    @Override
    protected void requstPermissionOk() {
        new ScanQRcodeUtil().scanQRcode(this);
    }

    @Override
    public void onActivityResult(int requestCode, int resultCode, Intent data) {
        super.onActivityResult(requestCode, resultCode, data);
        //处理扫描结果（在界面上显示）
        if (resultCode == RESULT_OK && requestCode == ScanQRcodeUtil.SCAN_QR_REQUEST_CODE && data != null) {
            String result = data.getStringExtra("result");//&& matcherUtil.isMatcherAddr(result)
            if (!TextUtils.isEmpty(result) /*&& matcherUtil.isMatcherAddr(result)*/) {
                Log.i("::", result);
                try {
                    JsonObject jsonObject = new JsonParser().parse(result).getAsJsonObject();
                    int type = jsonObject.get("type").getAsInt();
                    if (type == Constant.SIGN) {
                        String attribute = getData(jsonObject);
                        if (!TextUtils.isEmpty(attribute)) {
                            //new PwdPresenter().signTransaction(wallet.getWalletId(), "ELA", attribute, "aa111111", this);
                            Intent intent = new Intent(getBaseActivity(), PwdActivity.class);
                            intent.putExtra("wallet", wallet);
                            intent.putExtra("chainId", "ELA");
                            intent.putExtra("attributes", attribute);
                            intent.putExtra("type", 1);
                            startActivity(intent);
                        }
                    }

                } catch (Exception e) {
                    showToast(e.getMessage());
                }

            }
        }

    }

    private String getData(JsonObject jsonObject) {

        try {
            String mydata = jsonObject.get("data").getAsString();
            int max = jsonObject.get("max").getAsInt();
            int current = jsonObject.get("current").getAsInt();
            dataMap.put(current, mydata);
            String msg = String.format(getContext().getString(R.string.scanprocess), dataMap.size() + "/" + max);
            showToast(msg);
            if (dataMap.size() == max) {
                StringBuilder signData = new StringBuilder();
                for (String s : dataMap.values()) {
                    signData.append(s);
                }
                return signData.toString();
            }
            requstManifestPermission(getString(R.string.needpermission));
        } catch (Exception e) {
            showToast(getString(R.string.error_30000));
        }
        return null;
    }

    @Override
    public void onGetCommonData(String methodname, String data) {

    }

    @Subscribe(threadMode = ThreadMode.MAIN)
    public void Event(BusEvent result) {
        int integer = result.getCode();
        if (integer == RxEnum.SIGNSUCCESS.ordinal()) {
            String signData = (String) result.getObj();
            showToast("签名成功");

        }
    }
}
