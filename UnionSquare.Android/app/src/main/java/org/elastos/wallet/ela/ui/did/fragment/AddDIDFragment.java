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
    private long minData;
    private long endDate;

    @Override
    protected int getLayoutId() {
        return R.layout.fragment_add_did;
    }


    @Override
    protected void initView(View view) {

        tvTitle.setText(R.string.createdid);
        Calendar calendar = Calendar.getInstance();
        calendar.add(Calendar.DAY_OF_MONTH, 1);
        calendar.set(Calendar.HOUR_OF_DAY, 0);
        minData = calendar.getTimeInMillis();
        calendar.add(Calendar.YEAR, 5);
        calendar.add(Calendar.DAY_OF_MONTH, -1);
        didEndDate = calendar.getTime();
        endDate = calendar.getTimeInMillis();
        tvDate.setText(getString(R.string.validtime) + DateUtil.timeNYR(didEndDate, getContext()));
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
                start(AddPersonalInfoFragment.class, bundle);
                break;

            case R.id.rl_outdate:
                new DialogUtil().showTime(getBaseActivity(), getString(R.string.plzselctoutdate), minData, endDate, new WarmPromptListener() {
                    @Override
                    public void affireBtnClick(View view) {
                        String date = ((TextConfigDataPicker) view).getYear() + "." + (((TextConfigDataPicker) view).getMonth() + 1)
                                + "." + ((TextConfigDataPicker) view).getDayOfMonth();
                        didEndDate = DateUtil.parseToDate(date);

                        tvDate.setText(getString(R.string.validtime) + DateUtil.timeNYR(didEndDate, getContext()));
                    }
                });
                break;


        }
    }


}
