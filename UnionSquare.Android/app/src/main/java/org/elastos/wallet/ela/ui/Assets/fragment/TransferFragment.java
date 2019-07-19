package org.elastos.wallet.ela.ui.Assets.fragment;

import android.content.Intent;
import android.os.Bundle;
import android.text.InputFilter;
import android.text.TextUtils;
import android.view.View;
import android.widget.EditText;
import android.widget.ImageView;
import android.widget.TextView;

import org.elastos.wallet.R;
import org.elastos.wallet.ela.ElaWallet.MyWallet;
import org.elastos.wallet.ela.base.BaseFragment;
import org.elastos.wallet.ela.bean.BusEvent;
import org.elastos.wallet.ela.db.table.Contact;
import org.elastos.wallet.ela.db.table.Wallet;
import org.elastos.wallet.ela.ui.Assets.activity.TransferActivity;
import org.elastos.wallet.ela.ui.Assets.bean.BalanceEntity;
import org.elastos.wallet.ela.ui.Assets.fragment.transfer.SignFragment;
import org.elastos.wallet.ela.ui.Assets.fragment.transfer.ToSignFragment;
import org.elastos.wallet.ela.ui.Assets.presenter.CommonGetBalancePresenter;
import org.elastos.wallet.ela.ui.Assets.presenter.TransferPresenter;
import org.elastos.wallet.ela.ui.Assets.viewdata.CommonBalanceViewData;
import org.elastos.wallet.ela.ui.common.viewdata.CommmonBooleanViewData;
import org.elastos.wallet.ela.ui.common.viewdata.CommmonStringViewData;
import org.elastos.wallet.ela.utils.Arith;
import org.elastos.wallet.ela.utils.ClipboardUtil;
import org.elastos.wallet.ela.utils.Constant;
import org.elastos.wallet.ela.utils.DialogUtil;
import org.elastos.wallet.ela.utils.MatcherUtil;
import org.elastos.wallet.ela.utils.NumberiUtil;
import org.elastos.wallet.ela.utils.RxEnum;
import org.elastos.wallet.ela.utils.ScanQRcodeUtil;
import org.elastos.wallet.ela.utils.listener.WarmPromptListener;
import org.greenrobot.eventbus.EventBus;
import org.greenrobot.eventbus.Subscribe;
import org.greenrobot.eventbus.ThreadMode;

import butterknife.BindView;
import butterknife.OnClick;
import butterknife.Unbinder;

public class TransferFragment extends BaseFragment implements CommonBalanceViewData, CommmonBooleanViewData, CommmonStringViewData {
    @BindView(R.id.tv_title)
    TextView tvTitle;
    @BindView(R.id.iv_title_right)
    ImageView ivTitleRight;
    @BindView(R.id.et_payeeaddr)
    EditText etPayeeaddr;
    @BindView(R.id.iv_paste)
    ImageView ivPaste;
    @BindView(R.id.iv_contact)
    ImageView ivContact;
    @BindView(R.id.et_balance)
    EditText etBalance;
    @BindView(R.id.tv_max)
    TextView tvMax;
    @BindView(R.id.et_remark)
    EditText etRemark;
    Unbinder unbinder;
    /*@BindView(R.id.st_utxo)
    SuperTextView stUtxo;*/
    Unbinder unbinder1;
    private String chainId;
    private Wallet wallet;
    private String maxBalance = "0";
    private TransferPresenter presenter;
    private String address;
    private String amount;
    public boolean Checked = true;

    @Override
    protected int getLayoutId() {
        return R.layout.fragment_transfer;
    }

    @Override
    protected void setExtraData(Bundle data) {
        chainId = data.getString("ChainId", "ELA");
        wallet = data.getParcelable("wallet");
        new CommonGetBalancePresenter().getBalance(wallet.getWalletId(), chainId, 2, this);
    }

    @Override
    protected void initView(View view) {
        etBalance.setFilters(new InputFilter[]{MatcherUtil.filter(4)});
        tvTitle.setText(getString(R.string.transfer));
        ivTitleRight.setVisibility(View.VISIBLE);
        ivTitleRight.setImageResource(R.mipmap.setting_adding_scan);
        EventBus.getDefault().register(this);
        presenter = new TransferPresenter();
       /* stUtxo.setOnSuperTextViewClickListener(new SuperTextView.OnSuperTextViewClickListener() {
            @Override
            public void onClickListener(SuperTextView superTextView) {
                superTextView.setSwitchIsChecked(!superTextView.getSwitchIsChecked());
            }
        }).setSwitchCheckedChangeListener(new SuperTextView.OnSwitchCheckedChangeListener() {
            @Override
            public void onCheckedChanged(CompoundButton buttonView, boolean isChecked) {
                Checked = isChecked;
            }
        });*/
    }

