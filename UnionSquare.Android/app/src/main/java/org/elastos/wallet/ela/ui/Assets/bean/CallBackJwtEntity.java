package org.elastos.wallet.ela.ui.Assets.bean;

import android.os.Parcel;
import android.os.Parcelable;

public class CallBackJwtEntity implements Parcelable {
    /**
     * type : credaccess
     * iss : did:ela:iUQtoHoQx8zgxRcLx6FxLKE4eYJiEz8nzC
     * iat : 1566352213
     * exp : 1566382213
     * aud : did:ela:e02e05a2e7dc29a5f2a5882c509a56CeYJ
     * req : xx
     */

    private String type;
    private String iss;
    private int iat;
    private int exp;
    private String aud;
    private String req;

    public String getType() {
        return type;
    }

    public void setType(String type) {
        this.type = type;
    }

    public String getIss() {
        return iss;
    }

    public void setIss(String iss) {
        this.iss = iss;
    }

    public int getIat() {
        return iat;
    }

    public void setIat(int iat) {
        this.iat = iat;
    }

    public int getExp() {
        return exp;
    }

    public void setExp(int exp) {
        this.exp = exp;
    }

    public String getAud() {
        return aud;
    }

    public void setAud(String aud) {
        this.aud = aud;
    }

    public String getReq() {
        return req;
    }

    public void setReq(String req) {
        this.req = req;
    }

    @Override
    public int describeContents() {
        return 0;
    }

    @Override
    public void writeToParcel(Parcel dest, int flags) {
        dest.writeString(this.type);
        dest.writeString(this.iss);
        dest.writeInt(this.iat);
        dest.writeInt(this.exp);
        dest.writeString(this.aud);
        dest.writeString(this.req);
    }

    public CallBackJwtEntity() {
    }

    protected CallBackJwtEntity(Parcel in) {
        this.type = in.readString();
        this.iss = in.readString();
        this.iat = in.readInt();
        this.exp = in.readInt();
        this.aud = in.readString();
        this.req = in.readString();
    }

    public static final Parcelable.Creator<CallBackJwtEntity> CREATOR = new Parcelable.Creator<CallBackJwtEntity>() {
        @Override
        public CallBackJwtEntity createFromParcel(Parcel source) {
            return new CallBackJwtEntity(source);
        }

        @Override
        public CallBackJwtEntity[] newArray(int size) {
            return new CallBackJwtEntity[size];
        }
    };
}
