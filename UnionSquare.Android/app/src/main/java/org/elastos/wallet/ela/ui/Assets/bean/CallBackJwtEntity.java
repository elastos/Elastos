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
     * "userId": "5e561e879ba6e50078684107",
     */

    private String type;
    private String iss;
    private long iat;
    private long exp;
    private String aud;
    private String req;
    private String presentation;
    private String userId;

    public String getUserId() {
        return userId;
    }

    public void setUserId(String userId) {
        this.userId = userId;
    }

    public String getPresentation() {
        return presentation;
    }

    public void setPresentation(String presentation) {
        this.presentation = presentation;
    }

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

    public long getIat() {
        return iat;
    }

    public void setIat(long iat) {
        this.iat = iat;
    }

    public long getExp() {
        return exp;
    }

    public void setExp(long exp) {
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
        dest.writeLong(this.iat);
        dest.writeLong(this.exp);
        dest.writeString(this.aud);
        dest.writeString(this.req);
        dest.writeString(this.presentation);
        dest.writeString(this.userId);
    }

    public CallBackJwtEntity() {
    }

    protected CallBackJwtEntity(Parcel in) {
        this.type = in.readString();
        this.iss = in.readString();
        this.iat = in.readLong();
        this.exp = in.readLong();
        this.aud = in.readString();
        this.req = in.readString();
        this.presentation = in.readString();
        this.userId = in.readString();
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
