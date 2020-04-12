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

package org.elastos.wallet.ela.ui.mine;

import android.content.Intent;
import android.os.Bundle;
import android.support.v7.widget.LinearLayoutManager;
import android.support.v7.widget.RecyclerView;
import android.view.View;
import android.widget.ImageView;
import android.widget.LinearLayout;
import android.widget.RelativeLayout;
import android.widget.TextView;

import org.elastos.wallet.R;
import org.elastos.wallet.ela.base.BaseFragment;
import org.elastos.wallet.ela.bean.BusEvent;
import org.elastos.wallet.ela.db.RealmUtil;
import org.elastos.wallet.ela.db.table.Contact;
import org.elastos.wallet.ela.ui.Assets.AssetskFragment;
import org.elastos.wallet.ela.ui.common.listener.CommonRvListener;
import org.elastos.wallet.ela.ui.main.MainActivity;
import org.elastos.wallet.ela.ui.mine.adapter.ContactRecAdapetr;
import org.elastos.wallet.ela.ui.mine.fragment.AboutFragment;
import org.elastos.wallet.ela.ui.mine.fragment.ContactDetailFragment;
import org.elastos.wallet.ela.ui.mine.fragment.MessageListFragment;
import org.elastos.wallet.ela.utils.CacheUtil;
import org.elastos.wallet.ela.utils.Constant;
import org.elastos.wallet.ela.utils.RxEnum;
import org.elastos.wallet.ela.utils.SPUtil;
import org.greenrobot.eventbus.Subscribe;
import org.greenrobot.eventbus.ThreadMode;

import java.util.ArrayList;
import java.util.List;

import butterknife.BindView;
import butterknife.OnClick;

/**
 * tab-设置
 */

public class MineFragment extends BaseFragment implements CommonRvListener {

    @BindView(R.id.iv_title_left)
    ImageView ivTitleLeft;
    @BindView(R.id.tv_title)
    TextView tvTitle;
    @BindView(R.id.iv_title_right)
    ImageView ivTitleRight;
    @BindView(R.id.iv_language)
    ImageView ivLanguage;
    @BindView(R.id.rl_language)
    RelativeLayout rlLanguage;
    @BindView(R.id.tv_chinese)
    TextView tvChinese;
    @BindView(R.id.tv_english)
    TextView tvEnglish;
    @BindView(R.id.ll_languge)
    LinearLayout llLanguge;
    @BindView(R.id.tv_language)
    TextView tvLanguage;
    @BindView(R.id.iv_contact)
    ImageView ivContact;
    @BindView(R.id.rl_contact)
    RelativeLayout rlContact;
    @BindView(R.id.tv_contact_none)
    TextView tvContactNone;
    @BindView(R.id.rv)
    RecyclerView rv;
    @BindView(R.id.iv_contact_add)
    ImageView ivContactAdd;
    private SPUtil sp;
    private RealmUtil realmUtil;
    private List<Contact> contacts = new ArrayList<>();
    private ContactRecAdapetr adapter;


    @Override
    protected int getLayoutId() {
        return R.layout.fragment_mine;
    }

    @Override
    protected void initInjector() {

    }

    @Override
    protected void initView(View view) {
        ivTitleRight.setVisibility(View.VISIBLE);
        ivTitleRight.setImageResource(R.mipmap.mine_message_center);
        tvTitle.setText(R.string.mine);
        ivTitleLeft.setVisibility(View.GONE);
        sp = new SPUtil(getContext());
        llLanguge.getChildAt(sp.getLanguage()).setSelected(true);
        tvLanguage.setText(sp.getLanguage() == 0 ? "中文(简体)" : "English");
        llLanguge.getChildAt(sp.getLanguage()).setSelected(true);
        realmUtil = new RealmUtil();
        registReceiver();
        if (sp.isOpenRedPoint() && ((AssetskFragment.messageList != null && AssetskFragment.messageList.size() > 0) || CacheUtil.getUnReadMessage().size() > 0)) {
            //有新消息
            ivTitleRight.setImageResource(R.mipmap.mine_message_center_red);
        }
    }

