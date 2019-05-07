package org.elastos.wallet.ela.ui.vote.activity;

import android.content.Intent;
import android.graphics.Color;
import android.graphics.drawable.ColorDrawable;
import android.text.TextUtils;
import android.view.View;
import android.view.ViewGroup;
import android.widget.TextView;

import org.elastos.wallet.ela.ElaWallet.MyWallet;
import org.elastos.wallet.R;
import org.elastos.wallet.ela.base.BaseActivity;
import org.elastos.wallet.ela.utils.AndroidWorkaround;
import org.elastos.wallet.ela.utils.Arith;
import org.elastos.wallet.ela.utils.ClearEditText;
import org.elastos.wallet.ela.utils.RxEnum;

import butterknife.BindView;
import butterknife.OnClick;

public class VoteActivity extends BaseActivity {
    @BindView(R.id.et_pwd)
    ClearEditText etPwd;
    String fee;
    @BindView(R.id.tv_max)
    TextView tv_max;
    @BindView(R.id.tv_maxvote)
    TextView tv_maxvote;

    @Override
    protected int getLayoutId() {
        if (AndroidWorkaround.checkDeviceHasNavigationBar(this)) {
            AndroidWorkaround.assistActivity(findViewById(android.R.id.content));
        }
        return R.layout.activity_vote;
    }

    @Override
    protected void initView() {
        getWindow().setBackgroundDrawable(new ColorDrawable(Color.TRANSPARENT));
        //一定要在setContentView之后调用，否则无效
        getWindow().setLayout(ViewGroup.LayoutParams.MATCH_PARENT, ViewGroup.LayoutParams.MATCH_PARENT);

    }

    @Override
    protected void setExtraData(Intent data) {
        fee = data.getStringExtra("fee");
        //Double num = Double.parseDouble(fee) / MyWallet.RATE;
        int num = Arith.div(fee,MyWallet.RATE_S).intValue();
        tv_max.setOnClickListener(v -> etPwd.setText(num + ""));
        tv_maxvote.setText(getString(R.string.maximum_voting_rights) + "  " + num);
    }

    @OnClick({R.id.tv_sure})
    public void onViewClicked(View view) {
        switch (view.getId()) {
            case R.id.tv_sure:
                //确定
                String num = etPwd.getText().toString().trim();
                if (TextUtils.isEmpty(num)) {
                    showToastMessage(getString(R.string.munnotnull));
                    return;
                }
                if (num.startsWith(".")) {
                    showToastMessage(getString(R.string.inputmustnum));
                    return;
                }
//                if (Long.parseLong(pwd) > Long.parseLong(fee)) {
//                    showToastMessage(getString(R.string.lack_of_balance));
//                    return;
//                }
                post(RxEnum.VOTETRANSFERACTIVITY.ordinal(), num, null);
                finish();

                // dialogUtil.showWarmPromptInput(this, null, null, this);

                break;

        }
    }

}
