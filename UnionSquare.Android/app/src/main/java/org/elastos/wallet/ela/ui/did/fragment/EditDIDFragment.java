package org.elastos.wallet.ela.ui.did.fragment;

import android.content.Intent;
import android.os.Bundle;
import android.text.TextUtils;
import android.view.View;
import android.widget.EditText;
import android.widget.RelativeLayout;
import android.widget.TextView;

import org.elastos.did.DIDDocument;
import org.elastos.wallet.R;
import org.elastos.wallet.ela.ElaWallet.MyWallet;
import org.elastos.wallet.ela.base.BaseFragment;
import org.elastos.wallet.ela.bean.BusEvent;
import org.elastos.wallet.ela.db.table.Wallet;
import org.elastos.wallet.ela.rxjavahelp.BaseEntity;
import org.elastos.wallet.ela.rxjavahelp.NewBaseViewData;
import org.elastos.wallet.ela.ui.common.bean.CommmonLongEntity;
import org.elastos.wallet.ela.ui.crvote.presenter.CRSignUpPresenter;
import org.elastos.wallet.ela.ui.vote.activity.VoteTransferActivity;
import org.elastos.wallet.ela.utils.Constant;
import org.elastos.wallet.ela.utils.DateUtil;
import org.elastos.wallet.ela.utils.DialogUtil;
import org.elastos.wallet.ela.utils.RxEnum;
import org.elastos.wallet.ela.utils.listener.WarmPromptListener;
import org.elastos.wallet.ela.utils.widget.TextConfigDataPicker;
import org.greenrobot.eventbus.Subscribe;
import org.greenrobot.eventbus.ThreadMode;

import java.util.Calendar;
import java.util.Date;

import butterknife.BindView;
import butterknife.OnClick;

public class EditDIDFragment extends BaseFragment implements NewBaseViewData {


    @BindView(R.id.tv_title)
    TextView tvTitle;

    Wallet wallet;
    @BindView(R.id.et_didname)
    EditText etDidname;
    @BindView(R.id.tv_didpk)
    TextView tvDidpk;
    @BindView(R.id.tv_did)
    TextView tvDid;
    @BindView(R.id.tv_date)
    TextView tvDate;
    @BindView(R.id.rl_outdate)
    RelativeLayout rlOutdate;
    @BindView(R.id.tv_updata)
    TextView tvUpdata;
    private Date didEndDate;
    private String didName;

    @Override
    protected int getLayoutId() {
        return R.layout.fragment_edit_did;
    }

    @Override
    protected void setExtraData(Bundle data) {
        wallet = data.getParcelable("wallet");
        putData(getMyDID().getDIDDocument());

    }

    private void putData(DIDDocument doc) {
        didName = getMyDID().getName(doc);
        didEndDate = getMyDID().getExpires(doc);
        etDidname.setText(didName);
        tvDid.setText(getMyDID().getDidString());
        tvDidpk.setText(getMyDID().getDidPublicKey(doc));
        tvDate.setText(getString(R.string.validtime) + DateUtil.timeNYR(didEndDate, getContext()));
        registReceiver();
    }

    @Override
    protected void initView(View view) {
        tvTitle.setText(getString(R.string.editdid));
    }

    @OnClick({R.id.tv_updata, R.id.rl_outdate})
    public void onViewClicked(View view) {
        Bundle bundle;
        switch (view.getId()) {
            case R.id.tv_updata:
                //更新
                didName = etDidname.getText().toString().trim();
                if (TextUtils.isEmpty(didName)) {
                    showToast(getString(R.string.plzinputname));
                    break;
                }

                if (didEndDate == null) {
                    showToast(getString(R.string.plzselctoutdate));
                    break;
                }
                new CRSignUpPresenter().getFee(wallet.getWalletId(), MyWallet.IDChain, "", "8USqenwzA5bSAvj1mG4SGTABykE9n5RzJQ", "0", this);

                break;
            case R.id.rl_outdate:
                Calendar calendar = Calendar.getInstance();
                calendar.add(Calendar.DAY_OF_MONTH, 1);
                calendar.set(Calendar.HOUR_OF_DAY, 0);
                long minData = calendar.getTimeInMillis();
                calendar.add(Calendar.YEAR, 5);
                new DialogUtil().showTime(getBaseActivity(), minData, calendar.getTimeInMillis(), new WarmPromptListener() {
                    @Override
                    public void affireBtnClick(View view) {
                        String date = ((TextConfigDataPicker) view).getYear() + "-" + (((TextConfigDataPicker) view).getMonth() + 1)
                                + "-" + ((TextConfigDataPicker) view).getDayOfMonth();
                        didEndDate = DateUtil.parseToDate(date);

                        tvDate.setText(getString(R.string.validtime) + DateUtil.timeNYR(didEndDate, getContext()));
                    }
                });
                break;


        }
    }

    @Override
    public void onGetData(String methodName, BaseEntity baseEntity, Object o) {
        switch (methodName) {
            case "getFee":
                Intent intent = new Intent(getActivity(), VoteTransferActivity.class);
                intent.putExtra("didName", didName);
                intent.putExtra("didEndDate", didEndDate);
                intent.putExtra("wallet", wallet);
                intent.putExtra("chainId", MyWallet.IDChain);
                intent.putExtra("fee", ((CommmonLongEntity) baseEntity).getData());
                intent.putExtra("type", Constant.DIDSIGNUP);
                intent.putExtra("transType", 10);
                startActivity(intent);
                break;
        }

    }

    @Subscribe(threadMode = ThreadMode.MAIN)
    public void Event(BusEvent result) {
        int integer = result.getCode();
        if (integer == RxEnum.TRANSFERSUCESS.ordinal()) {
            new DialogUtil().showTransferSucess(getBaseActivity(), new WarmPromptListener() {
                @Override
                public void affireBtnClick(View view) {
                    toDIDDetailFragment();
                }
            });
        }
    }

}
