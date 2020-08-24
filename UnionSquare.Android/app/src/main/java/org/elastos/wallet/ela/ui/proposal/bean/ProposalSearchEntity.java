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

import com.google.gson.annotations.SerializedName;

import org.elastos.wallet.ela.rxjavahelp.BaseEntity;

import java.util.List;

public class ProposalSearchEntity extends BaseEntity {

    /**
     * code : 1
     * data : {"list":[{"id":75,"status":"VOTING","proposalHash":"62f578434da472bca175111cd24168c43dd41162cdf541bc52a384600cae9bb8","title":"test 3","createdAt":1597818647,"proposedBy":"newuser"},{"id":74,"status":"NOTIFICATION","proposalHash":"6c53cfec966cd779b340f2fcc82d810bb5e425055c85970eb1e7dbd34c9a19f2","title":"suggestion test 0818   Elastos Blockchain（亦来云区块链）是奠定Elastos生态信任和价值传递的基础，Elastos Blockchain采用主侧链双层结构：主链通过与比特币联合挖矿共享算力，采用PoW+DPoS联合共识机制保证可信和安全。集群侧链可灵活拓展区块链能力，侧链可以兼容以太坊、NEO等多种智能合约，实现区块数据的可信传递","createdAt":1597804477,"proposedBy":"qafangjj","rejectAmount":"0","rejectThroughAmount":"3201281.68375288","rejectRatio":0},{"id":73,"status":"ACTIVE","proposalHash":"fd0415ca0e00cb340a48817a5852c611b98287e56994ff4917f74b0dd920c6f0","title":"Test0817bbbbb","createdAt":1597715326,"proposedBy":"cr09-did"},{"id":72,"status":"ACTIVE","proposalHash":"a5557bd79bbcb1b1b6a753ef890601d6eea8734f4a921874e0f8ba071b1c9f54","title":"Test0817aaaaaa","createdAt":1597715325,"proposedBy":"cr10-did"},{"id":71,"status":"REJECTED","proposalHash":"d0daa12fef1fcdda358bdcd0d5060936b5b70e6fafb1263b72322127d5422d00","title":"0813","createdAt":1597310824,"proposedBy":"大琳琳"},{"id":70,"status":"FINAL","proposalHash":"11c538afa97785f26ccd20bd9a36519ad52b0dbda7428fbbddb0f911b91cdd17","title":"picture test","createdAt":1597198476,"proposedBy":"cr06-did"},{"id":69,"status":"FINAL","proposalHash":"df93ccccf7c697710fb0a6854b831023a731b6ea0813812465ecdec7763c2642","title":"Translation test","createdAt":1597198458,"proposedBy":"cr06-did"},{"id":68,"status":"REJECTED","proposalHash":"3cca7c9647f2a0482e71d9cd106fe9ecfd5fe688c52210bdc2d2ea5f9ee4c566","title":"Vote history test 2","createdAt":1596439832,"proposedBy":"cr06-did"},{"id":67,"status":"ACTIVE","proposalHash":"0054df6682979a497fe6b23e4858966a8dbe6f2e4c7816abf679cee5c779f93e","title":"Vote history test 1","createdAt":1596424967,"proposedBy":"cr06-did"},{"id":66,"status":"FINAL","proposalHash":"add0559873108b025de0c24a6b2caa035f3d433dc9117158d5825f82851339da","title":"community test2","createdAt":1596070286,"proposedBy":"cr01-did"},{"id":65,"status":"ACTIVE","proposalHash":"cbcd1191c11f3924f4b6930a597990a8d09f91f27a9fbada7994020ba2ef83e3","title":"community test1","createdAt":1596070273,"proposedBy":"cr01-did"},{"id":64,"status":"FINAL","proposalHash":"28808822a93a43f3de4966b7fe4e1402cb38ae3c329d1300707c2c56bfdb67f0","title":"test 0729 update1","createdAt":1595992225,"proposedBy":"qafangjj"},{"id":63,"status":"ACTIVE","proposalHash":"7b9cf80d9903d9b1e0413b9b6c714542d91418f1bf6826d87900f24590173068","title":"Test07271905","createdAt":1595853651,"proposedBy":"cr10-did"},{"id":62,"status":"ACTIVE","proposalHash":"f414d25386444643ba1676ebfcf5482c6feb3d02599f61452ea600899d8d7bee","title":"Test07271853","createdAt":1595850612,"proposedBy":"cr10-did"},{"id":61,"status":"ACTIVE","proposalHash":"12fd68970d57521ad53c16d9608ae200e9ecab85ab3b399f40c4e77e9b8e149f","title":"test","createdAt":1595826382,"proposedBy":"cr06-did"},{"id":60,"status":"VETOED","proposalHash":"9eecaaebfa9e8f1ede29ea9c8bb1acf93ab2828e04b4a61b203aa5c369531307","title":"Community vetoed test3","createdAt":1595570225,"proposedBy":"qafangjj"},{"id":59,"status":"VETOED","proposalHash":"b71c0cf0d6bb18efafbe50e7fb2e5282944f8b81d81205abd40dff890b1ad63b","title":"Community vetoed test2 0723","createdAt":1595493560,"proposedBy":"qafangjj"},{"id":58,"status":"VETOED","proposalHash":"efb4c8ab91130bfafe706f97edbc2b25852a4f86c4189ff89ea5cfcb38c2d9d4","title":"Community vetoed test","createdAt":1595491632,"proposedBy":"cr01-did"},{"id":57,"status":"REJECTED","proposalHash":"926588b0dd93e49a1310249a6ea50d631e48dc1cb3b009218b0fb01ab638d8f2","title":"7.23测试","createdAt":1595473635,"proposedBy":"大琳琳"},{"id":56,"status":"REJECTED","proposalHash":"7e0e22f0808bbd3de99f25e6e24c80d30c2b8e03f5c579ceec9eec3f8871aeb9","title":"QA TEST 3 0721","createdAt":1595387157,"proposedBy":"cr01-did"}],"total":75}
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
         * list : [{"id":75,"status":"VOTING","proposalHash":"62f578434da472bca175111cd24168c43dd41162cdf541bc52a384600cae9bb8","title":"test 3","createdAt":1597818647,"proposedBy":"newuser"},{"id":74,"status":"NOTIFICATION","proposalHash":"6c53cfec966cd779b340f2fcc82d810bb5e425055c85970eb1e7dbd34c9a19f2","title":"suggestion test 0818   Elastos Blockchain（亦来云区块链）是奠定Elastos生态信任和价值传递的基础，Elastos Blockchain采用主侧链双层结构：主链通过与比特币联合挖矿共享算力，采用PoW+DPoS联合共识机制保证可信和安全。集群侧链可灵活拓展区块链能力，侧链可以兼容以太坊、NEO等多种智能合约，实现区块数据的可信传递","createdAt":1597804477,"proposedBy":"qafangjj","rejectAmount":"0","rejectThroughAmount":"3201281.68375288","rejectRatio":0},{"id":73,"status":"ACTIVE","proposalHash":"fd0415ca0e00cb340a48817a5852c611b98287e56994ff4917f74b0dd920c6f0","title":"Test0817bbbbb","createdAt":1597715326,"proposedBy":"cr09-did"},{"id":72,"status":"ACTIVE","proposalHash":"a5557bd79bbcb1b1b6a753ef890601d6eea8734f4a921874e0f8ba071b1c9f54","title":"Test0817aaaaaa","createdAt":1597715325,"proposedBy":"cr10-did"},{"id":71,"status":"REJECTED","proposalHash":"d0daa12fef1fcdda358bdcd0d5060936b5b70e6fafb1263b72322127d5422d00","title":"0813","createdAt":1597310824,"proposedBy":"大琳琳"},{"id":70,"status":"FINAL","proposalHash":"11c538afa97785f26ccd20bd9a36519ad52b0dbda7428fbbddb0f911b91cdd17","title":"picture test","createdAt":1597198476,"proposedBy":"cr06-did"},{"id":69,"status":"FINAL","proposalHash":"df93ccccf7c697710fb0a6854b831023a731b6ea0813812465ecdec7763c2642","title":"Translation test","createdAt":1597198458,"proposedBy":"cr06-did"},{"id":68,"status":"REJECTED","proposalHash":"3cca7c9647f2a0482e71d9cd106fe9ecfd5fe688c52210bdc2d2ea5f9ee4c566","title":"Vote history test 2","createdAt":1596439832,"proposedBy":"cr06-did"},{"id":67,"status":"ACTIVE","proposalHash":"0054df6682979a497fe6b23e4858966a8dbe6f2e4c7816abf679cee5c779f93e","title":"Vote history test 1","createdAt":1596424967,"proposedBy":"cr06-did"},{"id":66,"status":"FINAL","proposalHash":"add0559873108b025de0c24a6b2caa035f3d433dc9117158d5825f82851339da","title":"community test2","createdAt":1596070286,"proposedBy":"cr01-did"},{"id":65,"status":"ACTIVE","proposalHash":"cbcd1191c11f3924f4b6930a597990a8d09f91f27a9fbada7994020ba2ef83e3","title":"community test1","createdAt":1596070273,"proposedBy":"cr01-did"},{"id":64,"status":"FINAL","proposalHash":"28808822a93a43f3de4966b7fe4e1402cb38ae3c329d1300707c2c56bfdb67f0","title":"test 0729 update1","createdAt":1595992225,"proposedBy":"qafangjj"},{"id":63,"status":"ACTIVE","proposalHash":"7b9cf80d9903d9b1e0413b9b6c714542d91418f1bf6826d87900f24590173068","title":"Test07271905","createdAt":1595853651,"proposedBy":"cr10-did"},{"id":62,"status":"ACTIVE","proposalHash":"f414d25386444643ba1676ebfcf5482c6feb3d02599f61452ea600899d8d7bee","title":"Test07271853","createdAt":1595850612,"proposedBy":"cr10-did"},{"id":61,"status":"ACTIVE","proposalHash":"12fd68970d57521ad53c16d9608ae200e9ecab85ab3b399f40c4e77e9b8e149f","title":"test","createdAt":1595826382,"proposedBy":"cr06-did"},{"id":60,"status":"VETOED","proposalHash":"9eecaaebfa9e8f1ede29ea9c8bb1acf93ab2828e04b4a61b203aa5c369531307","title":"Community vetoed test3","createdAt":1595570225,"proposedBy":"qafangjj"},{"id":59,"status":"VETOED","proposalHash":"b71c0cf0d6bb18efafbe50e7fb2e5282944f8b81d81205abd40dff890b1ad63b","title":"Community vetoed test2 0723","createdAt":1595493560,"proposedBy":"qafangjj"},{"id":58,"status":"VETOED","proposalHash":"efb4c8ab91130bfafe706f97edbc2b25852a4f86c4189ff89ea5cfcb38c2d9d4","title":"Community vetoed test","createdAt":1595491632,"proposedBy":"cr01-did"},{"id":57,"status":"REJECTED","proposalHash":"926588b0dd93e49a1310249a6ea50d631e48dc1cb3b009218b0fb01ab638d8f2","title":"7.23测试","createdAt":1595473635,"proposedBy":"大琳琳"},{"id":56,"status":"REJECTED","proposalHash":"7e0e22f0808bbd3de99f25e6e24c80d30c2b8e03f5c579ceec9eec3f8871aeb9","title":"QA TEST 3 0721","createdAt":1595387157,"proposedBy":"cr01-did"}]
         * total : 75
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
             * id : 75
             * status : VOTING
             * proposalHash : 62f578434da472bca175111cd24168c43dd41162cdf541bc52a384600cae9bb8
             * title : test 3
             * createdAt : 1597818647
             * proposedBy : newuser
             * rejectAmount : 0
             * rejectThroughAmount : 3201281.68375288
             * rejectRatio : 0
             */

