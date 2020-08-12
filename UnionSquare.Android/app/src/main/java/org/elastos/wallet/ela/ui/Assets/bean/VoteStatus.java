package org.elastos.wallet.ela.ui.Assets.bean;

import android.os.Parcel;
import android.os.Parcelable;

import java.math.BigDecimal;

public class VoteStatus implements Parcelable {
    private int index;//唯一表示
    private int iconID;
    private String name;
    private int status;//0没有投票   1 有投票部分失效 2 有投票完全失效 3有投票无失效
    private BigDecimal count;

    public int getIndex() {
        return index;
    }

    public void setIndex(int index) {
        this.index = index;
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

    @Override
    public int describeContents() {
        return 0;
    }

    @Override
    public void writeToParcel(Parcel dest, int flags) {
        dest.writeInt(this.index);
        dest.writeInt(this.iconID);
        dest.writeString(this.name);
        dest.writeInt(this.status);
        dest.writeSerializable(this.count);
    }

    public VoteStatus() {
    }

    protected VoteStatus(Parcel in) {
        this.index = in.readInt();
        this.iconID = in.readInt();
        this.name = in.readString();
        this.status = in.readInt();
        this.count = (BigDecimal) in.readSerializable();
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