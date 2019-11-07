package org.elastos.wallet.ela.ui.did.entity;

import android.os.Parcel;
import android.os.Parcelable;
import android.text.TextUtils;

public class CredentialSubjectBean implements Parcelable {
    private String did;
    private Info info;
    private Intro intro;
    private Social social;


    public static class Info implements Parcelable {
        private String name;
        private String nickname;
        private String gender;
        private long birthday;
        private String avatar;
        private String nation;//使用area code
        private String email;
        private String phone;
        private String phoneCode;
        private long editTime;

        public boolean isEmpty() {
            return (TextUtils.isEmpty(name) && TextUtils.isEmpty(nickname) && TextUtils.isEmpty(gender) && birthday == 0
                    && TextUtils.isEmpty(avatar) && TextUtils.isEmpty(nation) && TextUtils.isEmpty(email)
                    && TextUtils.isEmpty(phone) && TextUtils.isEmpty(phoneCode) && editTime == 0);
        }

        public String getName() {
            return name;
        }

        public void setName(String name) {
            this.name = name;
        }

        public String getNickname() {
            return nickname;
        }

        public void setNickname(String nickname) {
            this.nickname = nickname;
        }

        public String getGender() {
            return gender;
        }

        public void setGender(String gender) {
            this.gender = gender;
        }

        public long getBirthday() {
            return birthday;
        }

        public void setBirthday(long birthday) {
            this.birthday = birthday;
        }

        public String getAvatar() {
            return avatar;
        }

        public void setAvatar(String avatar) {
            this.avatar = avatar;
        }

        public String getNation() {
            return nation;
        }

        public void setNation(String nation) {
            this.nation = nation;
        }

        public String getEmail() {
            return email;
        }

        public void setEmail(String email) {
            this.email = email;
        }

        public String getPhone() {
            return phone;
        }

        public void setPhone(String phone) {
            this.phone = phone;
        }

        public String getPhoneCode() {
            return phoneCode;
        }

        public void setPhoneCode(String phoneCode) {
            this.phoneCode = phoneCode;
        }

        public long getEditTime() {
            return editTime;
        }

        public void setEditTime(long editTime) {
            this.editTime = editTime;
        }

        @Override
        public int describeContents() {
            return 0;
        }

        @Override
        public void writeToParcel(Parcel dest, int flags) {
            dest.writeString(this.name);
            dest.writeString(this.nickname);
            dest.writeString(this.gender);
            dest.writeLong(this.birthday);
            dest.writeString(this.avatar);
            dest.writeString(this.nation);
            dest.writeString(this.email);
            dest.writeString(this.phone);
            dest.writeString(this.phoneCode);
            dest.writeLong(this.editTime);
        }

        public Info() {
        }

        protected Info(Parcel in) {
            this.name = in.readString();
            this.nickname = in.readString();
            this.gender = in.readString();
            this.birthday = in.readLong();
            this.avatar = in.readString();
            this.nation = in.readString();
            this.email = in.readString();
            this.phone = in.readString();
            this.phoneCode = in.readString();
            this.editTime = in.readLong();
        }

        public static final Creator<Info> CREATOR = new Creator<Info>() {
            @Override
            public Info createFromParcel(Parcel source) {
                return new Info(source);
            }

            @Override
            public Info[] newArray(int size) {
                return new Info[size];
            }
        };
    }

    public static class Intro implements Parcelable {
        private String introduction;
        private long editTime;

        public String getIntroduction() {
            return introduction;
        }

        public void setIntroduction(String introduction) {
            this.introduction = introduction;
        }

        public long getEditTime() {
            return editTime;
        }

        public void setEditTime(long editTime) {
            this.editTime = editTime;
        }

        @Override
        public int describeContents() {
            return 0;
        }

        @Override
        public void writeToParcel(Parcel dest, int flags) {
            dest.writeString(this.introduction);
            dest.writeLong(this.editTime);
        }

        public Intro() {
        }

        protected Intro(Parcel in) {
            this.introduction = in.readString();
            this.editTime = in.readLong();
        }

        public static final Creator<Intro> CREATOR = new Creator<Intro>() {
            @Override
            public Intro createFromParcel(Parcel source) {
                return new Intro(source);
            }

            @Override
            public Intro[] newArray(int size) {
                return new Intro[size];
            }
        };
    }

