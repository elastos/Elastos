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

package org.elastos.wallet.ela.ui.proposal.bean;

import android.os.Parcel;
import android.os.Parcelable;

import org.elastos.wallet.ela.rxjavahelp.BaseEntity;

import java.util.List;

public class ProposalSearchEntity extends BaseEntity {
    /**
     * code : 1
     * data : {"list":[{"id":2,"title":"user02的建议","status":"ACTIVE","createdAt":1589271912,"proposedBy":"user02","proposalHash":"5aaC5p6c5L2g6KeJ5b6X5LiN6ZSZ77yM5Y+v5Lul5YiG5Lqr57uZpy"}],"total":0}
     */


    private DataBean data;

    public DataBean getData() {
        return data;
    }

    public void setData(DataBean data) {
        this.data = data;
    }

    public static class DataBean {
        /**
         * list : [{"id":2,"title":"user02的建议","status":"ACTIVE","createdAt":1589271912,"proposedBy":"user02","proposalHash":"5aaC5p6c5L2g6KeJ5b6X5LiN6ZSZ77yM5Y+v5Lul5YiG5Lqr57uZpy"}]
         * total : 0
         */

        private int total;
        private List<ListBean> list;

        public int getTotal() {
            return total;
        }

        public void setTotal(int total) {
            this.total = total;
        }

        public List<ListBean> getList() {
            return list;
        }

        public void setList(List<ListBean> list) {
            this.list = list;
        }

        public static class ListBean implements Parcelable {
            /**
             * id : 2
             * title : user02的建议
             * status : ACTIVE
             * createdAt : 1589271912
             * proposedBy : user02
             * proposalHash : 5aaC5p6c5L2g6KeJ5b6X5LiN6ZSZ77yM5Y+v5Lul5YiG5Lqr57uZpy
             */

            private int id;
            private String title;
            private String status;
            private int createdAt;
            private String proposedBy;
            private String proposalHash;

            public int getId() {
                return id;
            }

            public void setId(int id) {
                this.id = id;
            }

            public String getTitle() {
                return title;
            }

            public void setTitle(String title) {
                this.title = title;
            }

            public String getStatus() {
                return status;
            }

            public void setStatus(String status) {
                this.status = status;
            }

            public int getCreatedAt() {
                return createdAt;
            }

            public void setCreatedAt(int createdAt) {
                this.createdAt = createdAt;
            }

            public String getProposedBy() {
                return proposedBy;
            }

            public void setProposedBy(String proposedBy) {
                this.proposedBy = proposedBy;
            }

            public String getProposalHash() {
                return proposalHash;
            }

            public void setProposalHash(String proposalHash) {
                this.proposalHash = proposalHash;
            }

            @Override
            public int describeContents() {
                return 0;
            }

            @Override
            public void writeToParcel(Parcel dest, int flags) {
                dest.writeInt(this.id);
                dest.writeString(this.title);
                dest.writeString(this.status);
                dest.writeInt(this.createdAt);
                dest.writeString(this.proposedBy);
                dest.writeString(this.proposalHash);
            }

            public ListBean() {
            }

            protected ListBean(Parcel in) {
                this.id = in.readInt();
                this.title = in.readString();
                this.status = in.readString();
                this.createdAt = in.readInt();
                this.proposedBy = in.readString();
                this.proposalHash = in.readString();
            }

            public static final Parcelable.Creator<ListBean> CREATOR = new Parcelable.Creator<ListBean>() {
                @Override
                public ListBean createFromParcel(Parcel source) {
                    return new ListBean(source);
                }

                @Override
                public ListBean[] newArray(int size) {
                    return new ListBean[size];
                }
            };
        }
    }
}
