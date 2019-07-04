package org.elastos.wallet.ela.ui.Assets.fragment.mulsignwallet;

import android.content.Intent;
import android.os.Bundle;
import android.support.v7.widget.LinearLayoutManager;
import android.support.v7.widget.RecyclerView;
import android.text.TextUtils;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.CheckBox;
import android.widget.CompoundButton;
import android.widget.EditText;
import android.widget.ImageView;
import android.widget.LinearLayout;
import android.widget.TextView;

import org.elastos.wallet.R;
import org.elastos.wallet.ela.base.BaseFragment;
import org.elastos.wallet.ela.bean.BusEvent;
import org.elastos.wallet.ela.bean.CreateWalletBean;
import org.elastos.wallet.ela.ui.Assets.adapter.AddMulSignPublicKeyAdapter;
import org.elastos.wallet.ela.utils.ClearEditText;
import org.elastos.wallet.ela.utils.DialogUtil;
import org.elastos.wallet.ela.utils.Log;
import org.elastos.wallet.ela.utils.RxEnum;
import org.elastos.wallet.ela.utils.ScanQRcodeUtil;
import org.elastos.wallet.ela.utils.listener.WarmPromptListener;
import org.elastos.wallet.ela.utils.widget.TextConfigNumberPicker;
import org.greenrobot.eventbus.Subscribe;
import org.greenrobot.eventbus.ThreadMode;

import butterknife.BindView;
import butterknife.ButterKnife;
import butterknife.OnClick;
import butterknife.Unbinder;

public class CreateMulWalletFragment extends BaseFragment implements CompoundButton.OnCheckedChangeListener {

    @BindView(R.id.tv_title)
    TextView tvTitle;
    @BindView(R.id.ll_addmainkey)
    LinearLayout llAddmainkey;
    @BindView(R.id.et_walletname)
    ClearEditText etWalletname;
    @BindView(R.id.tv_signnum)
    TextView tvSignnum;
    @BindView(R.id.cb)
    CheckBox cb;
    @BindView(R.id.rv)
    RecyclerView rv;
    @BindView(R.id.iv_status)
    ImageView ivStatus;
    @BindView(R.id.tv_status)
    TextView tvStatus;
    @BindView(R.id.iv_add)
    ImageView ivAdd;
    Unbinder unbinder;
    private CreateWalletBean createWalletBean;
    private AddMulSignPublicKeyAdapter adapter;

    private int count = 2;

    @Override
    protected int getLayoutId() {
        return R.layout.fragment_create_mul;
    }

    @Override
    protected void initView(View view) {
        tvTitle.setText(getString(R.string.createmulwalllet));
        cb.setOnCheckedChangeListener(this);
        registReceiver();
        setRecycleView();

        //使得iv充满且不能滑动
        rv.setNestedScrollingEnabled(false);
        rv.setFocusableInTouchMode(false);
    }

    private void setRecycleView() {
        if (adapter == null) {
            adapter = new AddMulSignPublicKeyAdapter(this);
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
                Log.d("?????", adapter.getMap().toString());
                break;
            case R.id.iv_add:
                count++;
                setRecycleView();
                if ((cb.isChecked() && count == 6) || (!cb.isChecked() && count == 5)) {
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
        int integer = result.getCode();
        if (integer == RxEnum.CREATEPRIVATEKEY.ordinal()) {
            //添加主私钥额回调
            createWalletBean = (CreateWalletBean) result.getObj();
        }
        if (integer == RxEnum.IMPORTRIVATEKEY.ordinal()) {
            //导入助记词回调
            createWalletBean = (CreateWalletBean) result.getObj();
        }
        if (integer == RxEnum.SELECTRIVATEKEY.ordinal()) {
            //选择已有钱包回调
            setMainPrivaKeyStatus();
            createWalletBean = (CreateWalletBean) result.getObj();

        }

        //  Log.i("?????", createWalletBean.toString());

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
            String result = data.getStringExtra("result");//&& matcherUtil.isMatcherAddr(result)
            if (!TextUtils.isEmpty(result) /*&& matcherUtil.isMatcherAddr(result)*/) {
                rvEditText.setText(result);
            }
        }

    }
}