            private int id;
            private String title;
            private String status;
            private int createdAt;
            private String proposedBy;
            private String proposalHash;
            private String rejectAmount;
            private String rejectThroughAmount;
            private int rejectRatio;
            private String votes;

            public String getVotes() {
                return votes;
            }

            public void setVotes(String votes) {
                this.votes = votes;
            }

            public int getId() {
                return id;
            }

            public void setId(int id) {
                this.id = id;
            }

            public String getStatus() {
                return status;
            }

            public void setStatus(String status) {
                this.status = status;
            }

            public String getProposalHash() {
                return proposalHash;
            }

            public void setProposalHash(String proposalHash) {
                this.proposalHash = proposalHash;
            }

            public String getTitle() {
                return title;
            }

            public void setTitle(String title) {
                this.title = title;
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

            public String getRejectAmount() {
                return rejectAmount;
            }

            public void setRejectAmount(String rejectAmount) {
                this.rejectAmount = rejectAmount;
            }

            public String getRejectThroughAmount() {
                return rejectThroughAmount;
            }

            public void setRejectThroughAmount(String rejectThroughAmount) {
                this.rejectThroughAmount = rejectThroughAmount;
            }

            public int getRejectRatio() {
                return rejectRatio;
            }

            public void setRejectRatio(int rejectRatio) {
                this.rejectRatio = rejectRatio;
            }

            public ListBean() {
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
                dest.writeString(this.rejectAmount);
                dest.writeString(this.rejectThroughAmount);
                dest.writeInt(this.rejectRatio);
                dest.writeString(this.votes);
            }

            protected ListBean(Parcel in) {
                this.id = in.readInt();
                this.title = in.readString();
                this.status = in.readString();
                this.createdAt = in.readInt();
                this.proposedBy = in.readString();
                this.proposalHash = in.readString();
                this.rejectAmount = in.readString();
                this.rejectThroughAmount = in.readString();
                this.rejectRatio = in.readInt();
                this.votes = in.readString();
            }

            public static final Creator<ListBean> CREATOR = new Creator<ListBean>() {
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
