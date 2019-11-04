package org.elastos.wallet.ela.ui.did.entity;

import android.os.Parcel;
import android.os.Parcelable;

import org.elastos.wallet.ela.rxjavahelp.BaseEntity;

import java.util.ArrayList;
import java.util.List;

public class DIDListEntity extends BaseEntity implements Parcelable {
    /**
     * DID : [{"didName":"testName","expires":1575104460,"id":"iotCQuRW4RPbC3fqM2q7TPDYw8hG33RLLU","issuanceDate":1572594881,"operation":"create","status":"Confirmed"},{"expires":1575104460,"id":"innnNZJLqmJ8uKfVHKFxhdqVtvipNHzmZs","issuanceDate":1572516335,"operation":"update","status":"Confirmed"},{"expires":1575104460,"id":"iZFrhZLetd6i6qPu2MsYvE2aKrgw7Af4Ww","issuanceDate":1572507697,"operation":"update","status":"Confirmed"}]
     * MaxCount : 3
     */

    private int MaxCount;
    private List<DIDBean> DID;

    public int getMaxCount() {
        return MaxCount;
    }

    public void setMaxCount(int MaxCount) {
        this.MaxCount = MaxCount;
    }

    public List<DIDBean> getDID() {
        return DID;
    }

    public void setDID(List<DIDBean> DID) {
        this.DID = DID;
    }

    public static class DIDBean implements Parcelable {
        /**
         * didName : testName
         * expires : 1575104460
         * id : iotCQuRW4RPbC3fqM2q7TPDYw8hG33RLLU
         * issuanceDate : 1572594881
         * operation : create
         * status : Confirmed
         */

        private String didName;
        private int expires;
        private String id;
        private int issuanceDate;
        private String operation;
        private String status;

        public String getDidName() {
            return didName;
        }

        public void setDidName(String didName) {
            this.didName = didName;
        }

        public int getExpires() {
            return expires;
        }

        public void setExpires(int expires) {
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

        @Override
        public int describeContents() {
            return 0;
        }

        @Override
        public void writeToParcel(Parcel dest, int flags) {
            dest.writeString(this.didName);
            dest.writeInt(this.expires);
            dest.writeString(this.id);
            dest.writeInt(this.issuanceDate);
            dest.writeString(this.operation);
            dest.writeString(this.status);
        }

        public DIDBean() {
        }

        protected DIDBean(Parcel in) {
            this.didName = in.readString();
            this.expires = in.readInt();
            this.id = in.readString();
            this.issuanceDate = in.readInt();
            this.operation = in.readString();
            this.status = in.readString();
        }

        public static final Creator<DIDBean> CREATOR = new Creator<DIDBean>() {
            @Override
            public DIDBean createFromParcel(Parcel source) {
                return new DIDBean(source);
            }

            @Override
            public DIDBean[] newArray(int size) {
                return new DIDBean[size];
            }
        };
    }

    @Override
    public int describeContents() {
        return 0;
    }

    @Override
    public void writeToParcel(Parcel dest, int flags) {
        dest.writeInt(this.MaxCount);
        dest.writeList(this.DID);
    }

    public DIDListEntity() {
    }

    protected DIDListEntity(Parcel in) {
        this.MaxCount = in.readInt();
        this.DID = new ArrayList<DIDBean>();
        in.readList(this.DID, DIDBean.class.getClassLoader());
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
