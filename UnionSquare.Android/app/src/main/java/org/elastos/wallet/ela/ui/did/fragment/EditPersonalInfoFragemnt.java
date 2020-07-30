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
import android.widget.LinearLayout;
import android.widget.RelativeLayout;
import android.widget.ScrollView;
import android.widget.TextView;

import com.alibaba.fastjson.JSON;

import org.elastos.did.DIDDocument;
import org.elastos.wallet.R;
import org.elastos.wallet.ela.base.BaseFragment;
import org.elastos.wallet.ela.bean.BusEvent;
import org.elastos.wallet.ela.ui.common.listener.CommonRvListener1;
import org.elastos.wallet.ela.ui.did.adapter.PersonalChoseRecAdapetr;
import org.elastos.wallet.ela.ui.did.adapter.PersonalEditRecAdapetr;
import org.elastos.wallet.ela.ui.did.entity.CredentialSubjectBean;
import org.elastos.wallet.ela.ui.did.entity.PersonalInfoItemEntity;
import org.elastos.wallet.ela.ui.did.presenter.DIDUIPresenter;
import org.elastos.wallet.ela.ui.vote.activity.OtherPwdActivity;
import org.elastos.wallet.ela.ui.vote.bean.Area;
import org.elastos.wallet.ela.ui.vote.fragment.AreaCodeFragment;
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

public class EditPersonalInfoFragemnt extends BaseFragment implements CommonRvListener1 {

    @BindView(R.id.tv_title)
    TextView tvTitle;

    String[] sexs;

    @BindView(R.id.tv_title_right)
    TextView tvTitleRight;

    @BindView(R.id.tv_tip)
    TextView tvTip;
    @BindView(R.id.rv_show)
    RecyclerView rvShow;
    @BindView(R.id.rv_chose)
    RecyclerView rvChose;
    @BindView(R.id.sv_chose)
    ScrollView svChose;
    @BindView(R.id.iv_add)
    ImageView ivAdd;
    @BindView(R.id.rl_custom)
    RelativeLayout rlCustom;
    @BindView(R.id.ll_addcustom)
    LinearLayout llAddcustom;
    private long birthday;

    private List<PersonalInfoItemEntity> listShow;
    private List<PersonalInfoItemEntity> listChose;
    private PersonalEditRecAdapetr adapterShow;
    private PersonalChoseRecAdapetr adapterChose;
    private TextView curentTextView;

    @Override
    protected int getLayoutId() {
        return R.layout.fragment_personalinfo_edit;
    }


    @Override
    protected void setExtraData(Bundle data) {
        listShow = data.getParcelableArrayList("listShow");
        getChoseItem(listShow);
    }


    @Override
    protected void initView(View view) {

        tvTitleRight.setVisibility(View.VISIBLE);
        tvTitleRight.setText(getString(R.string.keep));
        tvTitle.setText(getString(R.string.editpersonalinfo));
        sexs = new String[]{getString(R.string.man), getString(R.string.woman)};
        setRecycleViewShow();
        setRecycleViewChose();
        registReceiver();
    }


    /**
     * 通过已经有数据的show 获得Chose
     *
     * @param listShow
     */
    private void getChoseItem(List<PersonalInfoItemEntity> listShow) {
        String choseData[] = getResources().getStringArray(R.array.personalinfo_chose);
        String showData[] = getResources().getStringArray(R.array.personalinfo_show);
        listChose = new ArrayList<>();
        for (int i = 0; i < choseData.length; i++) {
            PersonalInfoItemEntity personalInfoItemEntity = getPersonalInfoItemEntity(i, choseData[i], showData[i]);
            listChose.add(personalInfoItemEntity);

        }
        for (int j = 0; j < listShow.size(); j++) {
            PersonalInfoItemEntity personalInfoItemEntity = listShow.get(j);
            listChose.remove(personalInfoItemEntity);

        }

    }

    private PersonalInfoItemEntity getPersonalInfoItemEntity(int i, String choseDatum, String showDatum) {
        PersonalInfoItemEntity personalInfoItemEntity = new PersonalInfoItemEntity();
        personalInfoItemEntity.setIndex(i);
        personalInfoItemEntity.setHintShow1(showDatum);
        personalInfoItemEntity.setHintChose(choseDatum);
        if (i == 5) {
            personalInfoItemEntity.setHintShow2(getString(R.string.pleaseinputmobile));
            personalInfoItemEntity.setHintChose2(getString(R.string.phonecode));
        }

        return personalInfoItemEntity;
    }

