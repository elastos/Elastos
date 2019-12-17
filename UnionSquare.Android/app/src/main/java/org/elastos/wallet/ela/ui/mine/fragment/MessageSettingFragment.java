package org.elastos.wallet.ela.ui.mine.fragment;

import android.view.View;
import android.widget.CompoundButton;
import android.widget.TextView;

import com.allen.library.SuperTextView;

import org.elastos.wallet.R;
import org.elastos.wallet.ela.base.BaseFragment;
import org.elastos.wallet.ela.utils.SPUtil;

import butterknife.BindView;

public class MessageSettingFragment extends BaseFragment {
    @BindView(R.id.tv_title)
    TextView tvTitle;
    @BindView(R.id.st_redpoint)
    SuperTextView stRedpoint;
    @BindView(R.id.st_sendmsg)
    SuperTextView stSendmsg;
    private SPUtil sp;


    @Override
    protected int getLayoutId() {
        return R.layout.fragment_messageset;
    }

    @Override
    protected void initView(View view) {
        tvTitle.setText(R.string.setting);
        sp = new SPUtil(getContext());
        stRedpoint.setSwitchIsChecked(sp.isOpenRedPoint());
        stSendmsg.setSwitchIsChecked(sp.isOpenSendMsg());
        stRedpoint.setSwitchCheckedChangeListener(new SuperTextView.OnSwitchCheckedChangeListener() {
            @Override
            public void onCheckedChanged(CompoundButton buttonView, boolean isChecked) {
                sp.setOpenRedPoint(isChecked);
            }
        });
        stSendmsg.setSwitchCheckedChangeListener(new SuperTextView.OnSwitchCheckedChangeListener() {
            @Override
            public void onCheckedChanged(CompoundButton buttonView, boolean isChecked) {
                sp.setOpenSendMsg(isChecked);
            }
        });
    }


}
