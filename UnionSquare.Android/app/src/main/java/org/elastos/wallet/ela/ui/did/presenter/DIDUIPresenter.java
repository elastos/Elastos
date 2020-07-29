package org.elastos.wallet.ela.ui.did.presenter;

import org.elastos.wallet.R;
import org.elastos.wallet.ela.base.BaseFragment;
import org.elastos.wallet.ela.ui.did.entity.CredentialSubjectBean;
import org.elastos.wallet.ela.ui.did.entity.PersonalInfoItemEntity;
import org.elastos.wallet.ela.utils.AppUtlis;
import org.elastos.wallet.ela.utils.DateUtil;

import java.util.ArrayList;
import java.util.Iterator;

public class DIDUIPresenter {
    /**
     * 对list <PersonalInfoItemEntity>的iterator中的内容赋值或删除
     * @param iterator
     * @param personalInfoItemEntity
     * @param text1
     */
    public void resetShowList(Iterator<PersonalInfoItemEntity> iterator, PersonalInfoItemEntity personalInfoItemEntity, String text1) {
        if (text1 == null) {
            iterator.remove();
        } else {
            personalInfoItemEntity.setText1(text1);
        }
    }

    public void resetShowList(Iterator<PersonalInfoItemEntity> iterator, PersonalInfoItemEntity personalInfoItemEntity, String text1, String text2) {
        if (text1 == null && text2 == null) {
            iterator.remove();
        } else {
            personalInfoItemEntity.setText1(text1);
            personalInfoItemEntity.setText2(text2);
        }
    }
    /**
     * 将CredentialSubjectBean的数据转换到listShow
     *
     * @return
     */
    public void convertCredentialSubjectBean(BaseFragment fragment,ArrayList<PersonalInfoItemEntity> listShow, CredentialSubjectBean credentialSubjectBean) {
       if (credentialSubjectBean==null){
           listShow.clear();
       }
        ArrayList<CredentialSubjectBean.CustomInfo> infos = credentialSubjectBean.getCustomInfos();
        if (infos != null) {
            for (CredentialSubjectBean.CustomInfo info : infos) {
                PersonalInfoItemEntity personalInfoItemEntity = new PersonalInfoItemEntity();
                personalInfoItemEntity.setIndex(listShow.size());
                personalInfoItemEntity.setText1(info.getTitle());
                personalInfoItemEntity.setText2(info.getContent());
                int type = info.getType();
                personalInfoItemEntity.setType(type);
                personalInfoItemEntity.setHintShow2(fragment.getString(R.string.plzinputcontent));
                if (type == -1) {
                    personalInfoItemEntity.setHintShow1(fragment.getString(R.string.singletextitem));

                } else if (type == -2) {
                    personalInfoItemEntity.setHintShow1(fragment.getString(R.string.mutiltextitem));
                }

                listShow.add(personalInfoItemEntity);

            }
        }
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
                        gender = fragment.getString(R.string.man);
                    else if ("2".equals(gender))
                        gender = fragment.getString(R.string.woman);
                    resetShowList(iterator, personalInfoItemEntity, gender);
                    break;
                case 2:
                    String birthday = credentialSubjectBean.getBirthday();
                    String birthDate = DateUtil.timeNYR(birthday, fragment.getContext(), true);
                    resetShowList(iterator, personalInfoItemEntity, birthDate);
                    break;
                case 3:
                    resetShowList(iterator, personalInfoItemEntity, credentialSubjectBean.getAvatar());
                    break;
                case 4:
                    resetShowList(iterator, personalInfoItemEntity, credentialSubjectBean.getEmail());
                    break;
                case 5:
                    resetShowList(iterator, personalInfoItemEntity, credentialSubjectBean.getPhoneCode(), credentialSubjectBean.getPhone());
                    break;
                case 6:
                    String areaCode = credentialSubjectBean.getNation();
                    resetShowList(iterator, personalInfoItemEntity, AppUtlis.getLoc(fragment.getContext(), areaCode));
                    break;
                case 7:

                    resetShowList(iterator, personalInfoItemEntity, credentialSubjectBean.getIntroduction());

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
}
