package org.elastos.wallet.ela.ui.committee.fragment;

import android.support.v7.widget.Toolbar;
import android.view.View;
import android.widget.TextView;

import org.elastos.wallet.R;
import org.elastos.wallet.ela.ElaWallet.MyWallet;
import org.elastos.wallet.ela.base.BaseFragment;
import org.elastos.wallet.ela.bean.BusEvent;
import org.elastos.wallet.ela.db.RealmUtil;
import org.elastos.wallet.ela.db.table.Wallet;
import org.elastos.wallet.ela.rxjavahelp.BaseEntity;
import org.elastos.wallet.ela.rxjavahelp.NewBaseViewData;
import org.elastos.wallet.ela.ui.crvote.presenter.CRSignUpPresenter;
import org.elastos.wallet.ela.ui.vote.bean.Area;
import org.elastos.wallet.ela.ui.vote.fragment.AreaCodeFragment;
import org.elastos.wallet.ela.utils.RxEnum;
import org.elastos.wallet.ela.utils.SPUtil;
import org.greenrobot.eventbus.EventBus;
import org.greenrobot.eventbus.Subscribe;
import org.greenrobot.eventbus.ThreadMode;

import butterknife.BindView;
import butterknife.OnClick;

/**
 * refresh cr infomation
 */
public class UpdateCtInformationFragment extends BaseFragment implements NewBaseViewData {

    @BindView(R.id.toolbar_title)
    TextView toolbarTitle;
    @BindView(R.id.toolbar)
    Toolbar toolbar;
    @BindView(R.id.area)
    TextView areaTv;

    @Override
    protected int getLayoutId() {
        return R.layout.fragment_update_ct_info;
    }

    @Override
    protected void initView(View view) {
        setToobar(toolbar, toolbarTitle, getContext().getString(R.string.edit));
        registReceiver();
    }

    @Subscribe(threadMode = ThreadMode.MAIN)
    public void Event(BusEvent result) {
        int integer = result.getCode();
        if (integer == RxEnum.AREA.ordinal()) {
            Area area = (Area) result.getObj();
            int Language = new SPUtil(getContext()).getLanguage();
            String name;
            if (Language == 0) {
                name = area.getZh();
            } else {
                name = area.getEn();
            }
            areaTv.setText(name);
        }

    }

    public void registReceiver() {
        if (!EventBus.getDefault().isRegistered(this))
            EventBus.getDefault().register(this);
    }

    @OnClick({R.id.confirm, R.id.area})
    public void onClick(View view) {
        switch (view.getId()) {
            case R.id.confirm:
                //TODO 待确认

                break;

            case R.id.area:
                start(AreaCodeFragment.class);
                break;
        }
    }

    @Override
    public void onGetData(String methodName, BaseEntity baseEntity, Object o) {

    }
}