    private void setRecycleViewShow() {
        if (adapterShow == null) {
            adapterShow = new PersonalEditRecAdapetr(getContext(), listShow);
            rvShow.setLayoutManager(new LinearLayoutManager(getContext(), LinearLayoutManager.VERTICAL, false));
            rvShow.setAdapter(adapterShow);
            adapterShow.setCommonRvListener(this);

        } else {
            adapterShow.notifyDataSetChanged();
        }
    }

    private void setRecycleViewChose() {
        customVisible();
        Collections.sort(listChose);
        if (adapterChose == null) {
            adapterChose = new PersonalChoseRecAdapetr(getContext(), listChose);
            rvChose.setLayoutManager(new LinearLayoutManager(getContext(), LinearLayoutManager.VERTICAL, false));
            rvChose.setAdapter(adapterChose);
            adapterChose.setCommonRvListener(this);

        } else {
            adapterChose.notifyDataSetChanged();
        }
    }

    private void insertShow(int insetPosition, PersonalInfoItemEntity personalInfoItemEntity) {
        storePersonalInfo();
        listShow.add(personalInfoItemEntity);
        Collections.sort(listShow);
        adapterShow.notifyItemInserted(insetPosition);//加动画
        adapterShow.notifyItemRangeChanged(insetPosition, listShow.size() - insetPosition);
        svChose.setVisibility(View.GONE);
        if (listShow.size() == 19) {//14项确定项和5项自定义项
            ivAdd.setVisibility(View.GONE);
        }
        listChose.remove(personalInfoItemEntity);
    }

    @OnClick({R.id.tv_title_right, R.id.iv_add, R.id.iv_showshow, R.id.rl_custom,
            R.id.iv_addcustom_back, R.id.rl_addcustom_single, R.id.rl_addcustom_mult})
    public void onViewClicked(View view) {
        switch (view.getId()) {
            case R.id.rl_addcustom_single:
                //增加单行自定义信息
                llAddcustom.setVisibility(View.GONE);
                int insetPosition = listShow.size();//插入位置最后的位置
                PersonalInfoItemEntity personalInfoItemEntity = new PersonalInfoItemEntity();
                int curentMaxIndex = listShow.get(insetPosition - 1).getIndex();
                personalInfoItemEntity.setIndex(curentMaxIndex > 13 ? curentMaxIndex + 1 : 14);
                personalInfoItemEntity.setType(-1);
                personalInfoItemEntity.setHintShow1(getString(R.string.singletextitem));
                personalInfoItemEntity.setHintShow2(getString(R.string.plzinputcontent));
                insertShow(insetPosition, personalInfoItemEntity);

                break;
            case R.id.rl_addcustom_mult:
                //增加多行自定义信息
                llAddcustom.setVisibility(View.GONE);
                int insetPosition1 = listShow.size();//插入位置最后的位置
                PersonalInfoItemEntity personalInfoItemEntity1 = new PersonalInfoItemEntity();
                int curentMaxIndex1 = listShow.get(insetPosition1 - 1).getIndex();
                personalInfoItemEntity1.setIndex(curentMaxIndex1 > 13 ? curentMaxIndex1 + 1 : 14);
                personalInfoItemEntity1.setType(-2);
                personalInfoItemEntity1.setHintShow1(getString(R.string.mutiltextitem));
                personalInfoItemEntity1.setHintShow2(getString(R.string.plzinputcontent));
                insertShow(insetPosition1, personalInfoItemEntity1);
                break;
            case R.id.iv_addcustom_back:
                llAddcustom.setVisibility(View.GONE);
                break;
            case R.id.rl_custom:
                llAddcustom.setVisibility(View.VISIBLE);
                break;
            case R.id.tv_title_right:
                //发布  保留在重写的方法里
                storePersonalInfo();
                CredentialSubjectBean credentialSubjectBean = new DIDUIPresenter().convertCredentialSubjectBean(this, getMyDID().getName(getMyDID().getDIDDocument()), listShow);
                Log.i("??", JSON.toJSONString(credentialSubjectBean));
                DIDDocument doc = getMyDID().getDIDDocument();
                //String didName = getMyDID().getName(doc);
                Date didEndDate = getMyDID().getExpires(doc);
                Intent intent = new Intent(getActivity(), OtherPwdActivity.class);
                intent.putExtra("didEndDate", didEndDate);
                intent.putExtra("credentialSubjectBean", credentialSubjectBean);
                intent.putExtra("type", Constant.EDITCREDENTIAL);
                startActivity(intent);
                break;
            case R.id.iv_add:
                svChose.setVisibility(View.VISIBLE);
                setRecycleViewChose();
                break;
            case R.id.iv_showshow:
                svChose.setVisibility(View.GONE);
                break;
        }
    }

