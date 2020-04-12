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

import android.content.Intent;
import android.os.Bundle;
import android.support.v7.widget.LinearLayoutManager;
import android.support.v7.widget.RecyclerView;
import android.text.TextUtils;
import android.view.View;
import android.widget.CheckBox;
import android.widget.CompoundButton;
import android.widget.EditText;
import android.widget.ImageView;
import android.widget.LinearLayout;
import android.widget.TextView;

import com.alibaba.fastjson.JSON;
import com.alibaba.fastjson.JSONObject;
import com.google.gson.JsonArray;
import com.google.gson.JsonPrimitive;

import org.elastos.wallet.R;
import org.elastos.wallet.ela.ElaWallet.MyWallet;
import org.elastos.wallet.ela.base.BaseFragment;
import org.elastos.wallet.ela.bean.BusEvent;
import org.elastos.wallet.ela.bean.CreateWalletBean;
import org.elastos.wallet.ela.db.RealmUtil;
import org.elastos.wallet.ela.db.listener.RealmTransactionAbs;
import org.elastos.wallet.ela.db.table.Wallet;
import org.elastos.wallet.ela.ui.Assets.adapter.AddMulSignPublicKeyAdapter;
import org.elastos.wallet.ela.ui.Assets.presenter.CommonCreateSubWalletPresenter;
import org.elastos.wallet.ela.ui.Assets.presenter.mulwallet.CreatMulWalletPresenter;
import org.elastos.wallet.ela.ui.Assets.viewdata.CommonCreateSubWalletViewData;
import org.elastos.wallet.ela.ui.common.viewdata.CommmonStringWithMethNameViewData;
import org.elastos.wallet.ela.utils.AppUtlis;
import org.elastos.wallet.ela.utils.ClearEditText;
import org.elastos.wallet.ela.utils.Constant;
import org.elastos.wallet.ela.utils.DialogUtil;
import org.elastos.wallet.ela.utils.QrBean;
import org.elastos.wallet.ela.utils.RxEnum;
import org.elastos.wallet.ela.utils.ScanQRcodeUtil;
import org.elastos.wallet.ela.utils.listener.WarmPromptListener;
import org.elastos.wallet.ela.utils.widget.TextConfigNumberPicker;
import org.greenrobot.eventbus.Subscribe;
import org.greenrobot.eventbus.ThreadMode;

import java.util.Map;

import butterknife.BindView;
import butterknife.OnClick;

public class CreateMulWalletFragment extends BaseFragment implements CompoundButton.OnCheckedChangeListener, CommmonStringWithMethNameViewData, CommonCreateSubWalletViewData {

    @BindView(R.id.tv_title)
    TextView tvTitle;
    @BindView(R.id.ll_addmainkey)
    LinearLayout llAddmainkey;
    @BindView(R.id.et_walletname)
    ClearEditText etWalletname;
    @BindView(R.id.tv_signnum)
    TextView tvSignnum;
    @BindView(R.id.cb_readonly)
    CheckBox cbReadonly;
    @BindView(R.id.cb_single)
    CheckBox cbSingle;
    @BindView(R.id.rv)
    RecyclerView rv;
    @BindView(R.id.iv_status)
    ImageView ivStatus;
    @BindView(R.id.tv_status)
    TextView tvStatus;
    @BindView(R.id.iv_add)
    ImageView ivAdd;
    private CreateWalletBean createWalletBean;
    private AddMulSignPublicKeyAdapter adapter;
    CreatMulWalletPresenter creatMulWalletPresenter;
    private int count = 2;
    int integer = -1;
    private String masterWalletID;
    private String name;
    private String publicKey;

    @Override
    protected int getLayoutId() {
        return R.layout.fragment_create_mul;
    }

    @Override
    protected void setExtraData(Bundle data) {
        super.setExtraData(data);
        String result = data.getString("result");
        try {
            JSONObject jsonObject = JSON.parseObject(result);
            publicKey = jsonObject.getString("data");
        } catch (Exception e) {
            publicKey = result;
        }

    }

    @Override
    protected void initView(View view) {
        tvTitle.setText(getString(R.string.createmulwalllet));
        cbReadonly.setOnCheckedChangeListener(this);
        registReceiver();
        setRecycleView();
        //使得iv充满且不能滑动
        rv.setNestedScrollingEnabled(false);
        rv.setFocusableInTouchMode(false);
        creatMulWalletPresenter = new CreatMulWalletPresenter();
    }

    private void setRecycleView() {
        if (adapter == null) {
            adapter = new AddMulSignPublicKeyAdapter(this, publicKey);
            adapter.setCount(count);
            rv.setLayoutManager(new LinearLayoutManager(getContext(), LinearLayoutManager.VERTICAL, false));
            rv.setAdapter(adapter);
        } else {
            adapter.setCount(count);
            adapter.notifyItemInserted(count);
            //adapter.notifyItemRangeChanged(0,count);
        }

    }

    @OnClick({R.id.ll_addmainkey, R.id.tv_create, R.id.iv_add, R.id.iv_chosenum})
    public void onViewClick(View view) {
        switch (view.getId()) {
            case R.id.ll_addmainkey:
                start(MainPrivateKeyFragment.class);
                break;
            case R.id.tv_create:
                createMulWallet(adapter.getMap());
                break;
            case R.id.iv_add:
                count++;
                setRecycleView();
                if ((cbReadonly.isChecked() && count == 6) || (!cbReadonly.isChecked() && count == 5)) {
                    ivAdd.setVisibility(View.GONE);
                }
                break;
            case R.id.iv_chosenum:
                new DialogUtil().showSelectNum(getBaseActivity(), new WarmPromptListener() {
                    @Override
                    public void affireBtnClick(View view) {
                        tvSignnum.setText(((TextConfigNumberPicker) view).getValue() + "");
                    }
                });
                break;
        }
    }

