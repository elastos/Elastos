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

package org.elastos.wallet.ela.ui.crvote.fragment;

import android.os.Bundle;
import android.support.v7.widget.LinearLayoutManager;
import android.support.v7.widget.RecyclerView;
import android.view.View;
import android.widget.TextView;

import org.elastos.wallet.R;
import org.elastos.wallet.ela.base.BaseFragment;
import org.elastos.wallet.ela.ui.common.listener.CommonRvListener1;
import org.elastos.wallet.ela.ui.did.adapter.PersonalShowRecAdapetr;
import org.elastos.wallet.ela.ui.did.entity.CredentialSubjectBean;
import org.elastos.wallet.ela.ui.did.entity.PersonalInfoItemEntity;
import org.elastos.wallet.ela.ui.did.fragment.ShowPersonalIntroFragemnt;
import org.elastos.wallet.ela.utils.AppUtlis;
import org.elastos.wallet.ela.utils.DateUtil;

import java.util.ArrayList;
import java.util.Iterator;

import butterknife.BindView;

public class CredentialInfoFragemnt extends BaseFragment {
    @BindView(R.id.tv_title)
    TextView tvTitle;
    @BindView(R.id.rv_show)
    RecyclerView rvShow;
    @BindView(R.id.tv_didname)
    TextView tvDidname;
    private ArrayList<PersonalInfoItemEntity> listShow;
    private CredentialSubjectBean credentialSubjectBean;

    @Override
    protected int getLayoutId() {
        return R.layout.fragment_cr_credencial;
    }

    @Override
    protected void setExtraData(Bundle data) {
        credentialSubjectBean = data.getParcelable("credentialSubjectBean");
    }

    @Override
    protected void initView(View view) {
        tvTitle.setText(getString(R.string.personalzl));

        initItemDate();
        tvDidname.setText(credentialSubjectBean.getDidName());
    }

    private void initItemDate() {
        String showData[] = getResources().getStringArray(R.array.personalinfo_chose);
        listShow = new ArrayList<>();

        for (int i = 0; i < showData.length; i++) {
            PersonalInfoItemEntity personalInfoItemEntity = new PersonalInfoItemEntity();
            personalInfoItemEntity.setIndex(i);
            personalInfoItemEntity.setHintChose(showData[i]);
            personalInfoItemEntity.setHintShow1(showData[i]);
            if (i == 5) {
                personalInfoItemEntity.setHintShow1(getString(R.string.phonecode));
                personalInfoItemEntity.setHintShow2(getString(R.string.phonenumber));
            }
            listShow.add(personalInfoItemEntity);
        }
        convertCredentialSubjectBean();
        setRecycleViewShow();

    }

    /**
     * 将CredentialSubjectBean的数据转换到listShow
     *
     * @return
     */
    private void convertCredentialSubjectBean() {
        Iterator<PersonalInfoItemEntity> iterator = listShow.iterator();
        while (iterator.hasNext()) {
            //只遍历show的数据
            PersonalInfoItemEntity personalInfoItemEntity = iterator.next();
            int index = personalInfoItemEntity.getIndex();
            switch (index) {
                case 0:
                    resetShowList(iterator, personalInfoItemEntity, credentialSubjectBean.getNickname());
                    break;
                case 1:
                    String gender = credentialSubjectBean.getGender();
                    if ("1".equals(gender))
                        gender = getString(R.string.man);
                    else if ("2".equals(gender))
                        gender = getString(R.string.woman);
                    resetShowList(iterator, personalInfoItemEntity, gender);
                    break;
                case 2:
                    String birthday = credentialSubjectBean.getBirthday();
                    String birthDate = DateUtil.timeNYR(birthday, getContext(), true);
                    resetShowList(iterator, personalInfoItemEntity, birthDate);
                    break;

                case 4:
                    resetShowList(iterator, personalInfoItemEntity, credentialSubjectBean.getEmail());
                    break;
                case 5:
                    resetShowList(iterator, personalInfoItemEntity, credentialSubjectBean.getPhoneCode(), credentialSubjectBean.getPhone());
                    break;
                case 6:
                    String areaCode = credentialSubjectBean.getNation();
                    resetShowList(iterator, personalInfoItemEntity, AppUtlis.getLoc(getContext(), areaCode));
                    break;
                case 3:
                    // resetShowList(iterator, personalInfoItemEntity, credentialSubjectBean.getAvatar());
                case 7:

                    // resetShowList(iterator, personalInfoItemEntity, credentialSubjectBean.getIntroduction());
                    iterator.remove();
                    break;
                case 8:
                    resetShowList(iterator, personalInfoItemEntity, credentialSubjectBean.getHomePage());
                    break;
                case 9:
                    resetShowList(iterator, personalInfoItemEntity, credentialSubjectBean.getWechat());
                    break;
                case 10:
                    resetShowList(iterator, personalInfoItemEntity, credentialSubjectBean.getTwitter());
                    break;
                case 11:
                    resetShowList(iterator, personalInfoItemEntity, credentialSubjectBean.getWeibo());
                    break;
                case 12:
                    resetShowList(iterator, personalInfoItemEntity, credentialSubjectBean.getFacebook());
                    break;
                case 13:
                    resetShowList(iterator, personalInfoItemEntity, credentialSubjectBean.getGoogleAccount());
                    break;
            }

        }
    }

    private void setRecycleViewShow() {
        PersonalShowRecAdapetr adapterShow = new PersonalShowRecAdapetr(getContext(), listShow);
        rvShow.setLayoutManager(new LinearLayoutManager(getContext(), LinearLayoutManager.VERTICAL, false));
        adapterShow.setCommonRvListener(new CommonRvListener1() {
            @Override
            public void onRvItemClick(View view, int position, Object o) {
                PersonalInfoItemEntity personalInfoItemEntity = (PersonalInfoItemEntity) o;
                if (personalInfoItemEntity.getIndex() == 7) {
                    //去个人简介详情
                    Bundle bundle = new Bundle();
                    bundle.putString("personalIntro", personalInfoItemEntity.getText1());
                    start(ShowPersonalIntroFragemnt.class, bundle);
                }
            }
        });
        rvShow.setAdapter(adapterShow);
    }

    private void resetShowList(Iterator<PersonalInfoItemEntity> iterator, PersonalInfoItemEntity personalInfoItemEntity, String text1) {
        if (text1 == null) {
            iterator.remove();
        } else {
            personalInfoItemEntity.setText1(text1);
        }
    }

    private void resetShowList(Iterator<PersonalInfoItemEntity> iterator, PersonalInfoItemEntity personalInfoItemEntity, String text1, String text2) {
        if (text1 == null && text2 == null) {
            iterator.remove();
        } else {
            personalInfoItemEntity.setText1(text1);
            personalInfoItemEntity.setText2(text2);
        }
    }
}
