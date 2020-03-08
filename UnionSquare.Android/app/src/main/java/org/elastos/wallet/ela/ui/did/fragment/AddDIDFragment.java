package org.elastos.wallet.ela.ui.did.fragment;

import android.os.Bundle;
import android.text.TextUtils;
import android.view.View;
import android.widget.EditText;
import android.widget.TextView;

import org.elastos.wallet.R;
import org.elastos.wallet.ela.base.BaseFragment;
import org.elastos.wallet.ela.utils.DateUtil;
import org.elastos.wallet.ela.utils.DialogUtil;
import org.elastos.wallet.ela.utils.listener.WarmPromptListener;
import org.elastos.wallet.ela.utils.widget.TextConfigDataPicker;

import java.util.Calendar;
import java.util.Date;

import butterknife.BindView;
import butterknife.OnClick;

public class AddDIDFragment extends BaseFragment {

    @BindView(R.id.tv_title)
    TextView tvTitle;

    @BindView(R.id.et_didname)
    EditText etDidname;
    @BindView(R.id.tv_date)
    TextView tvDate;
    private Date didEndDate;

    @Override
    protected int getLayoutId() {
        return R.layout.fragment_add_did;
    }


    @Override
    protected void initView(View view) {

        tvTitle.setText(R.string.createdid);


    }


    @OnClick({R.id.rl_outdate, R.id.tv_next})
    public void onViewClicked(View view) {
        switch (view.getId()) {
            case R.id.tv_next:
                String didName = etDidname.getText().toString().trim();
                if (TextUtils.isEmpty(didName)) {
                    showToast(getString(R.string.plzinputname));
                    break;
                }

                if (didEndDate == null) {
                    showToast(getString(R.string.plzselctoutdate));
                    break;
                }
                Bundle bundle = getArguments();
                bundle.putString("didName", didName);
                bundle.putSerializable("didEndDate", didEndDate);
                start(PersonalInfoFragment.class, bundle);
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


}
