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

package org.elastos.wallet.ela.ui.mine.fragment;

import android.support.v7.widget.LinearLayoutManager;
import android.support.v7.widget.RecyclerView;
import android.view.View;
import android.widget.ImageView;
import android.widget.LinearLayout;
import android.widget.TextView;

import org.elastos.wallet.R;
import org.elastos.wallet.ela.base.BaseFragment;
import org.elastos.wallet.ela.bean.BusEvent;
import org.elastos.wallet.ela.ui.Assets.AssetskFragment;
import org.elastos.wallet.ela.ui.mine.adapter.MessageListRecAdapetr;
import org.elastos.wallet.ela.ui.mine.bean.MessageEntity;
import org.elastos.wallet.ela.utils.CacheUtil;
import org.elastos.wallet.ela.utils.RxEnum;
import org.greenrobot.eventbus.Subscribe;
import org.greenrobot.eventbus.ThreadMode;

import java.util.List;

import butterknife.BindView;
import butterknife.OnClick;

public class MessageListFragment extends BaseFragment {


    @BindView(R.id.tv_title)
    TextView tvTitle;
    @BindView(R.id.iv_title_right)
    ImageView ivTitleRight;
    @BindView(R.id.rv_unread)
    RecyclerView rvUnread;
    @BindView(R.id.rv_read)
    RecyclerView rvRead;
    @BindView(R.id.tv_unread)
    TextView tvUnread;

    List<MessageEntity> unReadList;
    List<MessageEntity> readList;
    @BindView(R.id.ll_nomessage)
    LinearLayout llNomessage;


    @Override
    protected int getLayoutId() {
        return R.layout.fragment_messagelist;
    }

    @Override
    protected void initView(View view) {
        registReceiver();
        tvTitle.setText(getString(R.string.messagecenter));

        ivTitleRight.setVisibility(View.VISIBLE);
        ivTitleRight.setImageResource(R.mipmap.asset_wallet_setting);
        setDate();

    }

    private void setDate() {
        unReadList = CacheUtil.getUnReadMessage();
        if (AssetskFragment.messageList != null && AssetskFragment.messageList.size() > 0) {
            unReadList.addAll(AssetskFragment.messageList);
        }
        if (unReadList.size() == 0) {
            rvUnread.setVisibility(View.GONE);
            tvUnread.setVisibility(View.GONE);
        } else {
            //展示unReadList
            setRecycleView(rvUnread, unReadList);
        }
        readList = CacheUtil.getReadMessage();
        if (readList.size() == 0) {
            rvRead.setVisibility(View.GONE);
        } else {
            setRecycleView(rvRead, readList);
        }
        if (rvRead.getVisibility() == View.GONE && rvUnread.getVisibility() == View.GONE) {
            llNomessage.setVisibility(View.VISIBLE);

        }
        readList.addAll(unReadList);
        CacheUtil.setReadMessage(readList);
        CacheUtil.remove("unReadMessage");
        if (AssetskFragment.messageList != null)
            AssetskFragment.messageList.clear();
        post(RxEnum.READNOTICE.ordinal(), null, null);
    }

    public void setRecycleView(RecyclerView rv, List<MessageEntity> list) {
        MessageListRecAdapetr adapter = new MessageListRecAdapetr(getContext(), list);
        rv.setLayoutManager(new LinearLayoutManager(getContext(), LinearLayoutManager.VERTICAL, false));
        rv.setAdapter(adapter);
    }


    @OnClick({R.id.iv_title_right})
    public void onViewClicked(View view) {
        switch (view.getId()) {
            case R.id.iv_title_right:
                //设置
                start(MessageSettingFragment.class);
                break;
        }

    }

    @Subscribe(threadMode = ThreadMode.MAIN)
    public void Event(BusEvent result) {
        int code = result.getCode();
        if (code == RxEnum.REFRESHMESSAGE.ordinal()) {
            //刷新数据
            setDate();
        }

    }
}
