package org.elastos.wallet.ela.ui.did.entity;

import android.os.Parcel;
import android.os.Parcelable;
import android.support.annotation.Nullable;

import org.elastos.wallet.ela.rxjavahelp.BaseEntity;

import java.io.Serializable;
import java.util.ArrayList;
import java.util.List;

public class DIDInfoEntity extends BaseEntity implements Parcelable, Serializable {
    @Override
    public boolean equals(@Nullable Object other) {
        return this.getId().equals(((DIDInfoEntity) other).getId());

    }

    @Override
    public int hashCode() {

        int result = 17;
        result = 31 * result + id.hashCode();
        return result;
    }

    /**
     * id : innnNZJLqmJ8uKfVHKFxhdqVtvipNHzmZs
     * operation : create
     * status : "Pending
     * publicKey : [{"id":"#primary","publicKey":"031f7a5a6bf3b2450cd9da4048d00a8ef1cb4912b5057535f65f3cc0e0c36f13b4"},{"id":"#recovery","controller":"ip7ntDo2metGnU8wGP4FnyKCUdbHm4BPDh","publicKey":"03d25d582c485856520c501b2e2f92934eda0232ded70cad9e51cf13968cac22cc"}]
     * credentialSubject : {"id":"innnNZJLqmJ8uKfVHKFxhdqVtvipNHzmZs","name":"H60CZ","nickname":"jHo8AB","alipay":"alipay@223.com","avatar":"img.jpg","birthday":"2019-10-12","descript":"this is simple descript","email":"test@test.com","facebook":"facebook","gender":"male","googleAccount":"google@google.com","homePage":"homePage","microsoftPassport":"MicrosoftPassport","nation":"china","code":"86","phone":"13032454523","twitter":"twitter","wechat":"wechat2333","weibo":"test@sina.com"}
     * expires : 2024-02-10T17:00:00Z
     */
    private long issuanceDate;//编辑时间
    private String id;
    private String didName;
    private String walletId;
    private String operation;//1.创建：create 2.更新，修改：update 3.注销：deactivate
    private String status;//Pending 确认中   Confirmed已确认  Unpublished 未发布(草稿  这个api不提供,保存草稿时候自己设置)
    private CredentialSubjectBean credentialSubject;
    private long expires;
    private List<PublicKeyBean> publicKey;


    public long getIssuanceDate() {
        return issuanceDate;
    }

    public void setIssuanceDate(long issuanceDate) {
        this.issuanceDate = issuanceDate;
    }

    public String getId() {
        return id;
    }

    public void setId(String id) {
        this.id = id;
    }
    public String getDidName() {
        return didName;
    }

    public void setDidName(String didName) {
        this.didName = didName;
    }
    public String getWalletId() {
        return walletId;
    }

    public void setWalletId(String walletId) {
        this.walletId = walletId;
    }

    public String getOperation() {
        return operation;
    }

    public void setOperation(String operation) {
        this.operation = operation;
    }

    public String getStatus() {
        return status;
    }

    public void setStatus(String status) {
        this.status = status;
    }

    public CredentialSubjectBean getCredentialSubject() {
        return credentialSubject;
    }

    public void setCredentialSubject(CredentialSubjectBean credentialSubject) {
        this.credentialSubject = credentialSubject;
    }

    public long getExpires() {
        return expires;
    }

    public void setExpires(long expires) {
        this.expires = expires;
    }

    public List<PublicKeyBean> getPublicKey() {
        return publicKey;
    }

    public void setPublicKey(List<PublicKeyBean> publicKey) {
        this.publicKey = publicKey;
    }

    public static class CredentialSubjectBean implements Parcelable, Serializable {
        /**
         * id : innnNZJLqmJ8uKfVHKFxhdqVtvipNHzmZs
         * name : H60CZ
         * nickname : jHo8AB
         * alipay : alipay@223.com
         * avatar : img.jpg
         * birthday : 2019-10-12
         * descript : this is simple descript
         * email : test@test.com
         * facebook : facebook
         * gender : male
         * googleAccount : google@google.com
         * homePage : homePage
         * microsoftPassport : MicrosoftPassport
         * nation : china
         * phone : +8613032454523
         * code : 86
         * twitter : twitter
         * wechat : wechat2333
         * weibo : test@sina.com
         */

        private String name;
        private String nickname;
        private String alipay;
        private String avatar;
        private long birthday;
        private String descript;
        private String email;
        private String facebook;
        private String gender;
        private String googleAccount;
        private String homePage;
        private String microsoftPassport;
        private String nation;//使用area code
        private String phone;
        private String code;//新增
        private String twitter;
        private String wechat;
        private String weibo;
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

        public String getAlipay() {
            return alipay;
        }

        public void setAlipay(String alipay) {
            this.alipay = alipay;
        }

        public String getAvatar() {
            return avatar;
        }

        public void setAvatar(String avatar) {
            this.avatar = avatar;
        }

        public long getBirthday() {
            return birthday;
        }

        public void setBirthday(long birthday) {
            this.birthday = birthday;
        }

        public String getDescript() {
            return descript;
        }

        public void setDescript(String descript) {
            this.descript = descript;
        }

        public String getEmail() {
            return email;
        }

