package org.elastos.wallet.ela.utils;

import android.os.Parcel;
import android.os.Parcelable;

public class QrBean implements Parcelable {

    /**
     * version : 0
     * name : 0
     * total : 1.0
     * index : 1.0
     * data : ads
     * md5 : 2deb000b57bfac9d72c14d4ed967b572
     * extra : {"Type":1,"SubWallet":"ELA","transType":1}
     */

    private int version;
    private String name;
    private int total;
    private int index;
    private String data;
    private String md5;
    private ExtraBean extra;

    public int getVersion() {
        return version;
    }

    public void setVersion(int version) {
        this.version = version;
    }

    public String getName() {
        return name;
    }

    public void setName(String name) {
        this.name = name;
    }

    public int getTotal() {
        return total;
    }

    public void setTotal(int total) {
        this.total = total;
    }

    public int getIndex() {
        return index;
    }

    public void setIndex(int index) {
        this.index = index;
    }

    public String getData() {
        return data;
    }

    public void setData(String data) {
        this.data = data;
    }

    public String getMd5() {
        return md5;
    }

    public void setMd5(String md5) {
        this.md5 = md5;
    }

    public ExtraBean getExtra() {
        return extra;
    }

    public void setExtra(ExtraBean extra) {
        this.extra = extra;
    }

    public static class ExtraBean implements Parcelable {
        /**
         * Type : 1
         * SubWallet : ELA
         */

        private int Type;
        private String SubWallet;
        private int transType;

        public int getType() {
            return Type;
        }

        public void setType(int Type) {
            this.Type = Type;
        }

        public String getSubWallet() {
            return SubWallet;
        }

        public void setSubWallet(String SubWallet) {
            this.SubWallet = SubWallet;
        }

        public int getTransType() {
            return transType;
        }

        public void setTransType(int transType) {
            this.transType = transType;
        }




        @Override
        public int describeContents() {
            return 0;
        }

        @Override
        public void writeToParcel(Parcel dest, int flags) {
            dest.writeInt(this.Type);
            dest.writeString(this.SubWallet);
            dest.writeInt(this.transType);

        }

        public ExtraBean() {
        }

        protected ExtraBean(Parcel in) {
            this.Type = in.readInt();
            this.SubWallet = in.readString();
            this.transType = in.readInt();
           
        }

        public static final Creator<ExtraBean> CREATOR = new Creator<ExtraBean>() {
            @Override
            public ExtraBean createFromParcel(Parcel source) {
                return new ExtraBean(source);
            }

            @Override
            public ExtraBean[] newArray(int size) {
                return new ExtraBean[size];
            }
        };
    }

    @Override
    public int describeContents() {
        return 0;
    }

    @Override
    public void writeToParcel(Parcel dest, int flags) {
        dest.writeInt(this.version);
        dest.writeString(this.name);
        dest.writeInt(this.total);
        dest.writeInt(this.index);
        dest.writeString(this.data);
        dest.writeString(this.md5);
        dest.writeParcelable(this.extra, flags);
    }

    public QrBean() {
    }

    protected QrBean(Parcel in) {
        this.version = in.readInt();
        this.name = in.readString();
        this.total = in.readInt();
        this.index = in.readInt();
        this.data = in.readString();
        this.md5 = in.readString();
        this.extra = in.readParcelable(ExtraBean.class.getClassLoader());
    }

    public static final Parcelable.Creator<QrBean> CREATOR = new Parcelable.Creator<QrBean>() {
        @Override
        public QrBean createFromParcel(Parcel source) {
            return new QrBean(source);
        }

        @Override
        public QrBean[] newArray(int size) {
            return new QrBean[size];
        }
    };
}
