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

package org.elastos.wallet.ela.db.table;

import android.os.Parcel;
import android.os.Parcelable;

import io.realm.RealmObject;
import io.realm.annotations.PrimaryKey;
import io.realm.annotations.Required;

/**
 * 用户的表
 */

public class Contact extends RealmObject implements Parcelable, Cloneable {
    public Contact() {
    }

    public Contact(String id, String name, String walletAddr, String mobile, String email, String remark, String filed1, String filed2, String filed3) {
        this.id = id;
        this.name = name;
        this.walletAddr = walletAddr;
        this.mobile = mobile;
        this.email = email;
        this.remark = remark;

    }

    public Contact(String id, String name, String walletAddr, String mobile, String email, String remark) {
        this.id = id;
        this.name = name;
        this.walletAddr = walletAddr;
        this.mobile = mobile;
        this.email = email;
        this.remark = remark;
        this.filed1 = filed1;
        this.filed2 = filed2;
        this.filed3 = filed3;
    }
public void setContact(Contact contact){
    this.id =contact.getId();
    this.name = contact.getName();
    this.walletAddr = contact.getWalletAddr();
    this.mobile = contact.getMobile();
    this.email = contact.getEmail();
    this.remark = contact.getRemark();
    this.filed1 = contact.getFiled1();
    this.filed2 = contact.getFiled2();
    this.filed3 = contact.getFiled3();
}
    //尽量少用int  int在updata的时候不设置默认0
    @PrimaryKey
    private String id;//联系人唯一标示uuid
    @Required
    private String name;
    @Required
    private String walletAddr;
    private String mobile;
    private String email;
    private String remark;
    private String filed1;
    private String filed2;
    private String filed3;//

    public String getId() {
        return id;
    }

    public void setId(String id) {
        this.id = id;
    }

    public String getName() {
        return name;
    }

    public void setName(String name) {
        this.name = name;
    }

    public String getWalletAddr() {
        return walletAddr;
    }

    public void setWalletAddr(String walletAddr) {
        this.walletAddr = walletAddr;
    }

    public String getMobile() {
        return mobile;
    }

    public void setMobile(String mobile) {
        this.mobile = mobile;
    }

    public String getEmail() {
        return email;
    }

    public void setEmail(String email) {
        this.email = email;
    }

    public String getRemark() {
        return remark;
    }

    public void setRemark(String remark) {
        this.remark = remark;
    }

    public String getFiled1() {
        return filed1;
    }

    public void setFiled1(String filed1) {
        this.filed1 = filed1;
    }

    public String getFiled2() {
        return filed2;
    }

    public void setFiled2(String filed2) {
        this.filed2 = filed2;
    }

    public String getFiled3() {
        return filed3;
    }

    public void setFiled3(String filed3) {
        this.filed3 = filed3;
    }


    protected Contact(Parcel in) {
        id = in.readString();
        name = in.readString();
        walletAddr = in.readString();
        mobile = in.readString();
        email = in.readString();
        remark = in.readString();
        filed1 = in.readString();
        filed2 = in.readString();
        filed3 = in.readString();

    }

    public static final Creator<Contact> CREATOR = new Creator<Contact>() {
        @Override
        public Contact createFromParcel(Parcel in) {
            return new Contact(in);
        }

        @Override
        public Contact[] newArray(int size) {
            return new Contact[size];
        }
    };

    @Override
    public int describeContents() {
        return 0;
    }

    @Override
    public void writeToParcel(Parcel dest, int flags) {
        dest.writeString(id);
        dest.writeString(name);
        dest.writeString(walletAddr);
        dest.writeString(mobile);
        dest.writeString(email);
        dest.writeString(remark);
        dest.writeString(filed1);
        dest.writeString(filed2);
        dest.writeString(filed3);
    }

    @Override
    public Object clone() throws CloneNotSupportedException {
        return super.clone();
    }

    @Override
    public String toString() {
        return "Contact{" +
                "id='" + id + '\'' +
                ", name='" + name + '\'' +
                ", walletAddr='" + walletAddr + '\'' +
                ", mobile='" + mobile + '\'' +
                ", email='" + email + '\'' +
                ", remark='" + remark + '\'' +
                ", filed1='" + filed1 + '\'' +
                ", filed2='" + filed2 + '\'' +
                ", filed3='" + filed3 + '\'' +
                '}';
    }
}
