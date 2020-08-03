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

import android.os.Bundle;
import android.support.v7.widget.LinearLayoutManager;
import android.support.v7.widget.RecyclerView;
import android.view.View;
import android.widget.ImageView;
import android.widget.LinearLayout;
import android.widget.TextView;

import org.elastos.wallet.R;
import org.elastos.wallet.ela.base.BaseFragment;
import org.elastos.wallet.ela.bean.BusEvent;
import org.elastos.wallet.ela.db.RealmUtil;
import org.elastos.wallet.ela.db.table.Contact;
import org.elastos.wallet.ela.ui.common.listener.CommonRvListener;
import org.elastos.wallet.ela.ui.mine.adapter.ContactRecAdapetr;
import org.elastos.wallet.ela.utils.Constant;
import org.elastos.wallet.ela.utils.RxEnum;
import org.greenrobot.eventbus.Subscribe;
import org.greenrobot.eventbus.ThreadMode;

import java.util.ArrayList;
import java.util.List;

import butterknife.BindView;
import butterknife.OnClick;

public class ContactFragment extends BaseFragment implements CommonRvListener {

    @BindView(R.id.tv_title)
    TextView tvTitle;
    @BindView(R.id.iv_title_right)
    ImageView ivTitleRight;
    @BindView(R.id.ll_nocontract)
    LinearLayout llNocontract;
    @BindView(R.id.rv)
    RecyclerView rv;
    private RealmUtil realmUtil;
    private List<Contact> contacts = new ArrayList<>();
    private ContactRecAdapetr adapter;

    @Override
    protected int getLayoutId() {
        return R.layout.fragment_contract;
    }

    @Override
    protected void initInjector() {

    }

    @Override
    protected void initView(View view) {
        tvTitle.setText(R.string.contact);
        realmUtil = new RealmUtil();
        ivTitleRight.setImageResource(R.mipmap.setting_add_contract);
        List<Contact> tempContacts = new RealmUtil().queryAllContact();
        if (tempContacts.size() == 0) {
            llNocontract.setVisibility(View.VISIBLE);
            rv.setVisibility(View.GONE);
        } else {
            ivTitleRight.setVisibility(View.VISIBLE);
            llNocontract.setVisibility(View.GONE);
            rv.setVisibility(View.VISIBLE);
            contacts.clear();
            contacts.addAll(tempContacts);
            setRecycleView();
        }
        registReceiver();
    }

    private void setRecycleView() {
        if (adapter == null) {
            adapter = new ContactRecAdapetr(getContext(), contacts);
            rv.setLayoutManager(new LinearLayoutManager(getContext(), LinearLayoutManager.VERTICAL, false));
            rv.setHasFixedSize(true);
            rv.setNestedScrollingEnabled(false);
            rv.setFocusableInTouchMode(false);
            rv.setAdapter(adapter);
            adapter.setCommonRvListener(this);

        } else {
            adapter.notifyDataSetChanged();
        }
    }

    @OnClick({R.id.iv_title_right, R.id.tv_contact_add})
    public void onViewClicked(View view) {
        switch (view.getId()) {
            case R.id.iv_title_right:
            case R.id.tv_contact_add:
                Bundle bundle = new Bundle();
                bundle.putString("type", Constant.CONTACTADD);
                ContactDetailFragment contactDetailFragment = new ContactDetailFragment();
                contactDetailFragment.setArguments(bundle);
                start(contactDetailFragment);
                break;

        }
    }

    @Subscribe(threadMode = ThreadMode.MAIN)
    public void Event(BusEvent result) {
        int integer = result.getCode();
        if (integer == RxEnum.UPDATACONTACT.ordinal()) {

            List<Contact> tempContacts = realmUtil.queryAllContact();
            if (tempContacts.size() == 0) {
                ivTitleRight.setVisibility(View.GONE);
                llNocontract.setVisibility(View.VISIBLE);
                rv.setVisibility(View.GONE);
            } else {
                ivTitleRight.setVisibility(View.VISIBLE);
                llNocontract.setVisibility(View.GONE);
                rv.setVisibility(View.VISIBLE);
                contacts.clear();
                contacts.addAll(tempContacts);
                setRecycleView();
            }
        }

    }

    @Override
    public void onRvItemClick(int position, Object o) {
        Bundle bundle = new Bundle();
        bundle.putParcelable("Contact", (Contact) o);
        bundle.putString("type", Constant.CONTACTSHOW);
        ContactDetailFragment contactDetailFragment = new ContactDetailFragment();
        contactDetailFragment.setArguments(bundle);
        start(contactDetailFragment);
    }
}
