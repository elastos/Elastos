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

package org.elastos.wallet.ela.ui.did.entity;

import android.os.Parcel;
import android.os.Parcelable;
import android.text.TextUtils;

public class CredentialSubjectBean implements Parcelable {
    //一共17
    private String didName;//did的name
    private String did;
    private String nickname;
    private String gender;// 1男2nv
    private String birthday;//秒值
    private String avatar;
    private String email;
    private String phone;
    private String phoneCode;
    private String nation;//使用area code
    private String introduction;
    private String homePage;
    private String wechat;
    private String twitter;
    private String weibo;
    private String facebook;
    private String googleAccount;
    private long editTime;//s

    public boolean whetherEmpty() {
        return TextUtils.isEmpty(nickname) && TextUtils.isEmpty(gender) && TextUtils.isEmpty(birthday)
                && TextUtils.isEmpty(avatar) && TextUtils.isEmpty(email) && TextUtils.isEmpty(phone)
                && TextUtils.isEmpty(phoneCode) && TextUtils.isEmpty(nation) && TextUtils.isEmpty(introduction)
                && TextUtils.isEmpty(homePage) && TextUtils.isEmpty(wechat) && TextUtils.isEmpty(twitter)
                && TextUtils.isEmpty(weibo) && TextUtils.isEmpty(facebook) && TextUtils.isEmpty(googleAccount);
    }

    public String getDidName() {
        return didName;
    }

    public void setDidName(String didName) {
        this.didName = didName;
    }

    public String getDid() {
        return did;
    }

    public void setDid(String did) {
        this.did = did;
    }

    public long getEditTime() {
        return editTime;
    }

    public void setEditTime(long editTime) {
        this.editTime = editTime;
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

    public String getBirthday() {
        return birthday;
    }

    public void setBirthday(String birthday) {
        this.birthday = birthday;
    }

    public String getAvatar() {
        return avatar;
    }

    public void setAvatar(String avatar) {
        this.avatar = avatar;
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

    public String getNation() {
        return nation;
    }

    public void setNation(String nation) {
        this.nation = nation;
    }

    public String getIntroduction() {
        return introduction;
    }

    public void setIntroduction(String introduction) {
        this.introduction = introduction;
    }

    public String getHomePage() {
        return homePage;
    }

    public void setHomePage(String homePage) {
        this.homePage = homePage;
    }

    public String getWechat() {
        return wechat;
    }

    public void setWechat(String wechat) {
        this.wechat = wechat;
    }

    public String getTwitter() {
        return twitter;
    }

    public void setTwitter(String twitter) {
        this.twitter = twitter;
    }

    public String getWeibo() {
        return weibo;
    }

    public void setWeibo(String weibo) {
        this.weibo = weibo;
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

    /* public CredentialSubjectBean() {
    }*/
    public CredentialSubjectBean(String did, String didName) {
        this.did = did;
        this.didName = didName;
    }

    public CredentialSubjectBean() {
    }

    @Override
    public String toString() {
        return "CredentialSubjectBean{" +
                "didName='" + didName + '\'' +
                ", did='" + did + '\'' +
                ", nickname='" + nickname + '\'' +
                ", gender='" + gender + '\'' +
                ", birthday='" + birthday + '\'' +
                ", avatar='" + avatar + '\'' +
                ", email='" + email + '\'' +
                ", phone='" + phone + '\'' +
                ", phoneCode='" + phoneCode + '\'' +
                ", nation='" + nation + '\'' +
                ", introduction='" + introduction + '\'' +
                ", homePage='" + homePage + '\'' +
                ", wechat='" + wechat + '\'' +
                ", twitter='" + twitter + '\'' +
                ", weibo='" + weibo + '\'' +
                ", facebook='" + facebook + '\'' +
                ", googleAccount='" + googleAccount + '\'' +
                ", editTime=" + editTime +
                '}';
    }

    @Override
    public int describeContents() {
        return 0;
    }

    @Override
    public void writeToParcel(Parcel dest, int flags) {
        dest.writeString(this.didName);
        dest.writeString(this.did);
        dest.writeString(this.nickname);
        dest.writeString(this.gender);
        dest.writeString(this.birthday);
        dest.writeString(this.avatar);
        dest.writeString(this.email);
        dest.writeString(this.phone);
        dest.writeString(this.phoneCode);
        dest.writeString(this.nation);
        dest.writeString(this.introduction);
        dest.writeString(this.homePage);
        dest.writeString(this.wechat);
        dest.writeString(this.twitter);
        dest.writeString(this.weibo);
        dest.writeString(this.facebook);
        dest.writeString(this.googleAccount);
        dest.writeLong(this.editTime);
    }

    protected CredentialSubjectBean(Parcel in) {
        this.didName = in.readString();
        this.did = in.readString();
        this.nickname = in.readString();
        this.gender = in.readString();
        this.birthday = in.readString();
        this.avatar = in.readString();
        this.email = in.readString();
        this.phone = in.readString();
        this.phoneCode = in.readString();
        this.nation = in.readString();
        this.introduction = in.readString();
        this.homePage = in.readString();
        this.wechat = in.readString();
        this.twitter = in.readString();
        this.weibo = in.readString();
        this.facebook = in.readString();
        this.googleAccount = in.readString();
        this.editTime = in.readLong();
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

