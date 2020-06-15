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
public class RecieveReviewJwtEntity extends RecieveProposalFatherJwtEntity {


    /**
     * iat : 1566352213
     * exp : 1580607089
     * data : {"proposalhash":"9b6c85d10b1208d896eb7efc391d1a562ba91452a78f37acfbb1ac1ed63f1a83","voteresult":"approve","opinionhash":"fc44baa69cfcca1c6f5bb9451555034a0244e50b511dadc3454f77ab24da1a27","DID":"did:elastos:inDUQR73UQLFfgZocbC3PH4SFiRggffcNw"}
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
         * voteresult : approve
         * opinionhash : fc44baa69cfcca1c6f5bb9451555034a0244e50b511dadc3454f77ab24da1a27
         * DID : did:elastos:inDUQR73UQLFfgZocbC3PH4SFiRggffcNw
         */

        private String proposalhash;
        private String voteresult;
        private String opinionhash;
        private String DID;

        public String getProposalhash() {
            return proposalhash;
        }

        public void setProposalhash(String proposalhash) {
            this.proposalhash = proposalhash;
        }

        public String getVoteresult() {
            return voteresult;
        }

        public void setVoteresult(String voteresult) {
            this.voteresult = voteresult;
        }

        public String getOpinionhash() {
            return opinionhash;
        }

        public void setOpinionhash(String opinionhash) {
            this.opinionhash = opinionhash;
        }

        public String getDID() {
            return DID;
        }

        public void setDID(String DID) {
            this.DID = DID;
        }

        @Override
        public int describeContents() {
            return 0;
        }

        @Override
        public void writeToParcel(Parcel dest, int flags) {
            dest.writeString(this.proposalhash);
            dest.writeString(this.voteresult);
            dest.writeString(this.opinionhash);
            dest.writeString(this.DID);
        }

        public DataBean() {
        }

        protected DataBean(Parcel in) {
            this.proposalhash = in.readString();
            this.voteresult = in.readString();
            this.opinionhash = in.readString();
            this.DID = in.readString();
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
