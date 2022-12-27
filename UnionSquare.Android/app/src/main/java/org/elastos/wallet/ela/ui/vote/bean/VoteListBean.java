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

package org.elastos.wallet.ela.ui.vote.bean;

import android.os.Parcel;
import android.os.Parcelable;
import android.support.annotation.NonNull;

import org.elastos.wallet.ela.rxjavahelp.BaseEntity;

import java.io.Serializable;
import java.util.List;

public class VoteListBean extends BaseEntity implements Serializable {

    @Override
    public String toString() {
        return "VoteListBean{" +
                ", data=" + data +
                ", exceptionMsg=" + exceptionMsg +
                '}';
    }

    /**
     * message : Query successful ^_^
     * data : {"error":null,"id":null,"jsonrpc":"2.0","result":{"producers":[{"ownerpublickey":"035ecacbdacb997ef284bf9d4d62ff1a07cee034be455c2a2b5b2435651fd4716b","nodepublickey":"035ecacbdacb997ef284bf9d4d62ff1a07cee034be455c2a2b5b2435651fd4716b","nickname":"r_adr_us00","url":"www.r_adr_us00.com","location":86,"active":true,"votes":"18071.07053744","state":"Active","registerheight":317077,"cancelheight":0,"inactiveheight":0,"illegalheight":0,"index":34,"voterate":"0.001094719222928"},{"ownerpublickey":"026482b31d6cf94d8d5529414192ff37535984dd9810368a463cc33cbb40e57823","nodepublickey":"026482b31d6cf94d8d5529414192ff37535984dd9810368a463cc33cbb40e57823","nickname":"呵呵","url":"www.12306.cn","location":82,"active":false,"votes":"15052","state":"Returned","registerheight":190506,"cancelheight":197102,"inactiveheight":0,"illegalheight":0,"index":35,"voterate":"0.000911828311963"},{"ownerpublickey":"0320c4ae2c4748371eb9b3ade9523e229555541f7a068ded91de7d68cac3460bb1","nodepublickey":"ffff","nickname":"田","url":"qw","location":244,"active":false,"votes":"0","state":"Returned","registerheight":193107,"cancelheight":193757,"inactiveheight":0,"illegalheight":0,"index":80,"voterate":"0.000000000000000"}],"totalvotes":"16507493.57365232","totalcounts":81,"totalvoterate":"0.093102692892006"}}
     * exceptionMsg : null
     */



    private DataBean data;
    private Object exceptionMsg;




    public DataBean getData() {
        return data;
    }

    public void setData(DataBean data) {
        this.data = data;
    }

    public Object getExceptionMsg() {
        return exceptionMsg;
    }

    public void setExceptionMsg(Object exceptionMsg) {
        this.exceptionMsg = exceptionMsg;
    }

    public static class DataBean implements Serializable {
        @Override
        public String toString() {
            return "DataBean{" +
                    "error=" + error +
                    ", id=" + id +
                    ", jsonrpc='" + jsonrpc + '\'' +
                    ", result=" + result +
                    '}';
        }

        /**
         * error : null
         * id : null
         * jsonrpc : 2.0
         * result : {"producers":[{"ownerpublickey":"035ecacbdacb997ef284bf9d4d62ff1a07cee034be455c2a2b5b2435651fd4716b","nodepublickey":"035ecacbdacb997ef284bf9d4d62ff1a07cee034be455c2a2b5b2435651fd4716b","nickname":"r_adr_us00","url":"www.r_adr_us00.com","location":86,"active":true,"votes":"18071.07053744","state":"Active","registerheight":317077,"cancelheight":0,"inactiveheight":0,"illegalheight":0,"index":34,"voterate":"0.001094719222928"},{"ownerpublickey":"026482b31d6cf94d8d5529414192ff37535984dd9810368a463cc33cbb40e57823","nodepublickey":"026482b31d6cf94d8d5529414192ff37535984dd9810368a463cc33cbb40e57823","nickname":"呵呵","url":"www.12306.cn","location":82,"active":false,"votes":"15052","state":"Returned","registerheight":190506,"cancelheight":197102,"inactiveheight":0,"illegalheight":0,"index":35,"voterate":"0.000911828311963"},{"ownerpublickey":"0320c4ae2c4748371eb9b3ade9523e229555541f7a068ded91de7d68cac3460bb1","nodepublickey":"ffff","nickname":"田","url":"qw","location":244,"active":false,"votes":"0","state":"Returned","registerheight":193107,"cancelheight":193757,"inactiveheight":0,"illegalheight":0,"index":80,"voterate":"0.000000000000000"}],"totalvotes":"16507493.57365232","totalcounts":81,"totalvoterate":"0.093102692892006"}
         */