    @OnClick({R.id.iv_paste, R.id.iv_contact, R.id.tv_max, R.id.tv_next, R.id.iv_title_right})
    public void onViewClicked(View view) {
        switch (view.getId()) {
            case R.id.iv_paste:
                etPayeeaddr.setText(ClipboardUtil.paste(getBaseActivity()));
                break;
            case R.id.iv_contact:
                start(new ChooseContactFragment());
                break;
            case R.id.tv_max:
                //  etBalance.setText(maxBalance);
                break;
            case R.id.tv_next:
                startTransfer();

                break;
            case R.id.iv_title_right:
                requstManifestPermission(getString(R.string.needpermission));
                break;
        }
    }

    private void startTransfer() {
        address = etPayeeaddr.getText().toString().trim();
        if (TextUtils.isEmpty(address)) {
            showToastMessage(getString(R.string.payeeaddrnotnull));
            return;
        }
        amount = etBalance.getText().toString().trim();
        if (TextUtils.isEmpty(amount)) {
            showToastMessage(getString(R.string.transferamountnotnull));
            return;
        }
      /*  if (Arith.mul(amount, MyWallet.RATE_S).add(new BigDecimal(MyWallet.feePerKb + ""))
                .compareTo(new BigDecimal(maxBalance)) > 0) {
            showToastMessage(getString(R.string.lack_of_balance));
            return;
        }*/
        presenter.isAddressValid(wallet.getWalletId(), address, this);
    }

    @Subscribe(threadMode = ThreadMode.MAIN)
    public void Event(BusEvent result) {
        int integer = result.getCode();
        if (integer == RxEnum.CHOOSECONTACT.ordinal()) {
            Contact contact = (Contact) result.getObj();
            etPayeeaddr.setText(contact.getWalletAddr());

        }
        if (integer == RxEnum.TRANSFERSUCESS.ordinal()) {
            new DialogUtil().showTransferSucess(getBaseActivity(), new WarmPromptListener() {
                @Override
                public void affireBtnClick(View view) {
                    popBackFragment();
                }
            });

        }
        if (integer == RxEnum.TOSIGN.ordinal()) {
            //生成待签名交易
            String attributes = (String) result.getObj();
            Bundle bundle = new Bundle();
            bundle.putString("attributes", attributes);
            bundle.putParcelable("wallet", wallet);
            start(SignFragment.class, bundle);

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
                etPayeeaddr.setText(result);
            }
        }

    }


    @Override
    public void onBalance(BalanceEntity data) {
        //请输入金额（可用：0 ELA）
        maxBalance = data.getBalance();
        // String balance = String.format(getString(R.string.inputbalance), NumberiUtil.maxNumberFormat((Double.parseDouble(maxBalance) / MyWallet.RATE) + "", 12) + " " + data.getChainId());
        String balance = String.format(getString(R.string.inputbalance), NumberiUtil.maxNumberFormat(Arith.div(maxBalance, MyWallet.RATE_S), 12) + " ELA");
        etBalance.setHint(balance);
    }

    @Override
    public void onGetCommonData(boolean data) {
        //这里是判断地址是否合法
        if (!data) {
            showToastMessage("不正确的钱包地址");
            return;
        }
        String remark = etRemark.getText().toString().trim();
        //presenter.createTransaction(wallet.getWalletId(), chainId, "", address, (long) (Double.parseDouble(amount) * MyWallet.RATE), "", remark, Checked, this);
        presenter.createTransaction(wallet.getWalletId(), chainId, "", address, Arith.mul(amount, MyWallet.RATE_S).toPlainString(), remark, Checked, this);
    }

    @Override
    public void onGetCommonData(String data) {
        //获得createTransaction

        Intent intent = new Intent(getActivity(), TransferActivity.class);
        intent.putExtra("amount", amount);
        intent.putExtra("toAddress", address);
        intent.putExtra("wallet", wallet);
        intent.putExtra("chainId", chainId);
        intent.putExtra("attributes", data);
        intent.putExtra("type", Constant.TRANFER);
        startActivity(intent);

    }


}
