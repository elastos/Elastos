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

import android.content.Intent;
import android.os.Bundle;
import android.support.v7.widget.LinearLayoutManager;
import android.support.v7.widget.RecyclerView;
import android.view.View;
import android.view.ViewGroup;
import android.widget.ImageView;
import android.widget.ScrollView;
import android.widget.TextView;

import com.alibaba.fastjson.JSON;

import org.elastos.did.DIDDocument;
import org.elastos.wallet.R;
import org.elastos.wallet.ela.ElaWallet.MyWallet;
import org.elastos.wallet.ela.base.BaseFragment;
import org.elastos.wallet.ela.bean.BusEvent;
import org.elastos.wallet.ela.db.table.Wallet;
import org.elastos.wallet.ela.rxjavahelp.BaseEntity;
import org.elastos.wallet.ela.rxjavahelp.NewBaseViewData;
import org.elastos.wallet.ela.ui.common.listener.CommonRvListener1;
import org.elastos.wallet.ela.ui.crvote.presenter.CRSignUpPresenter;
import org.elastos.wallet.ela.ui.did.adapter.PersonalAddRecAdapetr;
import org.elastos.wallet.ela.ui.did.adapter.PersonalChoseRecAdapetr;
import org.elastos.wallet.ela.ui.did.entity.CredentialSubjectBean;
import org.elastos.wallet.ela.ui.did.entity.PersonalInfoItemEntity;
import org.elastos.wallet.ela.ui.vote.activity.OtherPwdActivity;
import org.elastos.wallet.ela.ui.vote.activity.VoteTransferActivity;
import org.elastos.wallet.ela.ui.vote.bean.Area;
import org.elastos.wallet.ela.ui.vote.fragment.AreaCodeFragment;
import org.elastos.wallet.ela.utils.AppUtlis;
import org.elastos.wallet.ela.utils.Constant;
import org.elastos.wallet.ela.utils.DateUtil;
import org.elastos.wallet.ela.utils.DialogUtil;
import org.elastos.wallet.ela.utils.Log;
import org.elastos.wallet.ela.utils.RxEnum;
import org.elastos.wallet.ela.utils.SPUtil;
import org.elastos.wallet.ela.utils.listener.WarmPromptListener;
import org.elastos.wallet.ela.utils.widget.TextConfigDataPicker;
import org.elastos.wallet.ela.utils.widget.TextConfigNumberPicker;
import org.greenrobot.eventbus.Subscribe;
import org.greenrobot.eventbus.ThreadMode;

import java.util.ArrayList;
import java.util.Calendar;
import java.util.Collections;
import java.util.Date;
import java.util.List;

import butterknife.BindView;
import butterknife.OnClick;

public class AddPersonalInfoFragment extends BaseFragment implements CommonRvListener1, NewBaseViewData {

    @BindView(R.id.tv_title)
    TextView tvTitle;

    String[] sexs;

    @BindView(R.id.tv_title_right)
    TextView tvTitleRight;
    @BindView(R.id.iv_add)
    ImageView ivAdd;
    @BindView(R.id.tv_tip)
    TextView tvTip;
    @BindView(R.id.rv_show)
    RecyclerView rvShow;
    @BindView(R.id.rv_chose)
    RecyclerView rvChose;
    @BindView(R.id.sv_chose)
    ScrollView svChose;

    private CredentialSubjectBean credentialSubjectBean;
    private long birthday;
    private Wallet wallet;
    private String didName;
    private Date didEndDate;
    private List<PersonalInfoItemEntity> listShow;
    private List<PersonalInfoItemEntity> listChose;
    private PersonalAddRecAdapetr adapterShow;
    private PersonalChoseRecAdapetr adapterChose;
    private TextView tvPersonalIntro;

    @Override
    protected int getLayoutId() {
        return R.layout.fragment_personalinfo;
    }


    @Override
    protected void setExtraData(Bundle data) {

        String type = data.getString("type");
        if (Constant.EDITCREDENTIAL.equals(type)) {
            //新增did凭证  从凭证信息进入
            onAddPartCredential();
        } else {
            //新增did凭证  从新增did进入
            wallet = data.getParcelable("wallet");
            didName = data.getString("didName");
            didEndDate = (Date) data.getSerializable("didEndDate");
            tvTitleRight.setText(getString(R.string.publish));
        }
    }