        private Object error;
        private Object id;
        private String jsonrpc;
        private ResultBean result;

        public Object getError() {
            return error;
        }

        public void setError(Object error) {
            this.error = error;
        }

        public Object getId() {
            return id;
        }

        public void setId(Object id) {
            this.id = id;
        }

        public String getJsonrpc() {
            return jsonrpc;
        }

        public void setJsonrpc(String jsonrpc) {
            this.jsonrpc = jsonrpc;
        }

        public ResultBean getResult() {
            return result;
        }

        public void setResult(ResultBean result) {
            this.result = result;
        }

        public static class ResultBean implements Serializable {
            /**
             * producers : [{"ownerpublickey":"035ecacbdacb997ef284bf9d4d62ff1a07cee034be455c2a2b5b2435651fd4716b","nodepublickey":"035ecacbdacb997ef284bf9d4d62ff1a07cee034be455c2a2b5b2435651fd4716b","nickname":"r_adr_us00","url":"www.r_adr_us00.com","location":86,"active":true,"votes":"18071.07053744","state":"Active","registerheight":317077,"cancelheight":0,"inactiveheight":0,"illegalheight":0,"index":34,"voterate":"0.001094719222928"},{"ownerpublickey":"026482b31d6cf94d8d5529414192ff37535984dd9810368a463cc33cbb40e57823","nodepublickey":"026482b31d6cf94d8d5529414192ff37535984dd9810368a463cc33cbb40e57823","nickname":"呵呵","url":"www.12306.cn","location":82,"active":false,"votes":"15052","state":"Returned","registerheight":190506,"cancelheight":197102,"inactiveheight":0,"illegalheight":0,"index":35,"voterate":"0.000911828311963"},{"ownerpublickey":"0320c4ae2c4748371eb9b3ade9523e229555541f7a068ded91de7d68cac3460bb1","nodepublickey":"ffff","nickname":"田","url":"qw","location":244,"active":false,"votes":"0","state":"Returned","registerheight":193107,"cancelheight":193757,"inactiveheight":0,"illegalheight":0,"index":80,"voterate":"0.000000000000000"}]
             * totalvotes : 16507493.57365232
             * totalcounts : 81
             * totalvoterate : 0.093102692892006
             */

            private String totalvotes;
            private int totalcounts;
            private String totalvoterate;
            private List<ProducersBean> producers;

            public String getTotalvotes() {
                return totalvotes;
            }

            public void setTotalvotes(String totalvotes) {
                this.totalvotes = totalvotes;
            }

            public int getTotalcounts() {
                return totalcounts;
            }

            public void setTotalcounts(int totalcounts) {
                this.totalcounts = totalcounts;
            }

            public String getTotalvoterate() {
                return totalvoterate;
            }

            public void setTotalvoterate(String totalvoterate) {
                this.totalvoterate = totalvoterate;
            }

            public List<ProducersBean> getProducers() {
                return producers;
            }

            public void setProducers(List<ProducersBean> producers) {
                this.producers = producers;
            }

            public static class ProducersBean implements Serializable, Comparable<ProducersBean>, Parcelable {
                /**
                 * ownerpublickey : 035ecacbdacb997ef284bf9d4d62ff1a07cee034be455c2a2b5b2435651fd4716b
                 * nodepublickey : 035ecacbdacb997ef284bf9d4d62ff1a07cee034be455c2a2b5b2435651fd4716b
                 * nickname : r_adr_us00
                 * url : www.r_adr_us00.com
                 * location : 86
                 * active : true
                 * votes : 18071.07053744
                 * state : Active
                 * registerheight : 317077
                 * cancelheight : 0
                 * inactiveheight : 0
                 * illegalheight : 0
                 * index : 34
                 * voterate : 0.001094719222928
                 */
                @Override
                public boolean equals(Object other) {
                    return this.getOwnerpublickey().equals(((ProducersBean) other).getOwnerpublickey());
                }

                private String ownerpublickey;
                private String nodepublickey;
                private String nickname;
                private String url;
                private int location;
                private boolean active;
                private String votes;
                private String state;
                private int registerheight;
                private int cancelheight;
                private int inactiveheight;
                private int illegalheight;
                private int index;
                private String voterate;

                public String getOwnerpublickey() {
                    return ownerpublickey;
                }