    private void customVisible() {
        int customCount = 0;
        for (int i = 0; i < listShow.size(); i++) {
            if (listShow.get(i).getIndex() > 13) {
                customCount++;
            }
        }
        if (customCount >= 5) {//最多5项自定义项
            rlCustom.setVisibility(View.GONE);
        } else {
            rlCustom.setVisibility(View.VISIBLE);
        }
    }

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
            curentTextView.setText(name);
        }
        if (integer == RxEnum.EDITPERSONALINTRO.ordinal()) {
            String intro = (String) result.getObj();
            curentTextView.setText(intro);
        }
        if (integer == RxEnum.EDITPERSONALINFO.ordinal()) {
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
            int insetPosition = listShow.size();
            for (int i = 0; i < listShow.size(); i++) {
                if (personalInfoItemEntity.getIndex() < listShow.get(i).getIndex()) {
                    insetPosition = i;
                    break;
                }
            }
            insertShow(insetPosition, personalInfoItemEntity);
        } else {
            if (v instanceof ImageView) {
                //去除某一项 数据会重新渲染
                //提示是否删除
                new DialogUtil().showWarmPrompt1(getBaseActivity(), getString(R.string.whetherdeletethisitem), new WarmPromptListener() {
                    @Override
                    public void affireBtnClick(View view) {
                        storePersonalInfo();
                        if (personalInfoItemEntity.getIndex() <= 13) {
                            //自定义项目
                            personalInfoItemEntity.setText1(null);
                            personalInfoItemEntity.setText2(null);
                            listChose.add(personalInfoItemEntity);
                        }
                        listShow.remove(personalInfoItemEntity);
                        setRecycleViewShow();
                        if (ivAdd.getVisibility() == View.GONE) {
                            ivAdd.setVisibility(View.VISIBLE);
                        }
                    }
                });

            } else {
                //特殊条目数据填充 序号是 1 2 6 7
                onRvTextViewClick((TextView) v, personalInfoItemEntity);
            }


        }

    }


    private void onRvTextViewClick(TextView v, PersonalInfoItemEntity personalInfoItemEntity) {
        int index = personalInfoItemEntity.getIndex();
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
            curentTextView = v;
        } else if (index == 7) {
            //个人简介
            Bundle bundle = new Bundle();
            bundle.putString("content", getText(v));
            bundle.putAll(getArguments());
            start(PersonalIntroFragment.class, bundle);
            curentTextView = v;
        } else if (index > 13 && personalInfoItemEntity.getType() == -2) {
            //自定义项多行
            Bundle bundle = new Bundle();
            bundle.putString("title", personalInfoItemEntity.getText1() == null ? "" : personalInfoItemEntity.getText1());
            bundle.putString("content", getText(v));
            bundle.putAll(getArguments());
            start(PersonalIntroFragment.class, bundle);
            curentTextView = v;
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

            if (personalInfoItemEntity.getIndex() == 5) {
                //电话号的特殊情况
                TextView child1 = (TextView) view.getChildAt(1);
                personalInfoItemEntity.setText1(getText(child1));
                View child4 = view.getChildAt(4);
                personalInfoItemEntity.setText2(getText((TextView) child4));
            } else if (personalInfoItemEntity.getIndex() > 13) {
                TextView child0 = (TextView) view.getChildAt(0);
                personalInfoItemEntity.setText1(getText(child0));
                View child2 = view.getChildAt(2);
                personalInfoItemEntity.setText2(getText((TextView) child2));

            } else {
                TextView child1 = (TextView) view.getChildAt(1);
                personalInfoItemEntity.setText1(getText(child1));

            }

        }
    }


    @Override
    public boolean onBackPressedSupport() {
        if (llAddcustom.getVisibility() == View.VISIBLE) {
            llAddcustom.setVisibility(View.GONE);
            return true;
        } else if (svChose.getVisibility() == View.VISIBLE) {
            svChose.setVisibility(View.GONE);
            return true;
        } else {
            return super.onBackPressedSupport();
        }
    }


}
