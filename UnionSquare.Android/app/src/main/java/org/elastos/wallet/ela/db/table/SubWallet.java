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

import java.io.Serializable;

import io.realm.RealmObject;
import io.realm.annotations.PrimaryKey;
import io.realm.annotations.Required;

/**
 * 子钱包表
 */

public class SubWallet extends RealmObject implements Parcelable, Serializable {
    //主键必须有而且必须设置 否则默认"" 这样再次插入新的就会覆盖
    @PrimaryKey
    private String wallletId;//主键 belongid+chainId
    @Required
    private String belongId;//所属钱包的标识
    private String walletName;//
    private String walletType;//钱包type
    @Required
    private String chainId;//子钱包id
    private String balance;
    private String syncTime;
    private int progress;
    private long bytesPerSecond;//节点同步数率
    private String downloadPeer;//节点同步数率
    private String filed1 = "Connecting";//连接状态"Connecting", "Connected", "Disconnected"  不会在数据库里取这个字段 所有只启用备用字段字段了
    private String filed2 = "false";////是否已经同步半分百了  不会在数据库里取这个字段 所有只启用备用字段字段了
    private String filed3;//

    @Deprecated
    public SubWallet() {
        this.progress = 0;
        this.syncTime = "- -";
    }

    public SubWallet(String belongId, String chainId) {
        this.belongId = belongId;
        this.chainId = chainId;
        this.wallletId = belongId + chainId;
        this.progress = 0;
        this.syncTime = "- -";
    }

    public SubWallet(String wallletId, String belongId, String walletName, String walletType,
                     String chainId, String balance, String syncTime, int progress, long bytesPerSecond,
                     String downloadPeer, String filed1, String filed2, String filed3) {

        this.wallletId = wallletId;
        this.belongId = belongId;
        this.walletName = walletName;
        this.walletType = walletType;
        this.chainId = chainId;
        this.balance = balance;
        this.syncTime = syncTime;
        this.progress = progress;
        this.bytesPerSecond = bytesPerSecond;
        this.downloadPeer = downloadPeer;
        this.filed1 = filed1;
        this.filed2 = filed2;
        this.filed3 = filed3;
    }

    public void setSubWallet(SubWallet subWallet) {
        this.wallletId = subWallet.getWallletId();
        this.belongId = subWallet.getBelongId();
        this.walletName = subWallet.getWalletName();
        this.walletType = subWallet.getWalletType();
        this.chainId = subWallet.getChainId();
        this.balance = subWallet.getBalance();
        this.syncTime = subWallet.getSyncTime();
        this.progress = subWallet.getProgress();
        this.bytesPerSecond = subWallet.getBytesPerSecond();
        this.downloadPeer = subWallet.getDownloadPeer();
        this.filed1 = subWallet.getFiled1();
        this.filed2 = subWallet.getFiled2();
        this.filed3 = subWallet.getFiled3();
    }

    public static final Creator<SubWallet> CREATOR = new Creator<SubWallet>() {
        @Override
        public SubWallet createFromParcel(Parcel in) {
            return new SubWallet(in);
        }

        @Override
        public SubWallet[] newArray(int size) {
            return new SubWallet[size];
        }
    };


    @Override
    public int describeContents() {
        return 0;
    }

    @Override
    public void writeToParcel(Parcel dest, int flags) {
        dest.writeString(wallletId);
        dest.writeString(belongId);
        dest.writeString(walletName);
        dest.writeString(walletType);
        dest.writeString(chainId);
        dest.writeString(balance);
        dest.writeString(syncTime);
        dest.writeLong(bytesPerSecond);
        dest.writeString(downloadPeer);
        dest.writeInt(progress);
        dest.writeString(filed1);
        dest.writeString(filed2);
        dest.writeString(filed3);
    }

    protected SubWallet(Parcel in) {
        wallletId = in.readString();
        belongId = in.readString();
        walletName = in.readString();
        walletType = in.readString();
        chainId = in.readString();
        balance = in.readString();
        syncTime = in.readString();
        progress = in.readInt();
        bytesPerSecond = in.readLong();
        downloadPeer = in.readString();
        filed1 = in.readString();
        filed2 = in.readString();
        filed3 = in.readString();
    }

    private String getWallletId() {
        return wallletId;
    }

    public void setWallletId(String wallletId) {
        this.wallletId = wallletId;
    }

    public String getBelongId() {
        return belongId;
    }

    public void setBelongId(String belongId) {
        this.belongId = belongId;
    }

    public String getWalletName() {
        return walletName;
    }

    public void setWalletName(String walletName) {
        this.walletName = walletName;
    }

    public String getWalletType() {
        return walletType;
    }

    public void setWalletType(String walletType) {
        this.walletType = walletType;
    }

    public String getChainId() {
        return chainId;
    }

    public void setChainId(String chainId) {
        this.chainId = chainId;
    }

    public String getBalance() {
        return balance;
    }

    public void setBalance(String balance) {
        this.balance = balance;
    }

    public String getSyncTime() {
        return syncTime;
    }

    public void setSyncTime(String syncTime) {
        this.syncTime = syncTime;
    }

    public int getProgress() {
        return progress;
    }

    public void setProgress(int progress) {
        this.progress = progress;
    }

    public long getBytesPerSecond() {
        return bytesPerSecond;
    }

    public void setBytesPerSecond(long bytesPerSecond) {
        this.bytesPerSecond = bytesPerSecond;
    }

    public String getDownloadPeer() {
        return downloadPeer;
    }

    public void setDownloadPeer(String downloadPeer) {
        this.downloadPeer = downloadPeer;
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
    public boolean equals(Object obj) {
        if (!(obj instanceof SubWallet)) {
            return false;
        }

        if (this == obj) {
            return true;
        }
        SubWallet e = (SubWallet) obj;
        return e.getBelongId().equals(belongId) && e.getChainId().equals(chainId);


    }

    @Override
    public String toString() {
        return "SubWallet{" +
                "wallletId='" + wallletId + '\'' +
                ", belongId='" + belongId + '\'' +
                ", walletName='" + walletName + '\'' +
                ", walletType='" + walletType + '\'' +
                ", chainId='" + chainId + '\'' +
                ", balance='" + balance + '\'' +
                ", syncTime='" + syncTime + '\'' +
                ", progress=" + progress +
                ", bytesPerSecond=" + bytesPerSecond +
                ", downloadPeer='" + downloadPeer + '\'' +
                ", filed1='" + filed1 + '\'' +
                ", filed2='" + filed2 + '\'' +
                ", filed3='" + filed3 + '\'' +
                '}';
    }
}
