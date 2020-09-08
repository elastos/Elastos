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

package org.elastos.wallet.ela.ui.committee.bean;

import android.os.Parcel;
import android.os.Parcelable;

import org.elastos.wallet.ela.rxjavahelp.BaseEntity;

import java.util.List;

public class CtListBean extends BaseEntity {

    private DataBean data;

    public DataBean getData() {
        return data;
    }

    public void setData(DataBean data) {
        this.data = data;
    }

    public static class DataBean {
        private List<Council> council;
        private List<Secretariat> secretariat;

        public List<Council> getCouncil() {
            return council;
        }

        public void setCouncil(List<Council> council) {
            this.council = council;
        }

        public List<Secretariat> getSecretariat() {
            return secretariat;
        }

        public void setSecretariat(List<Secretariat> secretariat) {
            this.secretariat = secretariat;
        }
    }

    public static class Secretariat {
        private String did;
        private String didName;
        private String avatar;
        private int location;
        private long startDate;
        private long endDate;
        private String status;

        public String getDid() {
            return did;
        }

        public void setDid(String did) {
            this.did = did;
        }

        public String getDidName() {
            return didName;
        }

        public void setDidName(String didName) {
            this.didName = didName;
        }

        public String getAvatar() {
            return avatar;
        }

        public void setAvatar(String avatar) {
            this.avatar = avatar;
        }

        public int getLocation() {
            return location;
        }

        public void setLocation(int location) {
            this.location = location;
        }

        public long getStartDate() {
            return startDate;
        }

        public void setStartDate(long startDate) {
            this.startDate = startDate;
        }

        public long getEndDate() {
            return endDate;
        }

        public void setEndDate(long endDate) {
            this.endDate = endDate;
        }

        public String getStatus() {
            return status;
        }

        public void setStatus(String status) {
            this.status = status;
        }
    }

    public static class Council implements Parcelable {
        /**
         * cid : iZrJ54tu9EdznPFHgXfd8w1eMmRX3rrES8
         * did : iftZraCKy5kZ9Mmu1eMnEFT9NvgrKg3Jk7
         * location : 376
         * impeachmentVotes : 0
         * status : Elected
         * didName : kongx
         * rejectRatio : 0
         */

        private String cid;
        private String did;
        private String didName;
        private String avatar;
        private int location;
        private String status; //'Elected', 'Impeached', 'Returned'
        private String votes;
        private int impeachmentVotes;
        private float rejectRatio;

        public String getVotes() {
            return votes;
        }

        public void setVotes(String votes) {
            this.votes = votes;
        }

        public String getDid() {
            return did;
        }

        public void setDid(String did) {
            this.did = did;
        }

        public String getDidName() {
            return didName;
        }

        public void setDidName(String didName) {
            this.didName = didName;
        }

        public String getAvatar() {
            return avatar;
        }

        public void setAvatar(String avatar) {
            this.avatar = avatar;
        }

        public int getLocation() {
            return location;
        }

        public void setLocation(int location) {
            this.location = location;
        }

        public String getStatus() {
            return status;
        }

        public void setStatus(String status) {
            this.status = status;
        }

        public Council() {
        }
        public String getCid() {
            return cid;
        }

        public void setCid(String cid) {
            this.cid = cid;
        }

        public float getRejectRatio() {
            return rejectRatio;
        }

        public void setRejectRatio(float rejectRatio) {
            this.rejectRatio = rejectRatio;
        }

        public int getImpeachmentVotes() {
            return impeachmentVotes;
        }

        public void setImpeachmentVotes(int impeachmentVotes) {
            this.impeachmentVotes = impeachmentVotes;
        }


        @Override
        public int describeContents() {
            return 0;
        }

        @Override
        public void writeToParcel(Parcel dest, int flags) {
            dest.writeString(this.cid);
            dest.writeString(this.did);
            dest.writeString(this.didName);
            dest.writeString(this.avatar);
            dest.writeInt(this.location);
            dest.writeString(this.status);
            dest.writeString(this.votes);
            dest.writeInt(this.impeachmentVotes);
            dest.writeFloat(this.rejectRatio);
        }

        protected Council(Parcel in) {
            this.cid = in.readString();
            this.did = in.readString();
            this.didName = in.readString();
            this.avatar = in.readString();
            this.location = in.readInt();
            this.status = in.readString();
            this.votes = in.readString();
            this.impeachmentVotes = in.readInt();
            this.rejectRatio = in.readFloat();
        }

        public static final Creator<Council> CREATOR = new Creator<Council>() {
            @Override
            public Council createFromParcel(Parcel source) {
                return new Council(source);
            }

            @Override
            public Council[] newArray(int size) {
                return new Council[size];
            }
        };
    }

}
