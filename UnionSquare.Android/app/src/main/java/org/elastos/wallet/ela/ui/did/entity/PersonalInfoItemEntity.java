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

public class PersonalInfoItemEntity implements Comparable<PersonalInfoItemEntity>, Parcelable {
    private int index;
    private String hintShow1;//初始化后不会再修改 提示语句
    private String hintShow2;//初始化后不会再修改 提示语句
    private String hintChose;//初始化后不会再修改 提示语句
    private String text1;//第一个可编辑的结果
    private String text2;//第二个可编辑的结果
    private boolean check;//第二个可编辑的结果

    public boolean isCheck() {
        return check;
    }

    public void setCheck(boolean check) {
        this.check = check;
    }

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

    public PersonalInfoItemEntity() {
    }

    @Override
    public boolean equals(@Nullable Object obj) {
        return this.getIndex() == ((PersonalInfoItemEntity) obj).getIndex();
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
        dest.writeByte(this.check ? (byte) 1 : (byte) 0);
    }

    protected PersonalInfoItemEntity(Parcel in) {
        this.index = in.readInt();
        this.hintShow1 = in.readString();
        this.hintShow2 = in.readString();
        this.hintChose = in.readString();
        this.text1 = in.readString();
        this.text2 = in.readString();
        this.check = in.readByte() != 0;
    }

    public static final Creator<PersonalInfoItemEntity> CREATOR = new Creator<PersonalInfoItemEntity>() {
        @Override
        public PersonalInfoItemEntity createFromParcel(Parcel source) {
            return new PersonalInfoItemEntity(source);
        }

        @Override
        public PersonalInfoItemEntity[] newArray(int size) {
            return new PersonalInfoItemEntity[size];
        }
    };
}
