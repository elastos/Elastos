package org.elastos.wallet.ela.bean;

import android.os.Parcel;
import android.os.Parcelable;

public class CreateWalletBean implements Parcelable {

    private String masterWalletID;
    private String masterWalletName;
    private String mnemonic;
    private String phrasePassword = "";//助记词密码
    private String payPassword;
    private boolean singleAddress;
    private String privateKey;

    public CreateWalletBean() {

    }

    public String getMasterWalletID() {
        return masterWalletID;
    }

    public String getMasterWalletName() {
        return masterWalletName;
    }

    public void setMasterWalletName(String masterWalletName) {
        this.masterWalletName = masterWalletName;
    }

    public void setMasterWalletID(String masterWalletID) {
        this.masterWalletID = masterWalletID;
    }

    public String getMnemonic() {
        return mnemonic;
    }

    public void setMnemonic(String mnemonic) {
        this.mnemonic = mnemonic;
    }

    public String getPhrasePassword() {
        return phrasePassword;
    }

    public void setPhrasePassword(String phrasePassword) {
        this.phrasePassword = phrasePassword;
    }

    public String getPayPassword() {
        return payPassword;
    }

    public void setPayPassword(String payPassword) {
        this.payPassword = payPassword;
    }

    public boolean getSingleAddress() {
        return singleAddress;
    }

    public void setSingleAddress(boolean singleAddress) {
        this.singleAddress = singleAddress;
    }

    public String getPrivateKey() {
        return privateKey;
    }

    public void setPrivateKey(String privateKey) {
        this.privateKey = privateKey;
    }

    @Override
    public String toString() {
        return "CreateWalletBean{" +
                "masterWalletID='" + masterWalletID + '\'' +
                ", masterWalletName='" + masterWalletName + '\'' +
                ", mnemonic='" + mnemonic + '\'' +
                ", phrasePassword='" + phrasePassword + '\'' +
                ", payPassword='" + payPassword + '\'' +
                ", singleAddress=" + singleAddress +
                ", privateKey='" + privateKey + '\'' +
                '}';
    }

    @Override
    public int describeContents() {
        return 0;
    }

    @Override
    public void writeToParcel(Parcel dest, int flags) {
        dest.writeString(this.masterWalletID);
        dest.writeString(this.masterWalletName);
        dest.writeString(this.mnemonic);
        dest.writeString(this.phrasePassword);
        dest.writeString(this.payPassword);
        dest.writeByte(this.singleAddress ? (byte) 1 : (byte) 0);
        dest.writeString(this.privateKey);
    }

    protected CreateWalletBean(Parcel in) {
        this.masterWalletID = in.readString();
        this.masterWalletName = in.readString();
        this.mnemonic = in.readString();
        this.phrasePassword = in.readString();
        this.payPassword = in.readString();
        this.singleAddress = in.readByte() != 0;
        this.privateKey = in.readString();
    }

    public static final Creator<CreateWalletBean> CREATOR = new Creator<CreateWalletBean>() {
        @Override
        public CreateWalletBean createFromParcel(Parcel source) {
            return new CreateWalletBean(source);
        }

        @Override
        public CreateWalletBean[] newArray(int size) {
            return new CreateWalletBean[size];
        }
    };
}
