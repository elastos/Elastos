package org.elastos.wallet.ela.ui.crvote.bean;

import android.os.Parcel;
import android.os.Parcelable;

public class CrStatusBean {
    /**
     * Info : {"CROwnerDID":"im4yHzAA68RRUCf8gXD6i43rJYHK9VJqLH","CROwnerPublicKey":"02c632e27b19260d80d58a857d2acd9eb603f698445cc07ba94d52296468706331","Confirms":10795,"Location":93,"NickName":"raocr","Url":"http://52.80.54.227:9000/"}
     * Status : ReturnDeposit
     */

    private InfoBean Info;
    private String Status;

    public InfoBean getInfo() {
        return Info;
    }

    public void setInfo(InfoBean Info) {
        this.Info = Info;
    }

    public String getStatus() {
        return Status;
    }

    public void setStatus(String Status) {
        this.Status = Status;
    }

    public static class InfoBean implements Parcelable {
        /**
         * CROwnerDID : im4yHzAA68RRUCf8gXD6i43rJYHK9VJqLH
         * CROwnerPublicKey : 02c632e27b19260d80d58a857d2acd9eb603f698445cc07ba94d52296468706331
         * Confirms : 10795
         * Location : 93
         * NickName : raocr
         * Url : http://52.80.54.227:9000/
         */

        private String CROwnerDID;
        private String CROwnerPublicKey;
        private int Confirms;
        private int Location;
        private String NickName;
        private String URL;

        public String getCROwnerDID() {
            return CROwnerDID;
        }

        public void setCROwnerDID(String CROwnerDID) {
            this.CROwnerDID = CROwnerDID;
        }

        public String getCROwnerPublicKey() {
            return CROwnerPublicKey;
        }

        public void setCROwnerPublicKey(String CROwnerPublicKey) {
            this.CROwnerPublicKey = CROwnerPublicKey;
        }

        public int getConfirms() {
            return Confirms;
        }

        public void setConfirms(int Confirms) {
            this.Confirms = Confirms;
        }

        public int getLocation() {
            return Location;
        }

        public void setLocation(int Location) {
            this.Location = Location;
        }

        public String getNickName() {
            return NickName;
        }

        public void setNickName(String NickName) {
            this.NickName = NickName;
        }

        public String getURL() {
            return URL;
        }

        public void setURL(String URL) {
            this.URL = URL;
        }

        @Override
        public int describeContents() {
            return 0;
        }

        @Override
        public void writeToParcel(Parcel dest, int flags) {
            dest.writeString(this.CROwnerDID);
            dest.writeString(this.CROwnerPublicKey);
            dest.writeInt(this.Confirms);
            dest.writeInt(this.Location);
            dest.writeString(this.NickName);
            dest.writeString(this.URL);
        }

        public InfoBean() {
        }

        protected InfoBean(Parcel in) {
            this.CROwnerDID = in.readString();
            this.CROwnerPublicKey = in.readString();
            this.Confirms = in.readInt();
            this.Location = in.readInt();
            this.NickName = in.readString();
            this.URL = in.readString();
        }

        public static final Parcelable.Creator<InfoBean> CREATOR = new Parcelable.Creator<InfoBean>() {
            @Override
            public InfoBean createFromParcel(Parcel source) {
                return new InfoBean(source);
            }

            @Override
            public InfoBean[] newArray(int size) {
                return new InfoBean[size];
            }
        };
    }
}
