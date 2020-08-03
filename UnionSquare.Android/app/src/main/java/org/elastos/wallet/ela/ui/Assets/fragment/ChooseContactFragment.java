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

package org.elastos.wallet.ela.ui.Assets.fragment;

import android.os.Bundle;
import android.support.v7.widget.LinearLayoutManager;
import android.support.v7.widget.RecyclerView;
import android.view.View;
import android.widget.LinearLayout;
import android.widget.TextView;

import org.elastos.wallet.R;
import org.elastos.wallet.ela.base.BaseFragment;
import org.elastos.wallet.ela.db.RealmUtil;
import org.elastos.wallet.ela.db.listener.RealmTransactionAbs;
import org.elastos.wallet.ela.db.table.Contact;
import org.elastos.wallet.ela.ui.Assets.adapter.ChooseContactRecAdapter;
import org.elastos.wallet.ela.ui.common.listener.CommonRvListener;
import org.elastos.wallet.ela.ui.did.fragment.DIDCardDetailFragment;
import org.elastos.wallet.ela.ui.mine.fragment.ContactDetailFragment;
import org.elastos.wallet.ela.utils.Constant;
import org.elastos.wallet.ela.utils.DialogUtil;
import org.elastos.wallet.ela.utils.RxEnum;
import org.elastos.wallet.ela.utils.listener.WarmPromptListener;

import java.util.List;

import butterknife.BindView;
import butterknife.OnClick;

public class ChooseContactFragment extends BaseFragment implements CommonRvListener {
    @BindView(R.id.tv_title)
    TextView tvTitle;
    @BindView(R.id.ll_add)
    LinearLayout llAdd;
    @BindView(R.id.tv_add)
    TextView tvAdd;
    @BindView(R.id.rv)
    RecyclerView rv;
    @BindView(R.id.tv_title_right)
    TextView tvTitleRight;
    private List<Contact> list;

    String type;

    @Override
    protected int getLayoutId() {
        return R.layout.fragment_contact_choose;
    }

    @Override
    protected void setExtraData(Bundle data) {
        type = data.getString("type");
        if ("update".equals(type)) {
            //did卡片详情进入
            llAdd.setVisibility(View.VISIBLE);
            tvTitleRight.setVisibility(View.VISIBLE);
            tvTitleRight.setText(R.string.sure);
        }


    }

    @Override
    protected void initView(View view) {
        tvTitle.setText(R.string.choosecontact);
        list = new RealmUtil().queryAllContact();
        if (list == null || list.size() == 0) {
            tvTitleRight.setVisibility(View.GONE);
        } else {
            setRecycleView();
        }
    }

    private void setRecycleView() {

        ChooseContactRecAdapter adapter = new ChooseContactRecAdapter(getContext(), list);
        rv.setLayoutManager(new LinearLayoutManager(getContext(), LinearLayoutManager.VERTICAL, false));
        rv.setAdapter(adapter);
        adapter.setCommonRvListener(this);
    }

    @OnClick({R.id.tv_add, R.id.tv_title_right})
    public void onViewClicked(View view) {
        switch (view.getId()) {
            case R.id.tv_title_right:

                if (selectContact == null) {
                    showToastMessage(getString(R.string.plzselectcontact));
                } else {

                    new DialogUtil().showCommonWarmPrompt(getBaseActivity(), getString(R.string.whetheroverridecontact), null, null, false, new WarmPromptListener() {
                        @Override
                        public void affireBtnClick(View view) {
                            //直接更新联系人页面
                            selectContact.setWalletAddr(getArguments().getString("address"));
                            selectContact.setDid(getArguments().getString("didString"));
                            new RealmUtil().insertContact(selectContact, new RealmTransactionAbs() {
                                @Override
                                public void onSuccess() {
                                    toTargetFragment(new DIDCardDetailFragment());
                                }
                            });

                        }
                    });
                }

                break;
            case R.id.tv_add:
                //添加联系人
                Bundle bundle = getArguments();
                bundle.putString("type", Constant.CONTACTADDDID);
                start(ContactDetailFragment.class, bundle);
                break;
        }
    }

    Contact selectContact;

    @Override
    public void onRvItemClick(int position, Object o) {
        selectContact = list.get(position);
        if (!"update".equals(type)) {
            //默认选择联系人并返回
            post(RxEnum.CHOOSECONTACT.ordinal(), "选择联系人", selectContact);
            popBackFragment();
        }
    }


}
