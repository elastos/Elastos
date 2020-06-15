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

package org.elastos.wallet.ela.ui.Assets.bean.qr.proposal;

import android.os.Parcel;
import android.os.Parcelable;

/**
 * 兼顾提建议和提案
 */
public class RecieveProcessJwtEntity extends RecieveProposalFatherJwtEntity {


    /**
     * iat : 1566352213
     * exp : 1580607089
     * data : {"proposalhash":"9b6c85d10b1208d896eb7efc391d1a562ba91452a78f37acfbb1ac1ed63f1a83","messagehash":"6739263b511de00b49e08855254d46dbade53ca10c3e10babfb79b8196032464","stage":1,"ownerpubkey":"023559273eec17bbfcedd041d2044163123a9bba34530540d864a6f3f484f7054a","newownerpubkey":"","ownersignature":"0d77e801761f45628270d6f7a86b02def1f35b1c8964a4094b7947730c86b63d0ee1e8e448192d6a0ac74e1c592ee27c3b6d9b7010a5830dbff9cb506d03d148","newownersignature":"","proposaltrackingtype":"progress","secretaryopinionhash":"0a429d8d18afb6e940de1df2a8e65eb8a377a9adbf4f3169fc61309e6caf4862"}
     */


    private DataBean data;

    public DataBean getData() {
        return data;
    }

    public void setData(DataBean data) {
        this.data = data;
    }

    public static class DataBean implements Parcelable {
        /**
         * proposalhash : 9b6c85d10b1208d896eb7efc391d1a562ba91452a78f37acfbb1ac1ed63f1a83
         * messagehash : 6739263b511de00b49e08855254d46dbade53ca10c3e10babfb79b8196032464
         * stage : 1
         * ownerpubkey : 023559273eec17bbfcedd041d2044163123a9bba34530540d864a6f3f484f7054a
         * newownerpubkey :
         * ownersignature : 0d77e801761f45628270d6f7a86b02def1f35b1c8964a4094b7947730c86b63d0ee1e8e448192d6a0ac74e1c592ee27c3b6d9b7010a5830dbff9cb506d03d148
         * newownersignature :
         * proposaltrackingtype : progress
         * secretaryopinionhash : 0a429d8d18afb6e940de1df2a8e65eb8a377a9adbf4f3169fc61309e6caf4862
         */

        private String proposalhash;
        private String messagehash;
        private int stage;
        private String ownerpubkey;
        private String newownerpubkey;
        private String ownersignature;
        private String newownersignature;
        private String proposaltrackingtype;
        private String secretaryopinionhash;

        public String getProposalhash() {
            return proposalhash;
        }

        public void setProposalhash(String proposalhash) {
            this.proposalhash = proposalhash;
        }

        public String getMessagehash() {
            return messagehash;
        }

        public void setMessagehash(String messagehash) {
            this.messagehash = messagehash;
        }

        public int getStage() {
            return stage;
        }

        public void setStage(int stage) {
            this.stage = stage;
        }

        public String getOwnerpubkey() {
            return ownerpubkey;
        }

        public void setOwnerpubkey(String ownerpubkey) {
            this.ownerpubkey = ownerpubkey;
        }

        public String getNewownerpubkey() {
            return newownerpubkey;
        }

        public void setNewownerpubkey(String newownerpubkey) {
            this.newownerpubkey = newownerpubkey;
        }

        public String getOwnersignature() {
            return ownersignature;
        }

        public void setOwnersignature(String ownersignature) {
            this.ownersignature = ownersignature;
        }

        public String getNewownersignature() {
            return newownersignature;
        }

        public void setNewownersignature(String newownersignature) {
            this.newownersignature = newownersignature;
        }

        public String getProposaltrackingtype() {
            return proposaltrackingtype;
        }

        public void setProposaltrackingtype(String proposaltrackingtype) {
            this.proposaltrackingtype = proposaltrackingtype;
        }

        public String getSecretaryopinionhash() {
            return secretaryopinionhash;
        }

        public void setSecretaryopinionhash(String secretaryopinionhash) {
            this.secretaryopinionhash = secretaryopinionhash;
        }

        @Override
        public int describeContents() {
            return 0;
        }

        @Override
        public void writeToParcel(Parcel dest, int flags) {
            dest.writeString(this.proposalhash);
            dest.writeString(this.messagehash);
            dest.writeInt(this.stage);
            dest.writeString(this.ownerpubkey);
            dest.writeString(this.newownerpubkey);
            dest.writeString(this.ownersignature);
            dest.writeString(this.newownersignature);
            dest.writeString(this.proposaltrackingtype);
            dest.writeString(this.secretaryopinionhash);
        }

        public DataBean() {
        }

        protected DataBean(Parcel in) {
            this.proposalhash = in.readString();
            this.messagehash = in.readString();
            this.stage = in.readInt();
            this.ownerpubkey = in.readString();
            this.newownerpubkey = in.readString();
            this.ownersignature = in.readString();
            this.newownersignature = in.readString();
            this.proposaltrackingtype = in.readString();
            this.secretaryopinionhash = in.readString();
        }

        public static final Parcelable.Creator<DataBean> CREATOR = new Parcelable.Creator<DataBean>() {
            @Override
            public DataBean createFromParcel(Parcel source) {
                return new DataBean(source);
            }

            @Override
            public DataBean[] newArray(int size) {
                return new DataBean[size];
            }
        };
    }
}
