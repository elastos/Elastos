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

public class SubWallet extends RealmObject implements Parcelable ,Serializable{
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
    private String filed1="Connecting";//连接状态  不会在数据库里取这个字段 所有只启用备用字段字段了
    private String filed2;//
    private String filed3;//


    public SubWallet() {
        this.progress = 0;
        this.syncTime = "- -";
    }

    public SubWallet(String wallletId, String belongId, String walletName, String walletType, String chainId, String balance, String syncTime, int progress, String filed1, String filed2, String filed3) {

        this.wallletId = wallletId;
        this.belongId = belongId;
        this.walletName = walletName;
        this.walletType = walletType;
        this.chainId = chainId;
        this.balance = balance;
        this.syncTime = syncTime;
        this.progress = progress;
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
}
