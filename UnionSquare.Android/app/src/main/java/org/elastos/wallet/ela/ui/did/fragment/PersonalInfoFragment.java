package org.elastos.wallet.ela.ui.did.fragment;

import android.os.Bundle;
import android.view.View;
import android.widget.EditText;
import android.widget.TextView;

import org.elastos.wallet.R;
import org.elastos.wallet.ela.base.BaseFragment;
import org.elastos.wallet.ela.bean.BusEvent;
import org.elastos.wallet.ela.ui.did.entity.CredentialSubjectBean;
import org.elastos.wallet.ela.ui.did.entity.DIDInfoEntity;
import org.elastos.wallet.ela.ui.vote.bean.Area;
import org.elastos.wallet.ela.ui.vote.fragment.AreaCodeFragment;
import org.elastos.wallet.ela.utils.AppUtlis;
import org.elastos.wallet.ela.utils.CacheUtil;
import org.elastos.wallet.ela.utils.Constant;
import org.elastos.wallet.ela.utils.DateUtil;
import org.elastos.wallet.ela.utils.DialogUtil;
import org.elastos.wallet.ela.utils.RxEnum;
import org.elastos.wallet.ela.utils.SPUtil;
import org.elastos.wallet.ela.utils.listener.WarmPromptListener;
import org.elastos.wallet.ela.utils.widget.TextConfigDataPicker;
import org.elastos.wallet.ela.utils.widget.TextConfigNumberPicker;
import org.greenrobot.eventbus.Subscribe;
import org.greenrobot.eventbus.ThreadMode;

import java.util.Calendar;
import java.util.Date;

import butterknife.BindView;
import butterknife.OnClick;

public class PersonalInfoFragment extends BaseFragment {

    @BindView(R.id.tv_title)
    TextView tvTitle;

    String[] sexs;
    @BindView(R.id.et_name)
    EditText etName;
    @BindView(R.id.et_nick)
    EditText etNick;
    @BindView(R.id.tv_sex)
    TextView tvSex;
    @BindView(R.id.tv_birthday)
    TextView tvBirthday;
    @BindView(R.id.et_headurl)
    EditText etHeadurl;
    @BindView(R.id.et_email)
    EditText etEmail;
    @BindView(R.id.et_code)
    EditText etCode;
    @BindView(R.id.et_phone)
    EditText etPhone;
    @BindView(R.id.tv_area)
    TextView tvArea;
    @BindView(R.id.tv_title_right)
    TextView tvTitleRight;
    @BindView(R.id.tv_next)
    TextView tvNext;
    @BindView(R.id.tv_tip)
    TextView tvTip;
    private DIDInfoEntity didInfo;
    private CredentialSubjectBean credentialSubjectBean;
    private long birthday;
    private String areaCode;

    @Override
    protected int getLayoutId() {
        return R.layout.fragment_personalinfo;
    }

    @Override
    protected void setExtraData(Bundle data) {
        String type = data.getString("type");
        if (Constant.EDITPERSONALINFO.equals(type)) {
            //编辑did  从凭证信息进入
            onAddPersonalInfo(data);
            putData();

        } else if (Constant.ADDPERSONALINFO.equals(type)) {
            //新增did  从凭证信息进入
            onAddPersonalInfo(data);
        } else {
            //从创建did 进入
            tvTitleRight.setVisibility(View.VISIBLE);
            tvTitle.setText(getString(R.string.addpersonalindo));
            didInfo = data.getParcelable("didInfo");
            if (data.getBoolean("useDraft")) {
                //使用草稿
                credentialSubjectBean = CacheUtil.getCredentialSubjectBean(didInfo.getId());
                putData();
            } else {
                credentialSubjectBean = new CredentialSubjectBean(didInfo.getId());
            }
        }
    }

