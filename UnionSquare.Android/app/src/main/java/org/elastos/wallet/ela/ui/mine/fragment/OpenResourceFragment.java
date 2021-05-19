package org.elastos.wallet.ela.ui.mine.fragment;

import android.text.SpannableString;
import android.text.style.LeadingMarginSpan;
import android.view.View;
import android.widget.TextView;

import com.blankj.utilcode.util.SizeUtils;

import org.elastos.wallet.R;
import org.elastos.wallet.ela.base.BaseFragment;

import butterknife.BindView;

public class OpenResourceFragment extends BaseFragment {
    @BindView(R.id.tv_detail)
    TextView tvDetail;

    @Override
    protected int getLayoutId() {
        return R.layout.fragment_openresource;
    }

    @Override
    protected void initView(View view) {
        SpannableString spannableString = new SpannableString(getString(R.string.openresorcedetaile));
        //0 第一行缩进像素 , SizeUtils.dp2px(15)非第一行缩进像素
        LeadingMarginSpan.Standard what = new LeadingMarginSpan.Standard(0, SizeUtils.dp2px(19));

        spannableString.setSpan(what, 0, spannableString.length(), SpannableString.SPAN_INCLUSIVE_INCLUSIVE);

        tvDetail.setText(spannableString);


    }

}