    public static class Social implements Parcelable {
        private String alipay;
        private String facebook;
        private String googleAccount;
        private String homePage;
        private String microsoftPassport;
        private String twitter;
        private String wechat;
        private String weibo;
        private long editTime;
        public boolean isEmpty() {
            return (TextUtils.isEmpty(alipay) && TextUtils.isEmpty(facebook) && TextUtils.isEmpty(googleAccount)
                    && TextUtils.isEmpty(homePage) && TextUtils.isEmpty(microsoftPassport) && TextUtils.isEmpty(twitter)
                    && TextUtils.isEmpty(wechat) && TextUtils.isEmpty(weibo) && editTime == 0);
        }
        public String getAlipay() {
            return alipay;
        }

        public void setAlipay(String alipay) {
            this.alipay = alipay;
        }

        public String getFacebook() {
            return facebook;
        }

        public void setFacebook(String facebook) {
            this.facebook = facebook;
        }

        public String getGoogleAccount() {
            return googleAccount;
        }

        public void setGoogleAccount(String googleAccount) {
            this.googleAccount = googleAccount;
        }

        public String getHomePage() {
            return homePage;
        }

        public void setHomePage(String homePage) {
            this.homePage = homePage;
        }

        public String getMicrosoftPassport() {
            return microsoftPassport;
        }

        public void setMicrosoftPassport(String microsoftPassport) {
            this.microsoftPassport = microsoftPassport;
        }

        public String getTwitter() {
            return twitter;
        }

        public void setTwitter(String twitter) {
            this.twitter = twitter;
        }

        public String getWechat() {
            return wechat;
        }

        public void setWechat(String wechat) {
            this.wechat = wechat;
        }

        public String getWeibo() {
            return weibo;
        }

        public void setWeibo(String weibo) {
            this.weibo = weibo;
        }

        public long getEditTime() {
            return editTime;
        }

        public void setEditTime(long editTime) {
            this.editTime = editTime;
        }

        @Override
        public int describeContents() {
            return 0;
        }

        @Override
        public void writeToParcel(Parcel dest, int flags) {
            dest.writeString(this.alipay);
            dest.writeString(this.facebook);
            dest.writeString(this.googleAccount);
            dest.writeString(this.homePage);
            dest.writeString(this.microsoftPassport);
            dest.writeString(this.twitter);
            dest.writeString(this.wechat);
            dest.writeString(this.weibo);
            dest.writeLong(this.editTime);
        }

        public Social() {
        }

        protected Social(Parcel in) {
            this.alipay = in.readString();
            this.facebook = in.readString();
            this.googleAccount = in.readString();
            this.homePage = in.readString();
            this.microsoftPassport = in.readString();
            this.twitter = in.readString();
            this.wechat = in.readString();
            this.weibo = in.readString();
            this.editTime = in.readLong();
        }

        public static final Creator<Social> CREATOR = new Creator<Social>() {
            @Override
            public Social createFromParcel(Parcel source) {
                return new Social(source);
            }

            @Override
            public Social[] newArray(int size) {
                return new Social[size];
            }
        };
    }

    public String getDid() {
        return did;
    }

    public void setDid(String did) {
        this.did = did;
    }

    public Info getInfo() {
        return info;
    }

    public void setInfo(Info info) {
        this.info = info;
    }

    public Intro getIntro() {
        return intro;
    }

    public void setIntro(Intro intro) {
        this.intro = intro;
    }

    public Social getSocial() {
        return social;
    }

    public void setSocial(Social social) {
        this.social = social;
    }

    @Override
    public int describeContents() {
        return 0;
    }

    @Override
    public void writeToParcel(Parcel dest, int flags) {
        dest.writeString(this.did);
        dest.writeParcelable(this.info, flags);
        dest.writeParcelable(this.intro, flags);
        dest.writeParcelable(this.social, flags);
    }

    public CredentialSubjectBean() {
    }

    protected CredentialSubjectBean(Parcel in) {
        this.did = in.readString();
        this.info = in.readParcelable(Info.class.getClassLoader());
        this.intro = in.readParcelable(Intro.class.getClassLoader());
        this.social = in.readParcelable(Social.class.getClassLoader());
    }

    public static final Parcelable.Creator<CredentialSubjectBean> CREATOR = new Parcelable.Creator<CredentialSubjectBean>() {
        @Override
        public CredentialSubjectBean createFromParcel(Parcel source) {
            return new CredentialSubjectBean(source);
        }

        @Override
        public CredentialSubjectBean[] newArray(int size) {
            return new CredentialSubjectBean[size];
        }
    };
}