        public void setEmail(String email) {
            this.email = email;
        }

        public String getFacebook() {
            return facebook;
        }

        public void setFacebook(String facebook) {
            this.facebook = facebook;
        }

        public String getGender() {
            return gender;
        }

        public void setGender(String gender) {
            this.gender = gender;
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

        public String getNation() {
            return nation;
        }

        public void setNation(String nation) {
            this.nation = nation;
        }

        public String getPhone() {
            return phone;
        }

        public void setPhone(String phone) {
            this.phone = phone;
        }

        public String getCode() {
            return code;
        }

        public void setCode(String code) {
            this.code = code;
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

        @Override
        public int describeContents() {
            return 0;
        }

        @Override
        public void writeToParcel(Parcel dest, int flags) {
            dest.writeString(this.name);
            dest.writeString(this.nickname);
            dest.writeString(this.alipay);
            dest.writeString(this.avatar);
            dest.writeLong(this.birthday);
            dest.writeString(this.descript);
            dest.writeString(this.email);
            dest.writeString(this.facebook);
            dest.writeString(this.gender);
            dest.writeString(this.googleAccount);
            dest.writeString(this.homePage);
            dest.writeString(this.microsoftPassport);
            dest.writeString(this.nation);
            dest.writeString(this.phone);
            dest.writeString(this.code);
            dest.writeString(this.twitter);
            dest.writeString(this.wechat);
            dest.writeString(this.weibo);
        }

        public CredentialSubjectBean() {
        }

        protected CredentialSubjectBean(Parcel in) {
            this.name = in.readString();
            this.nickname = in.readString();
            this.alipay = in.readString();
            this.avatar = in.readString();
            this.birthday = in.readLong();
            this.descript = in.readString();
            this.email = in.readString();
            this.facebook = in.readString();
            this.gender = in.readString();
            this.googleAccount = in.readString();
            this.homePage = in.readString();
            this.microsoftPassport = in.readString();
            this.nation = in.readString();
            this.phone = in.readString();
            this.code = in.readString();
            this.twitter = in.readString();
            this.wechat = in.readString();
            this.weibo = in.readString();
        }

        public static final Creator<CredentialSubjectBean> CREATOR = new Creator<CredentialSubjectBean>() {
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

    public static class PublicKeyBean implements Parcelable , Serializable{
        /**
         * id : #primary
         * publicKey : 031f7a5a6bf3b2450cd9da4048d00a8ef1cb4912b5057535f65f3cc0e0c36f13b4
         * controller : ip7ntDo2metGnU8wGP4FnyKCUdbHm4BPDh
         */

        private String id;
        private String publicKey;
        private String controller;

        public String getId() {
            return id;
        }

        public void setId(String id) {
            this.id = id;
        }

        public String getPublicKey() {
            return publicKey;
        }

        public void setPublicKey(String publicKey) {
            this.publicKey = publicKey;
        }

        public String getController() {
            return controller;
        }

        public void setController(String controller) {
            this.controller = controller;
        }

        @Override
        public int describeContents() {
            return 0;
        }

        @Override
        public void writeToParcel(Parcel dest, int flags) {
            dest.writeString(this.id);
            dest.writeString(this.publicKey);
            dest.writeString(this.controller);
        }

        public PublicKeyBean() {
        }

        protected PublicKeyBean(Parcel in) {
            this.id = in.readString();
            this.publicKey = in.readString();
            this.controller = in.readString();
        }

        public static final Creator<PublicKeyBean> CREATOR = new Creator<PublicKeyBean>() {
            @Override
            public PublicKeyBean createFromParcel(Parcel source) {
                return new PublicKeyBean(source);
            }

            @Override
            public PublicKeyBean[] newArray(int size) {
                return new PublicKeyBean[size];
            }
        };
    }

    @Override
    public int describeContents() {
        return 0;
    }

    @Override
    public void writeToParcel(Parcel dest, int flags) {
        dest.writeLong(this.issuanceDate);
        dest.writeString(this.id);
        dest.writeString(this.didName);
        dest.writeString(this.walletId);
        dest.writeString(this.operation);
        dest.writeString(this.status);
        dest.writeParcelable(this.credentialSubject, flags);
        dest.writeLong(this.expires);
        dest.writeList(this.publicKey);
    }

    public DIDInfoEntity() {
    }

    protected DIDInfoEntity(Parcel in) {
        this.issuanceDate = in.readLong();
        this.id = in.readString();
        this.didName = in.readString();
        this.walletId = in.readString();
        this.operation = in.readString();
        this.status = in.readString();
        this.credentialSubject = in.readParcelable(CredentialSubjectBean.class.getClassLoader());
        this.expires = in.readLong();
        this.publicKey = new ArrayList<PublicKeyBean>();
        in.readList(this.publicKey, PublicKeyBean.class.getClassLoader());
    }

    public static final Parcelable.Creator<DIDInfoEntity> CREATOR = new Parcelable.Creator<DIDInfoEntity>() {
        @Override
        public DIDInfoEntity createFromParcel(Parcel source) {
            return new DIDInfoEntity(source);
        }

        @Override
        public DIDInfoEntity[] newArray(int size) {
            return new DIDInfoEntity[size];
        }
    };
}
