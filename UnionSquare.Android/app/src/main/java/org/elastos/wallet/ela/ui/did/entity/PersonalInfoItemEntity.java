package org.elastos.wallet.ela.ui.did.entity;

import android.os.Parcel;
import android.os.Parcelable;
import android.support.annotation.Nullable;

public class PersonalInfoItemEntity implements Comparable<PersonalInfoItemEntity>, Parcelable {
    private int index;
    private String hintShow1;//初始化后不会再修改 提示语句
    private String hintShow2;//初始化后不会再修改 提示语句
    private String hintChose;//初始化后不会再修改 提示语句
    private String text1;//第一个可编辑的结果
    private String text2;//第二个可编辑的结果

    public int getIndex() {
        return index;
    }

    public void setIndex(int index) {
        this.index = index;
    }

    public String getHintShow1() {
        return hintShow1;
    }

    public void setHintShow1(String hintShow1) {
        this.hintShow1 = hintShow1;
    }

    public String getHintShow2() {
        return hintShow2;
    }

    public void setHintShow2(String hintShow2) {
        this.hintShow2 = hintShow2;
    }

    public String getHintChose() {
        return hintChose;
    }

    public void setHintChose(String hintChose) {
        this.hintChose = hintChose;
    }

    public String getText1() {
        return text1;
    }

    public void setText1(String text1) {
        this.text1 = text1;
    }

    public String getText2() {
        return text2;
    }

    public void setText2(String text2) {
        this.text2 = text2;
    }

    @Override
    public int compareTo(PersonalInfoItemEntity o) {
        return this.index - o.index;
    }

    @Override
    public int describeContents() {
        return 0;
    }

    @Override
    public void writeToParcel(Parcel dest, int flags) {
        dest.writeInt(this.index);
        dest.writeString(this.hintShow1);
        dest.writeString(this.hintShow2);
        dest.writeString(this.hintChose);
        dest.writeString(this.text1);
        dest.writeString(this.text2);
    }

    public PersonalInfoItemEntity() {
    }

    protected PersonalInfoItemEntity(Parcel in) {
        this.index = in.readInt();
        this.hintShow1 = in.readString();
        this.hintShow2 = in.readString();
        this.hintChose = in.readString();
        this.text1 = in.readString();
        this.text2 = in.readString();
    }

    public static final Parcelable.Creator<PersonalInfoItemEntity> CREATOR = new Parcelable.Creator<PersonalInfoItemEntity>() {
        @Override
        public PersonalInfoItemEntity createFromParcel(Parcel source) {
            return new PersonalInfoItemEntity(source);
        }

        @Override
        public PersonalInfoItemEntity[] newArray(int size) {
            return new PersonalInfoItemEntity[size];
        }
    };

    @Override
    public boolean equals(@Nullable Object obj) {
        return this.getIndex() == ((PersonalInfoItemEntity) obj).getIndex();
    }
}