                public void setOwnerpublickey(String ownerpublickey) {
                    this.ownerpublickey = ownerpublickey;
                }

                public String getNodepublickey() {
                    return nodepublickey;
                }

                public void setNodepublickey(String nodepublickey) {
                    this.nodepublickey = nodepublickey;
                }

                public String getNickname() {
                    return nickname;
                }

                public void setNickname(String nickname) {
                    this.nickname = nickname;
                }

                public String getUrl() {
                    return url;
                }

                public void setUrl(String url) {
                    this.url = url;
                }

                public int getLocation() {
                    return location;
                }

                public void setLocation(int location) {
                    this.location = location;
                }

                public boolean isActive() {
                    return active;
                }

                public void setActive(boolean active) {
                    this.active = active;
                }

                public String getVotes() {
                    return votes;
                }

                public void setVotes(String votes) {
                    this.votes = votes;
                }

                public String getState() {
                    return state;
                }

                public void setState(String state) {
                    this.state = state;
                }

                public int getRegisterheight() {
                    return registerheight;
                }

                public void setRegisterheight(int registerheight) {
                    this.registerheight = registerheight;
                }

                public int getCancelheight() {
                    return cancelheight;
                }

                public void setCancelheight(int cancelheight) {
                    this.cancelheight = cancelheight;
                }

                public int getInactiveheight() {
                    return inactiveheight;
                }

                public void setInactiveheight(int inactiveheight) {
                    this.inactiveheight = inactiveheight;
                }

                public int getIllegalheight() {
                    return illegalheight;
                }

                public void setIllegalheight(int illegalheight) {
                    this.illegalheight = illegalheight;
                }

                public int getIndex() {
                    return index;
                }

                public void setIndex(int index) {
                    this.index = index;
                }

                public String getVoterate() {
                    return voterate;
                }

                public void setVoterate(String voterate) {
                    this.voterate = voterate;
                }

                @Override
                public String toString() {
                    return "ProducersBean{" +
                            "ownerpublickey='" + ownerpublickey + '\'' +
                            ", nodepublickey='" + nodepublickey + '\'' +
                            ", nickname='" + nickname + '\'' +
                            ", url='" + url + '\'' +
                            ", location=" + location +
                            ", active=" + active +
                            ", votes='" + votes + '\'' +
                            ", state='" + state + '\'' +
                            ", registerheight=" + registerheight +
                            ", cancelheight=" + cancelheight +
                            ", inactiveheight=" + inactiveheight +
                            ", illegalheight=" + illegalheight +
                            ", index=" + index +
                            ", voterate='" + voterate + '\'' +
                            '}';
                }

                @Override
                public int compareTo(@NonNull ProducersBean o) {
                    return Integer.compare(this.getIndex(),o.getIndex());

                }

                @Override
                public int describeContents() {
                    return 0;
                }

                @Override
                public void writeToParcel(Parcel dest, int flags) {
                    dest.writeString(this.ownerpublickey);
                    dest.writeString(this.nodepublickey);
                    dest.writeString(this.nickname);
                    dest.writeString(this.url);
                    dest.writeInt(this.location);
                    dest.writeByte(this.active ? (byte) 1 : (byte) 0);
                    dest.writeString(this.votes);
                    dest.writeString(this.state);
                    dest.writeInt(this.registerheight);
                    dest.writeInt(this.cancelheight);
                    dest.writeInt(this.inactiveheight);
                    dest.writeInt(this.illegalheight);
                    dest.writeInt(this.index);
                    dest.writeString(this.voterate);
                }

                public ProducersBean() {
                }

                protected ProducersBean(Parcel in) {
                    this.ownerpublickey = in.readString();
                    this.nodepublickey = in.readString();
                    this.nickname = in.readString();
                    this.url = in.readString();
                    this.location = in.readInt();
                    this.active = in.readByte() != 0;
                    this.votes = in.readString();
                    this.state = in.readString();
                    this.registerheight = in.readInt();
                    this.cancelheight = in.readInt();
                    this.inactiveheight = in.readInt();
                    this.illegalheight = in.readInt();
                    this.index = in.readInt();
                    this.voterate = in.readString();
                }

                public static final Parcelable.Creator<ProducersBean> CREATOR = new Parcelable.Creator<ProducersBean>() {
                    @Override
                    public ProducersBean createFromParcel(Parcel source) {
                        return new ProducersBean(source);
                    }

                    @Override
                    public ProducersBean[] newArray(int size) {
                        return new ProducersBean[size];
                    }
                };
            }
        }
    }
}
