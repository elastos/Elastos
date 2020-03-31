package org.elastos.wallet.ela.ui.Assets.bean;

import android.os.Parcel;
import android.os.Parcelable;
import android.support.annotation.Nullable;

import java.io.Serializable;

public class IPEntity implements Serializable, Parcelable {
    private String address;
    private int port;

    public IPEntity(String address, int port) {
        this.address = address;
        this.port = port;
    }

    public String getAddress() {
        return address;
    }

    public void setAddress(String address) {
        this.address = address;
    }

    public int getPort() {
        return port;
    }

    public void setPort(int port) {
        this.port = port;
    }

    @Override
    public boolean equals(@Nullable Object obj) {
        return address.equals(((IPEntity) obj).address) && port == (((IPEntity) obj).port);
    }

    @Override
    public int hashCode() {

        int result = 17;
        result = 31 * result + address.hashCode() + port;
        return result;
    }

    @Override
    public int describeContents() {
        return 0;
    }

    @Override
    public void writeToParcel(Parcel dest, int flags) {
        dest.writeString(this.address);
        dest.writeInt(this.port);
    }

    IPEntity(Parcel in) {
        this.address = in.readString();
        this.port = in.readInt();
    }

    public static final Parcelable.Creator<IPEntity> CREATOR = new Parcelable.Creator<IPEntity>() {
        @Override
        public IPEntity createFromParcel(Parcel source) {
            return new IPEntity(source);
        }

        @Override
        public IPEntity[] newArray(int size) {
            return new IPEntity[size];
        }
    };
}
