package org.elastos.wallet.ela.ui.did.fragment;

import android.os.Bundle;
import android.text.TextUtils;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.EditText;
import android.widget.RelativeLayout;
import android.widget.TextView;

import org.elastos.wallet.R;
import org.elastos.wallet.ela.base.BaseFragment;
import org.elastos.wallet.ela.db.RealmUtil;
import org.elastos.wallet.ela.db.table.Wallet;
import org.elastos.wallet.ela.rxjavahelp.BaseEntity;
import org.elastos.wallet.ela.rxjavahelp.NewBaseViewData;
import org.elastos.wallet.ela.ui.mine.presenter.AboutPresenter;
import org.elastos.wallet.ela.utils.DialogUtil;
import org.elastos.wallet.ela.utils.listener.WarmPromptListener;
import org.elastos.wallet.ela.utils.widget.TextConfigDataPicker;
import org.elastos.wallet.ela.utils.widget.TextConfigNumberPicker;

import java.util.List;

import butterknife.BindView;
import butterknife.ButterKnife;
import butterknife.OnClick;
import butterknife.Unbinder;

public class AddDIDFragment extends BaseFragment implements NewBaseViewData {

    @BindView(R.id.tv_title)
    TextView tvTitle;

    AboutPresenter aboutPresenter;
    @BindView(R.id.et_didname)
    EditText etDidname;
    @BindView(R.id.tv_walletname)
    TextView tvWalletname;
    @BindView(R.id.rl_selectwallet)
    RelativeLayout rlSelectwallet;
    @BindView(R.id.et_didpk)
    TextView etDidpk;
    @BindView(R.id.et_did)
    TextView etDid;
    @BindView(R.id.tv_date)
    TextView tvDate;
    @BindView(R.id.rl_outdate)
    RelativeLayout rlOutdate;
    Unbinder unbinder;
    String[] walletNames;
    List<Wallet> wallets;

    @Override
    protected int getLayoutId() {
        return R.layout.fragment_add_did;
    }

    @Override
    protected void initInjector() {

    }

    @Override
    protected void initView(View view) {
        tvTitle.setText(getString(R.string.adddid));
        wallets = new RealmUtil().queryUserAllWallet();
        walletNames = new String[wallets.size()];
        for (int i = 0; i < wallets.size(); i++) {
            walletNames[i] = wallets.get(i).getWalletName();
        }
        aboutPresenter = new AboutPresenter();

    }


    @OnClick({R.id.rl_selectwallet, R.id.rl_outdate})
    public void onViewClicked(View view) {
        switch (view.getId()) {
            case R.id.rl_selectwallet:


                new DialogUtil().showCommonSelect(getBaseActivity(), walletNames, new WarmPromptListener() {
                    @Override
                    public void affireBtnClick(View view) {
                        tvWalletname.setText(walletNames[((TextConfigNumberPicker) view).getValue()]);
                    }
                });
                break;
            case R.id.rl_outdate:
                new DialogUtil().showTime(getBaseActivity(), new WarmPromptListener() {
                    @Override
                    public void affireBtnClick(View view) {
                        String endDate = ((TextConfigDataPicker) view).getYear() + "/" + (((TextConfigDataPicker) view).getMonth() + 1)
                                + "/" + ((TextConfigDataPicker) view).getDayOfMonth();
                        // String begainDate = DateUtil.getCurrentData(DateUtil.FORMART2);
                        tvDate.setText(getString(R.string.validtime) + endDate);
                    }
                });
                break;


        }
    }

    @Override
    public boolean onBackPressedSupport() {
        String didName = etDidname.getText().toString().trim();
        if (!TextUtils.isEmpty(didName)) {
            showToast("22");
            return true;
        }
        return super.onBackPressedSupport();


    }

    @Override
    public void onGetData(String methodName, BaseEntity baseEntity, Object o) {

    }

    @Override
    public View onCreateView(LayoutInflater inflater, ViewGroup container, Bundle savedInstanceState) {
        // TODO: inflate a fragment view
        View rootView = super.onCreateView(inflater, container, savedInstanceState);
        unbinder = ButterKnife.bind(this, rootView);
        return rootView;
    }

    @Override
    public void onDestroyView() {
        super.onDestroyView();
        unbinder.unbind();
    }
}
