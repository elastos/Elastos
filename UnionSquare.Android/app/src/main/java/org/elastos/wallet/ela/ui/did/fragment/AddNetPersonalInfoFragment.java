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
import org.elastos.wallet.ela.ElaWallet.MyWallet;
import org.elastos.wallet.ela.base.BaseFragment;
import org.elastos.wallet.ela.bean.BusEvent;
import org.elastos.wallet.ela.db.table.Wallet;
import org.elastos.wallet.ela.rxjavahelp.BaseEntity;
import org.elastos.wallet.ela.rxjavahelp.NewBaseViewData;
import org.elastos.wallet.ela.ui.common.listener.CommonRvListener1;
import org.elastos.wallet.ela.ui.did.adapter.PersonalAddRecAdapetr;
import org.elastos.wallet.ela.ui.did.adapter.PersonalChoseRecAdapetr;
import org.elastos.wallet.ela.ui.did.entity.CredentialSubjectBean;
import org.elastos.wallet.ela.ui.did.entity.PersonalInfoItemEntity;
import org.elastos.wallet.ela.ui.did.presenter.DIDUIPresenter;
import org.elastos.wallet.ela.ui.vote.activity.VoteTransferActivity;
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

public class AddNetPersonalInfoFragment extends BaseFragment implements CommonRvListener1, NewBaseViewData {

    @BindView(R.id.tv_title)
    TextView tvTitle;
    @BindView(R.id.rl_custom)
    RelativeLayout rlCustom;
    @BindView(R.id.ll_addcustom)
    LinearLayout llAddcustom;

    String[] sexs;

    @BindView(R.id.tv_title_right)
    TextView tvTitleRight;
    @BindView(R.id.iv_add)
    ImageView ivAdd;
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
    private List<PersonalInfoItemEntity> listShow;//同时管理自定义信息 最多管理5条
    private List<PersonalInfoItemEntity> listChose;//由于可以最多5个自定义信息 1对多 所有这里不会管理自定义信息
    private PersonalAddRecAdapetr adapterShow;
    private PersonalChoseRecAdapetr adapterChose;
    private TextView curentTextView;//需要在别的页面获取数据的view
    private DIDUIPresenter diduiPresenter;
    private String type;

    @Override
    protected int getLayoutId() {
        return R.layout.fragment_netpersonalinfo;
    }


    @Override
    protected void setExtraData(Bundle data) {

        type = data.getString("type");
        if (Constant.DIDUPDEATE.equals(type)) {
            //新增did凭证  从凭证信息进入
            tvTitleRight.setText(getString(R.string.publish));
            onAddPartCredential(data);
        } else {
            //新增did凭证  从新增did进入
            wallet = data.getParcelable("wallet");
            didName = data.getString("didName");
            didEndDate = (Date) data.getSerializable("didEndDate");
            tvTitleRight.setText(getString(R.string.next));
        }
    }

    /**
     * 发布   重写方法
     *
     * @param data
     */
    private void onAddPartCredential(Bundle data) {
        tvTitleRight.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                new DialogUtil().showCommonWarmPrompt(getBaseActivity(), getString(R.string.whetherpublishdid), null, null, false, new WarmPromptListener() {
                    @Override
                    public void affireBtnClick(View view) {
                        storePersonalInfo();
                        String didName = getMyDID().getName(getMyDID().getDIDDocument());
                        CredentialSubjectBean netCredentialSubjectBean = new DIDUIPresenter().convertCredentialSubjectBean(AddNetPersonalInfoFragment.this, didName, listShow);
                        Log.i("??", JSON.toJSONString(netCredentialSubjectBean));
                        DIDDocument doc = getMyDID().getDIDDocument();
                        Date didEndDate = getMyDID().getExpires(doc);
                        Wallet wallet = data.getParcelable("wallet");
                        Intent intent = new Intent(getActivity(), VoteTransferActivity.class);
                        intent.putExtra("didName", didName);
                        intent.putExtra("didEndDate", didEndDate);
                        intent.putExtra("wallet", wallet);
                        intent.putExtra("netCredentialSubjectBean", netCredentialSubjectBean);
                        intent.putExtra("chainId", MyWallet.IDChain);
                        intent.putExtra("fee", 20000L);
                        intent.putExtra("type", Constant.DIDUPDEATE);
                        intent.putExtra("transType", 10);
                        startActivity(intent);
                    }
                });
            }
        });
    }

    @Override
    protected void initView(View view) {
        tvTitle.setText(R.string.adddidinfo);
        tvTitleRight.setVisibility(View.VISIBLE);
        diduiPresenter = new DIDUIPresenter();
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
            if (i == 2 || i == 3 || i == 4 || i == 6 || i == 8 || i == 10 || i == 12)
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
                //下一步
                curentTextView = null;//防止下一页的对本页curentTextView有影响
                storePersonalInfo();
                credentialSubjectBean = diduiPresenter.convertCredentialSubjectBean(AddNetPersonalInfoFragment.this, didName, listShow);
                Log.i("??", JSON.toJSONString(credentialSubjectBean));
                Bundle bundle = getArguments();
                bundle.putParcelable("netCredentialSubjectBean", credentialSubjectBean);
                start(AddPersonalInfoFragment.class, bundle);
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
            if (curentTextView == null) {
                return;
            }
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
            if (curentTextView == null) {
                return;
            }
            String intro = (String) result.getObj();
            curentTextView.setText(intro);
        }
        if (integer == RxEnum.TRANSFERSUCESS.ordinal()) {
            new DialogUtil().showTransferSucess(getBaseActivity(), new WarmPromptListener() {
                @Override
                public void affireBtnClick(View view) {
                    if (Constant.DIDUPDEATE.equals(type))
                        toDIDDetailFragment();
                    else
                        toMainFragment();
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
            //通过右边的iv类型判断是条目的增减还是特殊条目数据填充
            if (v instanceof ImageView) {
                //去除某一项 数据会重新渲染
                storePersonalInfo();
                if (personalInfoItemEntity.getIndex() <= 13) {
                    //自定义项目
                    personalInfoItemEntity.setText1(null);
                    personalInfoItemEntity.setText2(null);
                    listChose.add(personalInfoItemEntity);
                }

                listShow.remove(personalInfoItemEntity);
                setRecycleViewShow();
                // adapterShow.notifyItemRemoved(position);//加动画
                //adapterShow.notifyItemRangeChanged(position, listShow.size() - position);
                if (ivAdd.getVisibility() == View.GONE) {
                    ivAdd.setVisibility(View.VISIBLE);
                }
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
     * //遍历所有show的对象,保存数据同时可以防止再次渲染rv时数据混乱
     */
    private void storePersonalInfo() {
        for (int i = 0; i < listShow.size(); i++) {
            PersonalInfoItemEntity personalInfoItemEntity = listShow.get(i);
            ViewGroup view = (ViewGroup) (rvShow.getLayoutManager().findViewByPosition(i));//一次获得rv的每一个item
            if (view == null) {
                break;
            }
            TextView child0 = (TextView) view.getChildAt(0);

            personalInfoItemEntity.setText1(getText(child0));
            if (personalInfoItemEntity.getIndex() == 5 ||
                    (personalInfoItemEntity.getIndex() > 13)) {
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
