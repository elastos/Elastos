/*
 * Copyright (c) 2019 Elastos Foundation
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

package org.elastos.wallet.ela.ui.crvote.bean;

import android.os.Parcel;
import android.os.Parcelable;

public class CrStatusBean implements Parcelable {

    /**
     * Info : {"BondedDID":false,"CID":"ihNSAaGACbGucnXjvPKCRA7Ei7puUyUnrd","CROwnerPublicKey":"0221d3e27c9636ee31b894ef3648df8ec8a266807144beb7ac7dbad43897630fb8","Confirms":4584,"DID":"","Location":1268,"NickName":"linda-reg17","URL":"www.baidu.com"}
     * Status : Registered
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
         * BondedDID : false
         * CID : ihNSAaGACbGucnXjvPKCRA7Ei7puUyUnrd
         * CROwnerPublicKey : 0221d3e27c9636ee31b894ef3648df8ec8a266807144beb7ac7dbad43897630fb8
         * Confirms : 4584
         * DID :
         * Location : 1268
         * NickName : linda-reg17
         * URL : www.baidu.com
         */

        private boolean BondedDID;
        private String CID;
        private String CROwnerPublicKey;
        private int Confirms;
        private String DID;
        private int Location;
        private String NickName;
        private String URL;

        public boolean isBondedDID() {
            return BondedDID;
        }

        public void setBondedDID(boolean BondedDID) {
            this.BondedDID = BondedDID;
        }

        public String getCID() {
            return CID;
        }

        public void setCID(String CID) {
            this.CID = CID;
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

        public String getDID() {
            return DID;
        }

        public void setDID(String DID) {
            this.DID = DID;
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
            dest.writeByte(this.BondedDID ? (byte) 1 : (byte) 0);
            dest.writeString(this.CID);
            dest.writeString(this.CROwnerPublicKey);
            dest.writeInt(this.Confirms);
            dest.writeString(this.DID);
            dest.writeInt(this.Location);
            dest.writeString(this.NickName);
            dest.writeString(this.URL);
        }

        public InfoBean() {
        }

        protected InfoBean(Parcel in) {
            this.BondedDID = in.readByte() != 0;
            this.CID = in.readString();
            this.CROwnerPublicKey = in.readString();
            this.Confirms = in.readInt();
            this.DID = in.readString();
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

    @Override
    public int describeContents() {
        return 0;
    }

    @Override
    public void writeToParcel(Parcel dest, int flags) {
        dest.writeParcelable(this.Info, flags);
        dest.writeString(this.Status);
    }

    public CrStatusBean() {
    }

    protected CrStatusBean(Parcel in) {
        this.Info = in.readParcelable(InfoBean.class.getClassLoader());
        this.Status = in.readString();
    }

    public static final Parcelable.Creator<CrStatusBean> CREATOR = new Parcelable.Creator<CrStatusBean>() {
        @Override
        public CrStatusBean createFromParcel(Parcel source) {
            return new CrStatusBean(source);
        }

        @Override
        public CrStatusBean[] newArray(int size) {
            return new CrStatusBean[size];
        }
    };
}
