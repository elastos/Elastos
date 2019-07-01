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
    private String privateKey;//钱包私钥
    private String keyStore;//钱包keystore
    private String mnemonic;//助记词
    private boolean isDefault;//默認錢包 是否是默認的
    private boolean singleAddress;//是否单地址
    private RealmList<String> walletAddrList;//所有钱包地址*/
    private int type=0;//0 普通单签 1单签只读 2普通多签 3多签只读
    private String filed1;//
    private String filed2;//
    private String filed3;//


    public Wallet() {
    }


    public void setWalletData(Wallet wallet) {
        this.walletId = wallet.getWalletId();
        this.walletName = wallet.getWalletName();
        this.mainWalletAddr = wallet.getMainWalletAddr();
        this.privateKey = wallet.getPrivateKey();
        this.keyStore = wallet.getKeyStore();
        this.mnemonic = wallet.getMnemonic();
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
        dest.writeString(privateKey);
        dest.writeString(keyStore);
        dest.writeString(mnemonic);
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
        privateKey = in.readString();
        keyStore = in.readString();
        mnemonic = in.readString();
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

    public String getPrivateKey() {
        return privateKey;
    }

    public void setPrivateKey(String privateKey) {
        this.privateKey = privateKey;
    }

    public String getKeyStore() {
        return keyStore;
    }

    public void setKeyStore(String keyStore) {
        this.keyStore = keyStore;
    }

    public String getMnemonic() {
        return mnemonic;
    }

    public void setMnemonic(String mnemonic) {
        this.mnemonic = mnemonic;
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
                ", privateKey='" + privateKey + '\'' +
                ", keyStore='" + keyStore + '\'' +
                ", mnemonic='" + mnemonic + '\'' +
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
