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

package org.elastos.wallet.ela.ui.did.entity;

import android.os.Parcel;
import android.os.Parcelable;

import org.elastos.wallet.ela.rxjavahelp.BaseEntity;

import java.util.ArrayList;
import java.util.List;

public class DIDListEntity extends BaseEntity implements Parcelable {

    /**
     * DID : [{"didName":"舅舅家","expires":1572883200,"id":"im4yHzAA68RRUCf8gXD6i43rJYHK9VJqLH","issuanceDate":1572947858,"operation":"create","publicKey":[{"id":"#primary","publicKey":"02c632e27b19260d80d58a857d2acd9eb603f698445cc07ba94d52296468706331"}],"status":"Pending"}]
     * MaxCount : 1
     */

    private int MaxCount;
    private List<DIDInfoEntity> DID;

    public int getMaxCount() {
        return MaxCount;
    }

    public void setMaxCount(int MaxCount) {
        this.MaxCount = MaxCount;
    }

    public List<DIDInfoEntity> getDID() {
        return DID;
    }

    public void setDID(List<DIDInfoEntity> DID) {
        this.DID = DID;
    }

   /* public static class DIDBean implements Parcelable {
        *//**
         * didName : 舅舅家
         * expires : 1572883200
         * id : im4yHzAA68RRUCf8gXD6i43rJYHK9VJqLH
         * issuanceDate : 1572947858
         * operation : create
         * publicKey : [{"id":"#primary","publicKey":"02c632e27b19260d80d58a857d2acd9eb603f698445cc07ba94d52296468706331"}]
         * status : Pending
         *//*

        private String didName;
        private long expires;
        private String id;
        private int issuanceDate;
        private String operation;
        private String status;
        private String walletId;//新增
        private List<PublicKeyBean> publicKey;

        public String getDidName() {
            return didName;
        }

        public void setDidName(String didName) {
            this.didName = didName;
        }

        public long getExpires() {
            return expires;
        }

        public void setExpires(long expires) {
            this.expires = expires;
        }

        public String getId() {
            return id;
        }

        public void setId(String id) {
            this.id = id;
        }

        public int getIssuanceDate() {
            return issuanceDate;
        }

        public void setIssuanceDate(int issuanceDate) {
            this.issuanceDate = issuanceDate;
        }

        public String getOperation() {
            return operation;
        }

        public void setOperation(String operation) {
            this.operation = operation;
        }

        public String getStatus() {
            return status;
        }

        public void setStatus(String status) {
            this.status = status;
        }

        public String getWalletId() {
            return walletId;
        }

        public void setWalletId(String walletId) {
            this.walletId = walletId;
        }

        public List<PublicKeyBean> getPublicKey() {
            return publicKey;
        }

        public void setPublicKey(List<PublicKeyBean> publicKey) {
            this.publicKey = publicKey;
        }

        public static class PublicKeyBean implements Parcelable {
            *//**
             * id : #primary
             * publicKey : 02c632e27b19260d80d58a857d2acd9eb603f698445cc07ba94d52296468706331
             *//*

            private String id;
            private String publicKey;

            public String getId() {
                return id;
            }

            public void setId(String id) {
                this.id = id;
            }

            public String getPublicKey() {
                return publicKey;
            }

            public void setPublicKey(String publicKey) {
                this.publicKey = publicKey;
            }

            @Override
            public int describeContents() {
                return 0;
            }

            @Override
            public void writeToParcel(Parcel dest, int flags) {
                dest.writeString(this.id);
                dest.writeString(this.publicKey);
            }

            public PublicKeyBean() {
            }

            protected PublicKeyBean(Parcel in) {
                this.id = in.readString();
                this.publicKey = in.readString();
            }

            public static final Creator<PublicKeyBean> CREATOR = new Creator<PublicKeyBean>() {
                @Override
                public PublicKeyBean createFromParcel(Parcel source) {
                    return new PublicKeyBean(source);
                }

                @Override
                public PublicKeyBean[] newArray(int size) {
                    return new PublicKeyBean[size];
                }
            };
        }

        @Override
        public int describeContents() {
            return 0;
        }

        @Override
        public void writeToParcel(Parcel dest, int flags) {
            dest.writeString(this.didName);
            dest.writeLong(this.expires);
            dest.writeString(this.id);
            dest.writeInt(this.issuanceDate);
            dest.writeString(this.operation);
            dest.writeString(this.status);
            dest.writeString(this.walletId);
            dest.writeList(this.publicKey);
        }

        public DIDBean() {
        }

        protected DIDBean(Parcel in) {
            this.didName = in.readString();
            this.expires = in.readLong();
            this.id = in.readString();
            this.issuanceDate = in.readInt();
            this.operation = in.readString();
            this.status = in.readString();
            this.walletId = in.readString();
            this.publicKey = new ArrayList<PublicKeyBean>();
            in.readList(this.publicKey, PublicKeyBean.class.getClassLoader());
        }

        public static final Parcelable.Creator<DIDBean> CREATOR = new Parcelable.Creator<DIDBean>() {
            @Override
            public DIDBean createFromParcel(Parcel source) {
                return new DIDBean(source);
            }

            @Override
            public DIDBean[] newArray(int size) {
                return new DIDBean[size];
            }
        };
    }*/

    @Override
    public int describeContents() {
        return 0;
    }

    @Override
    public void writeToParcel(Parcel dest, int flags) {
        dest.writeInt(this.MaxCount);
        dest.writeTypedList(this.DID);
    }

    public DIDListEntity() {
    }

    protected DIDListEntity(Parcel in) {
        this.MaxCount = in.readInt();
        this.DID = in.createTypedArrayList(DIDInfoEntity.CREATOR);
    }

    public static final Parcelable.Creator<DIDListEntity> CREATOR = new Parcelable.Creator<DIDListEntity>() {
        @Override
        public DIDListEntity createFromParcel(Parcel source) {
            return new DIDListEntity(source);
        }

        @Override
        public DIDListEntity[] newArray(int size) {
            return new DIDListEntity[size];
        }
    };
}
