package org.elastos.wallet.ela.ui.Assets.bean;

import android.os.Parcel;
import android.os.Parcelable;

public class RecieveJwtEntity implements Parcelable {
    /**
     * iss : iYpQMwheDxySqivocSJaoprcoDTqQsDYAu
     * userId : 5e561e879ba6e50078684107
     * callbackurl : https://staging-api.cyberrepublic.org/api/user/did-callback-ela
     * claims : {}
     * iat : 1583214652
     * exp : 1583243907
     * jti : b774543e-d470-4f08-aaf1-994bf51e4ab9
     */

    private String iss;
    private String userId;
    private String callbackurl;
    private ClaimsBean claims;
    private int iat;
    private int exp;
    private String jti;

    public String getIss() {
        return iss;
    }

    public void setIss(String iss) {
        this.iss = iss;
    }

    public String getUserId() {
        return userId;
    }

    public void setUserId(String userId) {
        this.userId = userId;
    }

    public String getCallbackurl() {
        return callbackurl;
    }

    public void setCallbackurl(String callbackurl) {
        this.callbackurl = callbackurl;
    }

    public ClaimsBean getClaims() {
        return claims;
    }

    public void setClaims(ClaimsBean claims) {
        this.claims = claims;
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

    public String getJti() {
        return jti;
    }

    public void setJti(String jti) {
        this.jti = jti;
    }

    public static class ClaimsBean implements Parcelable {
        @Override
        public int describeContents() {
            return 0;
        }

        @Override
        public void writeToParcel(Parcel dest, int flags) {
        }

        public ClaimsBean() {
        }

        protected ClaimsBean(Parcel in) {
        }

        public static final Creator<ClaimsBean> CREATOR = new Creator<ClaimsBean>() {
            @Override
            public ClaimsBean createFromParcel(Parcel source) {
                return new ClaimsBean(source);
            }

            @Override
            public ClaimsBean[] newArray(int size) {
                return new ClaimsBean[size];
            }
        };
    }

    public RecieveJwtEntity() {
    }

    @Override
    public int describeContents() {
        return 0;
    }

    @Override
    public void writeToParcel(Parcel dest, int flags) {
        dest.writeString(this.iss);
        dest.writeString(this.userId);
        dest.writeString(this.callbackurl);
        dest.writeParcelable(this.claims, flags);
        dest.writeInt(this.iat);
        dest.writeInt(this.exp);
        dest.writeString(this.jti);
    }

    protected RecieveJwtEntity(Parcel in) {
        this.iss = in.readString();
        this.userId = in.readString();
        this.callbackurl = in.readString();
        this.claims = in.readParcelable(ClaimsBean.class.getClassLoader());
        this.iat = in.readInt();
        this.exp = in.readInt();
        this.jti = in.readString();
    }

    public static final Creator<RecieveJwtEntity> CREATOR = new Creator<RecieveJwtEntity>() {
        @Override
        public RecieveJwtEntity createFromParcel(Parcel source) {
            return new RecieveJwtEntity(source);
        }

        @Override
        public RecieveJwtEntity[] newArray(int size) {
            return new RecieveJwtEntity[size];
        }
    };
}