    private void onAddPartCredential() {
        tvTitleRight.setText(getString(R.string.keep));
        tvTitleRight.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                CredentialSubjectBean credentialSubjectBean = convertCredentialSubjectBean();
                Log.i("??", JSON.toJSONString(credentialSubjectBean));
                DIDDocument doc = getMyDID().getDIDDocument();
                Date didEndDate = getMyDID().getExpires(doc);
                Intent intent = new Intent(getActivity(), OtherPwdActivity.class);
                intent.putExtra("didEndDate", didEndDate);
                intent.putExtra("credentialSubjectBean", credentialSubjectBean);
                intent.putExtra("type", Constant.EDITCREDENTIAL);
                startActivity(intent);
            }
        });
    }

    @Override
    protected void initView(View view) {
       /* rvShow.setNestedScrollingEnabled(false);
        //当知道Adapter内Item的改变不会影响RecyclerView宽高的时候，可以设置为true让RecyclerView避免重新计算大小
        rvShow.setHasFixedSize(true);
        //解决数据加载完成后, 没有停留在顶部的问题
        rvShow.setFocusable(false);*/
        //  rvCnto.setHasFixedSize(true);
        //            rvCnto.setNestedScrollingEnabled(false);
        //            rvCnto.setFocusableInTouchMode(false);
        //            rvCnto.requestFocus();
        tvTitle.setText(getString(R.string.addpersonalindo));
        tvTitleRight.setVisibility(View.VISIBLE);
        initItemDate();
        sexs = new String[]{getString(R.string.man), getString(R.string.woman)};
        registReceiver();
    }

    private void initItemDate() {
        String showData[] = getResources().getStringArray(R.array.personalinfo_show);
        String choseData[] = getResources().getStringArray(R.array.personalinfo_chose);
        /*  Map<Integer, String>*/
        listShow = new ArrayList<>();
        listChose = new ArrayList<>();
        for (int i = 0; i < showData.length; i++) {
            PersonalInfoItemEntity personalInfoItemEntity = new PersonalInfoItemEntity();
            personalInfoItemEntity.setIndex(i);
            personalInfoItemEntity.setHintShow1(showData[i]);
            if (i == 5) {
                personalInfoItemEntity.setHintShow2(getString(R.string.pleaseinputmobile));
            }
            personalInfoItemEntity.setHintChose(choseData[i]);
            if (i == 1 || i == 2 || i == 3 || i == 4 || i == 7 || i == 8 || i == 9 || i == 12)
                listShow.add(personalInfoItemEntity);
            else
                listChose.add(personalInfoItemEntity);

        }

        setRecycleViewShow();
        setRecycleViewChose();
    }

    private void setRecycleViewShow() {
        if (adapterShow == null) {
            adapterShow = new PersonalAddRecAdapetr(getContext(), listShow);
            rvShow.setLayoutManager(new LinearLayoutManager(getContext(), LinearLayoutManager.VERTICAL, false));
            rvShow.setAdapter(adapterShow);
            adapterShow.setCommonRvListener(this);

        } else {
            adapterShow.notifyDataSetChanged();
        }
    }

    private void setRecycleViewChose() {
        if (adapterChose == null) {
            adapterChose = new PersonalChoseRecAdapetr(getContext(), listChose);
            rvChose.setLayoutManager(new LinearLayoutManager(getContext(), LinearLayoutManager.VERTICAL, false));
            rvChose.setAdapter(adapterChose);
            adapterChose.setCommonRvListener(this);

        } else {
            adapterChose.notifyDataSetChanged();
        }
    }

    @OnClick({R.id.tv_title_right, R.id.iv_add, R.id.iv_showshow})
    public void onViewClicked(View view) {
        switch (view.getId()) {
            case R.id.tv_title_right:
                //发布  保留在重写的方法里
                credentialSubjectBean = convertCredentialSubjectBean();
                Log.i("??", JSON.toJSONString(credentialSubjectBean));
                //模拟手续费
                new CRSignUpPresenter().getFee(wallet.getWalletId(), MyWallet.IDChain, "", "8USqenwzA5bSAvj1mG4SGTABykE9n5RzJQ", "0", this);
                break;
            case R.id.iv_add:
                svChose.setVisibility(View.VISIBLE);
                break;
            case R.id.iv_showshow:
                svChose.setVisibility(View.GONE);
                break;
        }
    }

    TextView tvArea;

    @Subscribe(threadMode = ThreadMode.MAIN)
    public void Event(BusEvent result) {
        int integer = result.getCode();
        if (integer == RxEnum.AREA.ordinal()) {
            Area area = (Area) result.getObj();
            int Language = new SPUtil(getContext()).getLanguage();
            String name;
            if (Language == 0) {
                name = area.getZh();
            } else {
                name = area.getEn();
            }
            tvArea.setText(name);
        }
        if (integer == RxEnum.EDITPERSONALINTRO.ordinal()) {
            String intro = (String) result.getObj();
            tvPersonalIntro.setText(intro);
        }
        if (integer == RxEnum.TRANSFERSUCESS.ordinal()) {
            new DialogUtil().showTransferSucess(getBaseActivity(), new WarmPromptListener() {
                @Override
                public void affireBtnClick(View view) {
                    toMainFragment();
                }
            });
        }
        if (integer == RxEnum.EDITPERSONALINFO.ordinal()) {
            //保留did成功
            new DialogUtil().showTransferSucess(getString(R.string.savesucess), getBaseActivity(), new WarmPromptListener() {
                @Override
                public void affireBtnClick(View view) {
                    popTo(CredentialFragment.class, false);
                }
            });
        }
    }


    @Override
    public void onRvItemClick(View v, int position, Object o) {
        PersonalInfoItemEntity personalInfoItemEntity = (PersonalInfoItemEntity) o;
        if (svChose.getVisibility() == View.VISIBLE) {
            //选择添加某一项 数据会重新渲染
            storePersonalInfo();
            int insetPosition = listShow.size();
            for (int i = 0; i < listShow.size(); i++) {
                if (personalInfoItemEntity.getIndex() < listShow.get(i).getIndex()) {
                    insetPosition = i;
                    break;
                }
            }
            listShow.add(personalInfoItemEntity);
            Collections.sort(listShow);
            adapterShow.notifyItemInserted(insetPosition);//加动画
            adapterShow.notifyItemRangeChanged(position, listShow.size() - insetPosition);
            listChose.remove(personalInfoItemEntity);
            adapterChose.notifyDataSetChanged();
            svChose.setVisibility(View.GONE);
            if (listChose.size() == 0) {
                ivAdd.setVisibility(View.GONE);
            }

        } else {
            if (v instanceof ImageView) {
                //去除某一项 数据会重新渲染
                storePersonalInfo();
                personalInfoItemEntity.setText1(null);
                personalInfoItemEntity.setText2(null);
                //通过右边的iv类型判断是条目的增减还是特殊条目数据填充
                listChose.add(personalInfoItemEntity);
                Collections.sort(listChose);
                listShow.remove(personalInfoItemEntity);
                // adapterShow.notifyItemRemoved(position);//加动画
                //adapterShow.notifyItemRangeChanged(position, listShow.size() - position);
                adapterChose.notifyDataSetChanged();
                adapterShow.notifyDataSetChanged();
                if (ivAdd.getVisibility() == View.GONE) {
                    ivAdd.setVisibility(View.VISIBLE);
                }
            } else {
                //特殊条目数据填充 序号是 1 2 6 7
                onRvTextViewClick((TextView) v, personalInfoItemEntity.getIndex());
            }


        }

    }


    private void onRvTextViewClick(TextView v, int index) {
        if (index == 1) {
            //选择性别
            new DialogUtil().showCommonSelect(getBaseActivity(), getString(R.string.plzselcetsex), sexs, new WarmPromptListener() {
                @Override
                public void affireBtnClick(View view) {

                    v.setText(sexs[((TextConfigNumberPicker) view).getValue()]);
                }
            });
        } else if (index == 2) {
            //选择出生日期
            Calendar calendar = Calendar.getInstance();
            long minData = calendar.getTimeInMillis();
            int year = calendar.get(Calendar.YEAR);
            calendar.set(Calendar.YEAR, year - 100);
            new DialogUtil().showTime(getBaseActivity(), getString(R.string.plzselectbirthday), calendar.getTimeInMillis(), minData, new WarmPromptListener() {
                @Override
                public void affireBtnClick(View view) {
                    String date = ((TextConfigDataPicker) view).getYear() + "." + (((TextConfigDataPicker) view).getMonth() + 1)
                            + "." + ((TextConfigDataPicker) view).getDayOfMonth();
                    birthday = DateUtil.parseToLong(date);

                    v.setText(DateUtil.timeNYR(birthday, getContext(), false));
                }
            });
        } else if (index == 6) {
            //国家地区
            start(AreaCodeFragment.class);
            tvArea = v;
        } else if (index == 7) {
            //个人简介
            Bundle bundle = new Bundle();
            bundle.putString("personalIntro", getText(v));
            bundle.putAll(getArguments());
            start(PersonalIntroFragment.class, bundle);
            tvPersonalIntro = v;
        }
    }

    /**
     * 保存数据同时可以防止再次渲染rv时数据混乱
     */
    private void storePersonalInfo() {
        for (int i = 0; i < listShow.size(); i++) {
            PersonalInfoItemEntity personalInfoItemEntity = listShow.get(i);
            ViewGroup view = (ViewGroup) (rvShow.getLayoutManager().findViewByPosition(i));
            if (view == null) {
                break;
            }
            TextView child0 = (TextView) view.getChildAt(0);

            personalInfoItemEntity.setText1(getText(child0));
            if (personalInfoItemEntity.getIndex() == 5) {
                //电话号的特殊情况
                View child2 = view.getChildAt(2);
                personalInfoItemEntity.setText2(getText((TextView) child2));

            }
            if (personalInfoItemEntity.getIndex() == 7) {
                //个人简介
                View child1 = view.getChildAt(1);
                personalInfoItemEntity.setText2(getText((TextView) child1));

            }
        }
    }

    private CredentialSubjectBean convertCredentialSubjectBean() {
        //这种情况考虑去除全局变量credentialSubjectBean
        storePersonalInfo();
        CredentialSubjectBean result = new CredentialSubjectBean(getMyDID().getDidString(), didName);
        for (int i = 0; i < listShow.size(); i++) {
            //只遍历show的数据
            PersonalInfoItemEntity personalInfoItemEntity = listShow.get(i);
            int index = personalInfoItemEntity.getIndex();
            String text1 = personalInfoItemEntity.getText1();
            String text2 = personalInfoItemEntity.getText2();
            switch (index) {
                case 0:
                    result.setNickname(text1);
                    break;
                case 1:
                    if (getString(R.string.man).equals(text1))
                        result.setGender("1");
                    else if (getString(R.string.woman).equals(text1))
                        result.setGender("2");
                    break;
                case 2:
                    String birthDate = DateUtil.parseToLongWithLanguage(text1, getContext(), true);
                    result.setBirthday(birthDate);
                    break;
                case 3:
                    result.setAvatar(text1);
                    break;
                case 4:
                    result.setEmail(text1);
                    break;
                case 5:
                    result.setPhoneCode(text1);
                    result.setPhone(text2);
                    break;
                case 6:
                    result.setNation(AppUtlis.getLocCode(text1));
                    break;
                case 7:

                    result.setIntroduction(text2);

                    break;
                case 8:
                    result.setHomePage(text1);
                    break;
                case 9:
                    result.setWechat(text1);
                    break;
                case 10:
                    result.setTwitter(text1);
                    break;
                case 11:
                    result.setWeibo(text1);
                    break;
                case 12:
                    result.setFacebook(text1);
                    break;
                case 13:
                    result.setGoogleAccount(text1);
                    break;
            }

        }
        result.setEditTime(new Date().getTime() / 1000);
        return result;
    }

    @Override
    public void onGetData(String methodName, BaseEntity baseEntity, Object o) {
        switch (methodName) {
            case "getFee":
                Intent intent = new Intent(getActivity(), VoteTransferActivity.class);
                intent.putExtra("didName", didName);
                intent.putExtra("didEndDate", didEndDate);
                intent.putExtra("credentialSubjectBean", credentialSubjectBean);
                intent.putExtra("wallet", wallet);
                intent.putExtra("chainId", MyWallet.IDChain);
                // intent.putExtra("fee", ((CommmonLongEntity) baseEntity).getData());
                intent.putExtra("fee", 20000L);
                intent.putExtra("type", Constant.DIDSIGNUP);
                intent.putExtra("transType", 10);
                startActivity(intent);
                break;
        }

    }

    @Override
    public boolean onBackPressedSupport() {
        if (svChose.getVisibility() == View.VISIBLE) {
            svChose.setVisibility(View.GONE);
            return true;
        } else {
            return super.onBackPressedSupport();
        }
    }
}