    private void onAddPersonalInfo(Bundle data) {
        credentialSubjectBean = data.getParcelable("CredentialSubjectBean");
        tvTitle.setText(getString(R.string.editpersonalinfo));
        tvTip.setVisibility(View.GONE);
        tvTitleRight.setVisibility(View.GONE);
        tvNext.setText(getString(R.string.keep));
        tvNext.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                setData();
                CacheUtil.setCredentialSubjectBean(credentialSubjectBean);
                popBackFragment();
            }
        });
    }

    @Override
    protected void initView(View view) {


        sexs = new String[]{getString(R.string.man), getString(R.string.woman)};
    }


    @OnClick({R.id.rl_selectsex, R.id.rl_selectbirthday, R.id.rl_selectarea, R.id.tv_title_right, R.id.tv_next})
    public void onViewClicked(View view) {
        switch (view.getId()) {
            case R.id.rl_selectsex:
                new DialogUtil().showCommonSelect(getBaseActivity(), sexs, new WarmPromptListener() {
                    @Override
                    public void affireBtnClick(View view) {
                        tvSex.setText(sexs[((TextConfigNumberPicker) view).getValue()]);
                    }
                });
                break;
            case R.id.rl_selectbirthday:
                Calendar calendar = Calendar.getInstance();
                long minData = calendar.getTimeInMillis();
                int year = calendar.get(Calendar.YEAR);
                calendar.set(Calendar.YEAR, year - 100);
                new DialogUtil().showTime(getBaseActivity(), calendar.getTimeInMillis(), minData, new WarmPromptListener() {
                    @Override
                    public void affireBtnClick(View view) {
                        String date = ((TextConfigDataPicker) view).getYear() + "-" + (((TextConfigDataPicker) view).getMonth() + 1)
                                + "-" + ((TextConfigDataPicker) view).getDayOfMonth();
                        birthday = DateUtil.parseToLong(date) / 1000L;

                        tvBirthday.setText(DateUtil.timeNYR(birthday, getContext()));
                    }
                });
                break;
            case R.id.rl_selectarea:
                start(AreaCodeFragment.class);
                registReceiver();
                break;
            case R.id.tv_title_right:
            case R.id.tv_next:
                setData();
                Bundle bundle = new Bundle();
                bundle.putParcelable("credentialSubjectBean", credentialSubjectBean);
                bundle.putAll(getArguments());
                start(PersonalIntroFragment.class, bundle);
                break;
        }
    }

    private void putData() {
        CredentialSubjectBean.Info info = credentialSubjectBean.getInfo();
        if (info != null) {
            etName.setText(info.getName());
            etNick.setText(info.getNickname());
            tvSex.setText((info.getGender() == null) ? null : (info.getGender().equals("male") ? getString(R.string.man) : getString(R.string.woman)));
            birthday = info.getBirthday();
            tvBirthday.setText(DateUtil.timeNYR(birthday, getContext()));
            etHeadurl.setText(info.getAvatar());
            etEmail.setText(info.getEmail());
            etCode.setText(info.getPhoneCode());
            etPhone.setText(info.getPhone());
            tvArea.setText(AppUtlis.getLoc(getContext(), info.getNation()));
        }
    }

    private void setData() {
        CredentialSubjectBean.Info info = credentialSubjectBean.getInfo();
        if (info == null) {
            info = new CredentialSubjectBean.Info();

        }
        info.setEditTime(new Date().getTime() / 1000);
        info.setName(getText(etName));
        info.setNickname(getText(etNick));
        info.setGender(null == (getText(tvSex)) ? null : (getString(R.string.man).equals(getText(tvSex)) ? "male" : "female"));
        info.setBirthday(birthday);
        info.setAvatar(getText(etHeadurl));
        info.setEmail(getText(etEmail));
        info.setPhoneCode(getText(etCode));
        info.setPhone(getText(etPhone));
        info.setNation(areaCode);
        if (info.isEmpty()) {
            credentialSubjectBean.setInfo(null);
        } else {
            info.setEditTime(new Date().getTime() / 1000);
            credentialSubjectBean.setInfo(info);
        }


    }

    @Override
    public boolean onBackPressedSupport() {
        setData();
        post(RxEnum.RETURCER.ordinal(), "", credentialSubjectBean);
        return super.onBackPressedSupport();


    }

    @Subscribe(threadMode = ThreadMode.MAIN)
    public void Event(BusEvent result) {
        int integer = result.getCode();
        if (integer == RxEnum.AREA.ordinal()) {
            Area area = (Area) result.getObj();
            areaCode = area.getCode() + "";
            int Language = new SPUtil(getContext()).getLanguage();
            String name;
            if (Language == 0) {
                name = area.getZh();
            } else {
                name = area.getEn();
            }
            tvArea.setText(name);
        }
    }

}
