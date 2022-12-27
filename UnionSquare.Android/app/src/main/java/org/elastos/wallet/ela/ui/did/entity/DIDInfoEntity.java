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
import android.support.annotation.Nullable;

import org.elastos.wallet.ela.rxjavahelp.BaseEntity;

import java.io.Serializable;
import java.util.ArrayList;
import java.util.List;

public class DIDInfoEntity extends BaseEntity implements Parcelable, Serializable {
    @Override
    public boolean equals(@Nullable Object other) {
        return this.getId().equals(((DIDInfoEntity) other).getId());

    }

    @Override
    public int hashCode() {

        int result = 17;
        result = 31 * result + id.hashCode();
        return result;
    }

    /**
     * id : innnNZJLqmJ8uKfVHKFxhdqVtvipNHzmZs
     * operation : create
     * status : "Pending
     * publicKey : [{"id":"#primary","publicKey":"031f7a5a6bf3b2450cd9da4048d00a8ef1cb4912b5057535f65f3cc0e0c36f13b4"},{"id":"#recovery","controller":"ip7ntDo2metGnU8wGP4FnyKCUdbHm4BPDh","publicKey":"03d25d582c485856520c501b2e2f92934eda0232ded70cad9e51cf13968cac22cc"}]
     * expires : 2024-02-10T17:00:00Z
     */
    private long issuanceDate;//编辑时间 或者发布时间
    private String id;
    private String didName;
    private String walletId;
    private String operation;//1.创建：create 2.更新，修改：update 3.注销：deactivate
    private String status;//Pending 确认中   Confirmed已确认  Unpublished 未发布(草稿  这个api不提供,保存草稿时候自己设置)
    private long expires;
    private List<PublicKeyBean> publicKey;


    public long getIssuanceDate() {
        return issuanceDate;
    }

    public void setIssuanceDate(long issuanceDate) {
        this.issuanceDate = issuanceDate;
    }

    public String getId() {
        return id;
    }

    public void setId(String id) {
        this.id = id;
    }

    public String getDidName() {
        return didName;
    }

    public void setDidName(String didName) {
        this.didName = didName;
    }

    public String getWalletId() {
        return walletId;
    }

    public void setWalletId(String walletId) {
        this.walletId = walletId;
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
    public long getExpires() {
        return expires;
    }

    public void setExpires(long expires) {
        this.expires = expires;
    }

    public List<PublicKeyBean> getPublicKey() {
        return publicKey;
    }

    public void setPublicKey(List<PublicKeyBean> publicKey) {
        this.publicKey = publicKey;
    }


    public static class PublicKeyBean implements Parcelable, Serializable {
        /**
         * id : #primary
         * publicKey : 031f7a5a6bf3b2450cd9da4048d00a8ef1cb4912b5057535f65f3cc0e0c36f13b4
         * controller : ip7ntDo2metGnU8wGP4FnyKCUdbHm4BPDh
         */

        private String id;
        private String publicKey;
        private String controller;

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

        public String getController() {
            return controller;
        }

        public void setController(String controller) {
            this.controller = controller;
        }

        @Override
        public int describeContents() {
            return 0;
        }

        @Override
        public void writeToParcel(Parcel dest, int flags) {
            dest.writeString(this.id);
            dest.writeString(this.publicKey);
            dest.writeString(this.controller);
        }

        public PublicKeyBean() {
        }

        protected PublicKeyBean(Parcel in) {
            this.id = in.readString();
            this.publicKey = in.readString();
            this.controller = in.readString();
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
        dest.writeLong(this.issuanceDate);
        dest.writeString(this.id);
        dest.writeString(this.didName);
        dest.writeString(this.walletId);
        dest.writeString(this.operation);
        dest.writeString(this.status);
        dest.writeLong(this.expires);
        dest.writeList(this.publicKey);
    }

    public DIDInfoEntity() {
    }

    protected DIDInfoEntity(Parcel in) {
        this.issuanceDate = in.readLong();
        this.id = in.readString();
        this.didName = in.readString();
        this.walletId = in.readString();
        this.operation = in.readString();
        this.status = in.readString();
        this.expires = in.readLong();
        this.publicKey = new ArrayList<PublicKeyBean>();
        in.readList(this.publicKey, PublicKeyBean.class.getClassLoader());
    }

    public static final Parcelable.Creator<DIDInfoEntity> CREATOR = new Parcelable.Creator<DIDInfoEntity>() {
        @Override
        public DIDInfoEntity createFromParcel(Parcel source) {
            return new DIDInfoEntity(source);
        }

        @Override
        public DIDInfoEntity[] newArray(int size) {
            return new DIDInfoEntity[size];
        }
    };

    @Override
    public String toString() {
        return "DIDInfoEntity{" +
                "issuanceDate=" + issuanceDate +
                ", id='" + id + '\'' +
                ", didName='" + didName + '\'' +
                ", walletId='" + walletId + '\'' +
                ", operation='" + operation + '\'' +
                ", status='" + status + '\'' +
                ", expires=" + expires +
                ", publicKey=" + publicKey +
                '}';
    }
}
