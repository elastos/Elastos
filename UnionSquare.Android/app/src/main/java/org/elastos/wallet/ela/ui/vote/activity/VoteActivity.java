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

package org.elastos.wallet.ela.ui.vote.activity;

import android.content.Intent;
import android.graphics.Color;
import android.graphics.drawable.ColorDrawable;
import android.text.TextUtils;
import android.view.MotionEvent;
import android.view.View;
import android.view.ViewGroup;
import android.widget.TextView;

import org.elastos.wallet.R;
import org.elastos.wallet.ela.base.BaseActivity;
import org.elastos.wallet.ela.utils.AndroidWorkaround;
import org.elastos.wallet.ela.utils.ClearEditText;
import org.elastos.wallet.ela.utils.Constant;
import org.elastos.wallet.ela.utils.NumberiUtil;
import org.elastos.wallet.ela.utils.RxEnum;

import butterknife.BindView;
import butterknife.OnClick;

/**
 * 输入投票数量
 */
public class VoteActivity extends BaseActivity {
    @BindView(R.id.et_vote)
    ClearEditText etVote;
    String maxBalance;
    @BindView(R.id.tv_max)
    TextView tv_max;
    @BindView(R.id.tv_maxvote)
    TextView tv_maxvote;
    @BindView(R.id.tv_title)
    TextView tvTitle;
    @BindView(R.id.tv_vote_tag)
    TextView tvVoteTag;
    private String type, openType;

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
        openType = data.getStringExtra("openType");
        type = data.getStringExtra("type") + "";
        maxBalance = data.getStringExtra("maxBalance");
        NumberiUtil.editTestFormat(etVote, 8);
        switch (type) {
            case Constant.PROPOSALPUBLISHED:
                tvVoteTag.setText(R.string.votemoney);
                tvTitle.setText(R.string.votedisagree);
                tv_max.setBackgroundResource(0);
                tv_max.setText("ELA");
                etVote.setHint(getString(R.string.available) + maxBalance);
                break;
            case Constant.IMPEACHMENTCRC:
                tvVoteTag.setText(R.string.impeachvotes);
                tvTitle.setText(R.string.impeachment);
                tv_max.setBackgroundResource(0);
                tv_max.setText("ELA");
                etVote.setHint(getString(R.string.available) + maxBalance);
                break;
            default:
                tv_max.setOnClickListener(v -> etVote.setText("MAX"));
                tv_maxvote.setText(getString(R.string.maximum_voting_rights) + "  " + maxBalance);
                etVote.setOnTouchListener(new View.OnTouchListener() {
                    @Override
                    public boolean onTouch(View v, MotionEvent event) {
                        if (!TextUtils.isEmpty(etVote.getText()) && etVote.getText().toString().equals("MAX")) {
                            etVote.setText("");
                        }
                        return false;
                    }
                });

      /*  etPwd.setOnFocusChangeListener(new View.OnFocusChangeListener() {
            @Override
            public void onFocusChange(View v, boolean hasFocus) {
                Log.d("???",hasFocus+"");
                if (hasFocus && !TextUtils.isEmpty(etPwd.getText()) && etPwd.getText().toString().equals("MAX")) {
                    tv_maxvote.setText("");
                }
            }
        });*/
        }
    }

    @OnClick({R.id.tv_sure})
    public void onViewClicked(View view) {
        switch (view.getId()) {
            case R.id.tv_sure:
                //确定
                String num = etVote.getText().toString().trim();
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
                post(RxEnum.VOTETRANSFERACTIVITY.ordinal(), openType, num);
                finish();

                // dialogUtil.showWarmPromptInput(this, null, null, this);

                break;

        }
    }

}
