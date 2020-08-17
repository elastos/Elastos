package org.elastos.wallet.ela.ui.Assets.bean;

import android.os.Parcel;
import android.os.Parcelable;

import java.math.BigDecimal;
import java.util.ArrayList;

public class VoteStatus implements Parcelable {
    private String type;//唯一表示
    private int iconID;
    private String name;
    private int status;//0没有投票   1 有投票部分失效 2 有投票完全失效 3有投票无失效
    private BigDecimal count;
    private long  time;//投票时间
    private long  expire;//有效期
    private ArrayList<Object> data;//有效期

    public String getType() {
        return type;
    }

    public void setType(String type) {
        this.type = type;
    }

    public int getIconID() {
        return iconID;
    }

    public void setIconID(int iconID) {
        this.iconID = iconID;
    }

    public String getName() {
        return name;
    }

    public void setName(String name) {
        this.name = name;
    }

    public int getStatus() {
        return status;
    }

    public void setStatus(int status) {
        this.status = status;
    }

    public BigDecimal getCount() {
        return count;
    }

    public void setCount(BigDecimal count) {
        this.count = count;
    }

    public long getTime() {
        return time;
    }

    public void setTime(long time) {
        this.time = time;
    }

    public long getExpire() {
        return expire;
    }

    public void setExpire(long expire) {
        this.expire = expire;
    }

    public ArrayList<Object> getData() {
        return data;
    }

    public void setData(ArrayList<Object> data) {
        this.data = data;
    }

    @Override
    public int describeContents() {
        return 0;
    }

    @Override
    public void writeToParcel(Parcel dest, int flags) {
        dest.writeString(this.type);
        dest.writeInt(this.iconID);
        dest.writeString(this.name);
        dest.writeInt(this.status);
        dest.writeSerializable(this.count);
        dest.writeLong(this.time);
        dest.writeLong(this.expire);
        dest.writeList(this.data);
    }

    public VoteStatus() {
    }

    protected VoteStatus(Parcel in) {
        this.type = in.readString();
        this.iconID = in.readInt();
        this.name = in.readString();
        this.status = in.readInt();
        this.count = (BigDecimal) in.readSerializable();
        this.time = in.readLong();
        this.expire = in.readLong();
        this.data = new ArrayList<Object>();
        in.readList(this.data, Object.class.getClassLoader());
    }

    public static final Parcelable.Creator<VoteStatus> CREATOR = new Parcelable.Creator<VoteStatus>() {
        @Override
        public VoteStatus createFromParcel(Parcel source) {
            return new VoteStatus(source);
        }

        @Override
        public VoteStatus[] newArray(int size) {
            return new VoteStatus[size];
        }
    };
}