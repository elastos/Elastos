package org.elastos.wallet.ela.ui.Assets.bean.qr;

import android.os.Parcel;
import android.os.Parcelable;

public class RecieveJwtEntity implements Parcelable {
    private String iss;
    private String callbackurl;
    private long iat;
    private long exp;
    private WebsiteBean website;

    public String getIss() {
        return iss;
    }

    public void setIss(String iss) {
        this.iss = iss;
    }

    public String getCallbackurl() {
        return callbackurl;
    }

    public void setCallbackurl(String callbackurl) {
        this.callbackurl = callbackurl;
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

    public WebsiteBean getWebsite() {
        return website;
    }

    public void setWebsite(WebsiteBean website) {
        this.website = website;
    }

    public static class WebsiteBean implements Parcelable {
        /**
         * domain : https://staging.cyberrepublic.org
         * logo : https://staging.cyberrepublic.org/assets/images/logo.svg
         */

        private String domain;
        private String logo;

        public String getDomain() {
            return domain;
        }

        public void setDomain(String domain) {
            this.domain = domain;
        }

        public String getLogo() {
            return logo;
        }

        public void setLogo(String logo) {
            this.logo = logo;
        }

        @Override
        public int describeContents() {
            return 0;
        }

        @Override
        public void writeToParcel(Parcel dest, int flags) {
            dest.writeString(this.domain);
            dest.writeString(this.logo);
        }

        public WebsiteBean() {
        }

        protected WebsiteBean(Parcel in) {
            this.domain = in.readString();
            this.logo = in.readString();
        }

        public static final Parcelable.Creator<WebsiteBean> CREATOR = new Parcelable.Creator<WebsiteBean>() {
            @Override
            public WebsiteBean createFromParcel(Parcel source) {
                return new WebsiteBean(source);
            }

            @Override
            public WebsiteBean[] newArray(int size) {
                return new WebsiteBean[size];
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
        dest.writeString(this.callbackurl);
        dest.writeLong(this.iat);
        dest.writeLong(this.exp);
        dest.writeParcelable(this.website, flags);
    }

    protected RecieveJwtEntity(Parcel in) {
        this.iss = in.readString();
        this.callbackurl = in.readString();
        this.iat = in.readLong();
        this.exp = in.readLong();
        this.website = in.readParcelable(WebsiteBean.class.getClassLoader());
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
