package org.elastos.wallet.ela.ui.crvote.fragment;

import android.support.v4.widget.NestedScrollView;
import android.view.View;
import android.widget.CheckBox;
import android.widget.CompoundButton;
import android.widget.TextView;

import org.elastos.wallet.R;
import org.elastos.wallet.ela.base.BaseFragment;
import org.elastos.wallet.ela.utils.RxEnum;

import butterknife.BindView;
import butterknife.OnClick;

public class CRAgreementFragment extends BaseFragment {

    @BindView(R.id.scrollView)
    NestedScrollView scrollView;
    @BindView(R.id.tv_agree)
    TextView tvAgree;
    @BindView(R.id.tv_sure)
    TextView tvSure;
    @BindView(R.id.chechbox)
    CheckBox chechbox;
    //private boolean flag = false;

    @Override
    protected int getLayoutId() {
        return R.layout.fragment_cr_agreement;
    }

    @Override
    protected void initView(View view) {
        /*tvAgree.post(new Runnable() {
            @Override
            public void run() {
                if (scrollView.getMeasuredHeight() >= tvAgree.getMeasuredHeight() + ScreenUtil.dp2px(getContext(), 24)) {
                    flag = true;
                    tvSure.setVisibility(View.VISIBLE);
                    tvCancel.setVisibility(View.VISIBLE);
                    return;
                }
                scrollView.setOnScrollChangeListener(new NestedScrollView.OnScrollChangeListener() {
                    @Override
                    public void onScrollChange(NestedScrollView nestedScrollView, int i, int i1, int i2, int i3) {
                        if (!flag && scrollView.getMeasuredHeight() + i1 == tvAgree.getMeasuredHeight() + ScreenUtil.dp2px(getContext(), 24)) {
                            flag = true;
                            tvSure.setVisibility(View.VISIBLE);
                            tvCancel.setVisibility(View.VISIBLE);
                        }
                    }
                });
            }
        });*/
        tvSure.setAlpha(0.15f);
        tvSure.setEnabled(false);
        chechbox.setOnCheckedChangeListener(new CompoundButton.OnCheckedChangeListener() {
            @Override
            public void onCheckedChanged(CompoundButton buttonView, boolean isChecked) {
                if (isChecked) {
                    tvSure.setAlpha(1);
                    tvSure.setEnabled(true);
                } else {
                    tvSure.setAlpha(0.15f);
                    tvSure.setEnabled(false);
                }
            }
        });

    }

    @OnClick({R.id.tv_sure})
    public void onViewClicked(View view) {
        switch (view.getId()) {
            case R.id.tv_sure:
                post(RxEnum.AGREE.ordinal(), null, null);
                popBackFragment();
                break;

        }
    }


}