    @OnClick({R.id.rl_language, R.id.rl_contact, R.id.tv_chinese, R.id.tv_english,
            R.id.iv_contact_add, R.id.rl_about, R.id.iv_title_right})
    public void onViewClicked(View view) {
        switch (view.getId()) {
            case R.id.tv_chinese:
                if (sp.getLanguage() == 0) {
                    return;
                }
                tvEnglish.setSelected(false);
                tvChinese.setSelected(true);
                sp.setLanguage(0);
                changeAppLanguage();

                break;
            case R.id.tv_english:
                if (sp.getLanguage() == 1) {
                    return;
                }
                tvChinese.setSelected(false);
                tvEnglish.setSelected(true);
                sp.setLanguage(1);
                changeAppLanguage();
                break;
            case R.id.rl_language:
                if (llLanguge.getVisibility() == View.VISIBLE) {
                    llLanguge.setVisibility(View.GONE);
                    ivLanguage.setImageResource(R.mipmap.asset_list_arrow);
                } else {
                    llLanguge.setVisibility(View.VISIBLE);
                    ivLanguage.setImageResource(R.mipmap.setting_list_arrow);
                }
                break;


            case R.id.rl_contact:
                List<Contact> tempContacts = realmUtil.queryAllContact();
                if (ivContactAdd.getVisibility() == View.VISIBLE) {
                    ivContactAdd.setVisibility(View.GONE);
                    tvContactNone.setVisibility(View.GONE);
                    rv.setVisibility(View.GONE);
                    ivContact.setImageResource(R.mipmap.asset_list_arrow);
                } else {
                    ivContactAdd.setVisibility(View.VISIBLE);
                    ivContact.setImageResource(R.mipmap.setting_list_arrow);
                    if (tempContacts.size() == 0) {
                        tvContactNone.setVisibility(View.VISIBLE);
                        rv.setVisibility(View.GONE);
                    } else {
                        rv.setVisibility(View.VISIBLE);
                        contacts.clear();
                        contacts.addAll(tempContacts);
                        setRecycleView();
                    }

                }

                break;
            case R.id.iv_contact_add:
                Bundle bundle = new Bundle();
                bundle.putString("type", Constant.CONTACTADD);
                ContactDetailFragment contactDetailFragment = new ContactDetailFragment();
                contactDetailFragment.setArguments(bundle);
                ((BaseFragment) getParentFragment()).start(contactDetailFragment);

                break;
            case R.id.rl_about:
                ((BaseFragment) getParentFragment()).start(AboutFragment.class);
                break;
            case R.id.iv_title_right:
                //消息中心
                ((BaseFragment) getParentFragment()).start(MessageListFragment.class);
                break;
        }
    }

  /*  @Override
    public void onResume() {
        super.onResume();
        Log.i("dasdsa", "onResume");
    }

    @Override
    public void onHiddenChanged(boolean hidden) {
        super.onHiddenChanged(hidden);
        Log.i("dasdsa", "hidden" + hidden);
    }

    @Override
    public void onViewCreated(View view, @Nullable Bundle savedInstanceState) {
        super.onViewCreated(view, savedInstanceState);
        Log.i("dasdsa", "onViewCreated");
    }*/

    @Subscribe(threadMode = ThreadMode.MAIN)
    public void Event(BusEvent result) {
        int integer = result.getCode();
        if (integer == RxEnum.UPDATACONTACT.ordinal()) {

            List<Contact> tempContacts = realmUtil.queryAllContact();
            if (tempContacts.size() == 0) {
                tvContactNone.setVisibility(View.VISIBLE);
                rv.setVisibility(View.GONE);
            } else {
                tvContactNone.setVisibility(View.GONE);
                contacts.clear();
                contacts.addAll(tempContacts);
                rv.setVisibility(View.VISIBLE);
                setRecycleView();
            }
        }
        if (integer == RxEnum.NOTICE.ordinal()) {
            //新的消息通知
            if (sp.isOpenRedPoint()) {
                ivTitleRight.setImageResource(R.mipmap.mine_message_center_red);
            }
        }
        if (integer == RxEnum.READNOTICE.ordinal()) {
            //新的消息通知
            if (sp.isOpenRedPoint()) {
                ivTitleRight.setImageResource(R.mipmap.mine_message_center);
            }
        }
    }


    public static MineFragment newInstance() {
        Bundle args = new Bundle();
        MineFragment fragment = new MineFragment();
        fragment.setArguments(args);
        return fragment;
    }

    public void changeAppLanguage() {
        /*String sta = sp.getLanguage() == 0 ? "zh" : "en";//这是SharedPreferences工具类，用于保存设置，代码很简单，自己实现吧
        // 本地语言设置
        Locale myLocale = new Locale(sta);
        Resources res = getResources();
        DisplayMetrics dm = res.getDisplayMetrics();
        Configuration conf = res.getConfiguration();
        conf.locale = myLocale;
        res.updateConfiguration(conf, dm);*/
//todo  不销毁wallet post 或者application或者 firstfrag用activity
        post(RxEnum.CHANGELANGUAGE.ordinal(), null, null);
        Intent intent = new Intent(getActivity(), MainActivity.class);
        intent.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK | Intent.FLAG_ACTIVITY_CLEAR_TASK);
        startActivity(intent);
        getActivity().finish();
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

    /**
     * 处理回退事件
     *
     * @return
     */
    @Override
    public boolean onBackPressedSupport() {
        return closeApp();
    }


    @Override
    public void onRvItemClick(int position, Object o) {
        Bundle bundle = new Bundle();
        bundle.putParcelable("Contact", (Contact) o);
        bundle.putString("type", Constant.CONTACTSHOW);
        ContactDetailFragment contactDetailFragment = new ContactDetailFragment();
        contactDetailFragment.setArguments(bundle);
        ((BaseFragment) getParentFragment()).start(contactDetailFragment);
    }


}
