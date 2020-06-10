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

import io.realm.RealmList;
import io.realm.RealmObject;
import io.realm.annotations.PrimaryKey;

/**
 * 钱包表
 */

public class Wallet extends RealmObject implements Parcelable {

    @PrimaryKey
    private String walletId;//钱包id
    private String walletName;//钱包名称
    private String mainWalletAddr;//主钱包地址
    private String did;//钱包生成的didstring

    private boolean isDefault;//默認錢包 是否是默認的
    private boolean singleAddress;//是否单地址
    private RealmList<String> walletAddrList;//所有钱包地址*/
    private int type = 0;//0 普通单签 1单签只读 2普通多签 3多签只读
    private String filed1;//
    private String filed2;//
    private String filed3;//


    public Wallet() {
    }


    public void setWalletData(Wallet wallet) {
        this.walletId = wallet.getWalletId();
        this.walletName = wallet.getWalletName();
        this.mainWalletAddr = wallet.getMainWalletAddr();
        this.did = wallet.getDid();
        this.isDefault = wallet.isDefault();
        this.singleAddress = wallet.isSingleAddress();
        this.walletAddrList = wallet.getWalletAddrList();
        this.type = wallet.getType();
        this.filed1 = wallet.getFiled1();
        this.filed2 = wallet.getFiled2();
        this.filed3 = wallet.getFiled3();
    }

    public static final Creator<Wallet> CREATOR = new Creator<Wallet>() {
        @Override
        public Wallet createFromParcel(Parcel in) {
            return new Wallet(in);
        }

        @Override
        public Wallet[] newArray(int size) {
            return new Wallet[size];
        }
    };


    @Override
    public int describeContents() {
        return 0;
    }

    @Override
    public void writeToParcel(Parcel dest, int flags) {
        dest.writeString(walletId);
        dest.writeString(walletName);
        dest.writeString(mainWalletAddr);
        dest.writeString(did);
        dest.writeByte((byte) (isDefault ? 1 : 0));
        dest.writeByte((byte) (singleAddress ? 1 : 0));
        //dest.writeList(walletAddrList);
        dest.writeInt(type);
        dest.writeString(filed1);
        dest.writeString(filed2);
        dest.writeString(filed3);
    }

    protected Wallet(Parcel in) {
        walletId = in.readString();
        walletName = in.readString();
        mainWalletAddr = in.readString();
        did = in.readString();
        isDefault = in.readByte() != 0;
        singleAddress = in.readByte() != 0;
      /*  if (walletAddrList == null) {
            walletAddrList = new RealmList<String>();
        }
        in.readList(walletAddrList, String.class.getClassLoader());*/
        type = in.readInt();
        filed1 = in.readString();
        filed2 = in.readString();
        filed3 = in.readString();
    }

    public String getWalletId() {
        return walletId;
    }

    public void setWalletId(String walletId) {
        this.walletId = walletId;
    }

    public String getWalletName() {
        return walletName;
    }

    public void setWalletName(String walletName) {
        this.walletName = walletName;
    }

    public String getMainWalletAddr() {
        return mainWalletAddr;
    }

    public void setMainWalletAddr(String mainWalletAddr) {
        this.mainWalletAddr = mainWalletAddr;
    }

    //兼容没有did的情况  所以判空一定要用isEmpty
    public String getDid() {
        if (did == null)
            return "";
        return did;
    }

    public void setDid(String did) {
        this.did = did;
    }

    public boolean isDefault() {
        return isDefault;
    }

    public void setDefault(boolean aDefault) {
        isDefault = aDefault;
    }

    public boolean isSingleAddress() {
        return singleAddress;
    }

    public void setSingleAddress(boolean singleAddress) {
        this.singleAddress = singleAddress;
    }

    public RealmList<String> getWalletAddrList() {
        return walletAddrList;
    }

    public void setWalletAddrList(RealmList<String> walletAddrList) {
        this.walletAddrList = walletAddrList;
    }

    public int getType() {
        return type;
    }

    public void setType(int type) {
        this.type = type;
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

    @Override
    public String toString() {
        return "Wallet{" +
                "walletId='" + walletId + '\'' +
                ", walletName='" + walletName + '\'' +
                ", mainWalletAddr='" + mainWalletAddr + '\'' +
                ", did='" + did + '\'' +
                ", isDefault=" + isDefault +
                ", singleAddress=" + singleAddress +
                ", walletAddrList=" + walletAddrList +
                ", type=" + type +
                ", filed1='" + filed1 + '\'' +
                ", filed2='" + filed2 + '\'' +
                ", filed3='" + filed3 + '\'' +
                '}';
    }
}
