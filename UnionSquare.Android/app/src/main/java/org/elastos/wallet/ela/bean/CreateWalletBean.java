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

    @Override
    public String toString() {
        return "CreateWalletBean{" +
                "masterWalletID='" + masterWalletID + '\'' +
                ", mnemonic='" + mnemonic + '\'' +
                ", phrasePassword='" + phrasePassword + '\'' +
                ", payPassword='" + payPassword + '\'' +
                ", singleAddress=" + singleAddress +
                '}';
    }

    @Override
    public int describeContents() {
        return 0;
    }

    @Override
    public void writeToParcel(Parcel dest, int flags) {

        dest.writeString(masterWalletID);
        dest.writeString(masterWalletName);
        dest.writeString(mnemonic);
        dest.writeString(phrasePassword);
        dest.writeString(payPassword);
        dest.writeByte((byte) (singleAddress ? 1 : 0));

    }

    public static final Creator<CreateWalletBean> CREATOR = new Creator<CreateWalletBean>() {
        @Override
        public CreateWalletBean createFromParcel(Parcel in) {
            return new CreateWalletBean(in);
        }

        @Override
        public CreateWalletBean[] newArray(int size) {
            return new CreateWalletBean[size];
        }
    };

    protected CreateWalletBean(Parcel in) {
        masterWalletID = in.readString();
        masterWalletName = in.readString();
        mnemonic = in.readString();
        phrasePassword = in.readString();
        payPassword = in.readString();
        singleAddress = in.readByte() != 0;

    }
}