    private void createMulWallet(Map<Integer, String> publicKey) {
        name = etWalletname.getText().toString().trim();

        if (TextUtils.isEmpty(name)) {
            showToast(getString(R.string.inputWalletName));
            return;
        }
        int needItem = Integer.parseInt(tvSignnum.getText().toString().trim());
        JsonArray jsonArray = new JsonArray();
        for (String value : publicKey.values()) {
            JsonPrimitive valueJson = new JsonPrimitive(value);
            if (jsonArray.contains(valueJson)) {
                showToast(getString(R.string.pksame));
                return;
            } else {
                jsonArray.add(valueJson);
            }
        }
        masterWalletID = AppUtlis.getStringRandom(8);
        if (cbReadonly.isChecked()) {
            //只读
            if (publicKey.size() < needItem) {
                showToast(getString(R.string.publickeyneedmore));
                return;
            }
            creatMulWalletPresenter.createMultiSignMasterWalletReadOnly(masterWalletID, jsonArray.toString()
                    , needItem, cbSingle.isChecked(), false, 0, this);

        } else {
            if (integer == -1) {
                showToast(getString(R.string.mainprivatekeynotnull));
                return;
            }
            if (publicKey.size() < needItem - 1) {
                showToast(getString(R.string.publickeyneedmore));
                return;
            }
            if (publicKey.size() > 5) {
                showToast(getString(R.string.publickeytoomany));
                return;
            }
            if (integer == RxEnum.CREATEPRIVATEKEY.ordinal() || integer == RxEnum.IMPORTRIVATEKEY.ordinal()) {
                //非只读添加根私钥额回调 导入助记词回调
                creatMulWalletPresenter.createMultiSignMasterWalletByMnemonic(masterWalletID, createWalletBean.getMnemonic(),
                        createWalletBean.getPhrasePassword(), createWalletBean.getPayPassword(), jsonArray.toString()
                        , needItem, cbSingle.isChecked(), false, 0, this);

            } else if (integer == RxEnum.SELECTRIVATEKEY.ordinal()) {
                //选择已有钱包回调
                creatMulWalletPresenter.createMultiSignMasterWalletByPrivKey(masterWalletID, createWalletBean.getPrivateKey()
                        , createWalletBean.getPayPassword(), jsonArray.toString(), needItem, cbSingle.isChecked(), false, 0, this);
            }

        }
    }


    @Override
    public void onCheckedChanged(CompoundButton buttonView, boolean isChecked) {
        if (isChecked) {
            llAddmainkey.setEnabled(false);
            llAddmainkey.setAlpha(0.5f);
            if (count < 6) {
                ivAdd.setVisibility(View.VISIBLE);
            }

        } else {
            llAddmainkey.setEnabled(true);
            llAddmainkey.setAlpha(1);
        }
    }

    @Subscribe(threadMode = ThreadMode.MAIN)
    public void Event(BusEvent result) {
        int code = result.getCode();
        if (code == RxEnum.CREATEPRIVATEKEY.ordinal() //添加私钥额回调
                || code == RxEnum.IMPORTRIVATEKEY.ordinal()//导入助记词回调 //选择已有钱包回调
                || code == RxEnum.SELECTRIVATEKEY.ordinal()//选择已有钱包回调
        ) {
            integer = code;
            setMainPrivaKeyStatus();
            createWalletBean = (CreateWalletBean) result.getObj();
        }

    }

    private void setMainPrivaKeyStatus() {
        ivStatus.setImageResource(R.mipmap.asset_adding_select);
        tvStatus.setText(getString(R.string.editmainprivatekey));
    }

    private EditText rvEditText;

    public void requstManifestPermission(String requstStr, EditText editText) {
        super.requstManifestPermission(requstStr);
        this.rvEditText = editText;
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
            String result = data.getStringExtra("result");
            if (!TextUtils.isEmpty(result)) {
                try {
                    QrBean qrBean = JSON.parseObject(result, QrBean.class);
                    int type = qrBean.getExtra().getType();
                    if (type == Constant.CREATEMUL) {
                        rvEditText.setText(qrBean.getData());
                    } else {
                        rvEditText.setText(result);
                    }
                } catch (Exception e) {
                    rvEditText.setText(result);
                }

            }
        }

    }

    @Override
    public void onGetCommonData(String methodname, String data) {

        //无论何种createMultiSignMasterWallet方式创建的多签钱包都到这里
        if (data != null) {
            new CommonCreateSubWalletPresenter().createSubWallet(masterWalletID, MyWallet.ELA, this, null);
        }

    }

    @Override
    public void onCreateSubWallet(String data, Object o) {
        if (data != null) {
            //创建Mainchain子钱包
            RealmUtil realmUtil = new RealmUtil();
            Wallet masterWallet = realmUtil.updateWalletDetial(name, masterWalletID, data);
            realmUtil.updateSubWalletDetial(masterWalletID, data, new RealmTransactionAbs() {
                @Override
                public void onSuccess() {
                    realmUtil.updateWalletDefault(masterWalletID, new RealmTransactionAbs() {
                        @Override
                        public void onSuccess() {
                            post(RxEnum.ONE.ordinal(), null, masterWallet);
                            toMainFragment();
                        }
                    });
                }
            });


        }
    }
}
