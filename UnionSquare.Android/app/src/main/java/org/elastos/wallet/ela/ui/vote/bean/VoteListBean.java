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
                "message='" + message + '\'' +
                ", data=" + data +
                ", exceptionMsg=" + exceptionMsg +
                '}';
    }

    /**
     * code : 0
     * message : 查询成功^_^
     * data : {"error":null,"id":null,"jsonrpc":"2.0","result":{"producers":[{"ownerpublickey":"0341315fe4e1f26ba09c5c56bf76e1e97aaee992f59407b33c4fc9d42e11634bdc","nodepublickey":"022e2fce0641869a1a8af60f735279a45d2e28dc1d4c54ef7f9872b777d718b624","nickname":"arbiter-223","url":"ela_test.org","location":112211,"active":true,"votes":"1136.02815617","netaddress":"127.0.0.1","index":0,"voterate":"0.312349415480802"},{"ownerpublickey":"03b273e27a6820b55fe5a6b7a445814f7c1db300e961661aaed3a06cbdfd3dca5d","nodepublickey":"0296e28b9bced49e175de2d2ae0e6a03724da9d00241213c988eeb65583a14f0c9","nickname":"arbiter-225","url":"ela_test.org","location":112211,"active":true,"votes":"1087.66022539","netaddress":"127.0.0.1","index":1,"voterate":"0.299050717886824"},{"ownerpublickey":"028b6e4d5c85ce549cefb8987f61f3e50ae385737eadbefc3f90f70d6d1d530472","nodepublickey":"0385ddab890e035ceb8db18d23addeaf2634e967342589c71ace3f1d42aafe26d9","nickname":"arbiter-222","url":"ela_test.org","location":112211,"active":true,"votes":"700.70423291","netaddress":"127.0.0.1","index":2,"voterate":"0.192657687563168"},{"ownerpublickey":"02fe6d1f9e1e03ef7e41bf0cb65eea4f7c6e8f5faa902455b723b997a7e6ded40b","nodepublickey":"028bb013ce8228a28823a27e93c87dd5d92df186a0c3b947e430ccf667f120310e","nickname":"arbiter-221","url":"ela_test.org","location":112211,"active":true,"votes":"644.05835174","netaddress":"127.0.0.1","index":3,"voterate":"0.177082978629461"},{"ownerpublickey":"02dfd375f7f612e3a23e1a86cfb0d4d7de4ca1482bc36294a2d06d861201cb9f03","nodepublickey":"03244fc761c06bb5e7d1590df24cb5f0dab634d85865b9c30d620e76f3218b77af","nickname":"arbiter-224","url":"ela_test.org","location":112211,"active":true,"votes":"66.59171697","netaddress":"127.0.0.1","index":4,"voterate":"0.018309303126401"},{"ownerpublickey":"039a1de7457157317cb1578cfafa72a13eb8332a198960bb1e5733f4bfd559b09e","nodepublickey":"039a1de7457157317cb1578cfafa72a13eb8332a198960bb1e5733f4bfd559b09e","nickname":"ssss","url":"https://www.baidu.com","location":1,"active":true,"votes":"2","netaddress":"ssssssd","index":5,"voterate":"0.000549897313345"}],"totalvotes":"3637.04268318","totalcounts":6,"totalvoterate":"0.000003576128237"}}
     * exceptionMsg : null
     */


    private String message;
    private DataBean data;
    private Object exceptionMsg;


    public String getMessage() {
        return message;
    }

    public void setMessage(String message) {
        this.message = message;
    }

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
         * result : {"producers":[{"ownerpublickey":"0341315fe4e1f26ba09c5c56bf76e1e97aaee992f59407b33c4fc9d42e11634bdc","nodepublickey":"022e2fce0641869a1a8af60f735279a45d2e28dc1d4c54ef7f9872b777d718b624","nickname":"arbiter-223","url":"ela_test.org","location":112211,"active":true,"votes":"1136.02815617","netaddress":"127.0.0.1","index":0,"voterate":"0.312349415480802"},{"ownerpublickey":"03b273e27a6820b55fe5a6b7a445814f7c1db300e961661aaed3a06cbdfd3dca5d","nodepublickey":"0296e28b9bced49e175de2d2ae0e6a03724da9d00241213c988eeb65583a14f0c9","nickname":"arbiter-225","url":"ela_test.org","location":112211,"active":true,"votes":"1087.66022539","netaddress":"127.0.0.1","index":1,"voterate":"0.299050717886824"},{"ownerpublickey":"028b6e4d5c85ce549cefb8987f61f3e50ae385737eadbefc3f90f70d6d1d530472","nodepublickey":"0385ddab890e035ceb8db18d23addeaf2634e967342589c71ace3f1d42aafe26d9","nickname":"arbiter-222","url":"ela_test.org","location":112211,"active":true,"votes":"700.70423291","netaddress":"127.0.0.1","index":2,"voterate":"0.192657687563168"},{"ownerpublickey":"02fe6d1f9e1e03ef7e41bf0cb65eea4f7c6e8f5faa902455b723b997a7e6ded40b","nodepublickey":"028bb013ce8228a28823a27e93c87dd5d92df186a0c3b947e430ccf667f120310e","nickname":"arbiter-221","url":"ela_test.org","location":112211,"active":true,"votes":"644.05835174","netaddress":"127.0.0.1","index":3,"voterate":"0.177082978629461"},{"ownerpublickey":"02dfd375f7f612e3a23e1a86cfb0d4d7de4ca1482bc36294a2d06d861201cb9f03","nodepublickey":"03244fc761c06bb5e7d1590df24cb5f0dab634d85865b9c30d620e76f3218b77af","nickname":"arbiter-224","url":"ela_test.org","location":112211,"active":true,"votes":"66.59171697","netaddress":"127.0.0.1","index":4,"voterate":"0.018309303126401"},{"ownerpublickey":"039a1de7457157317cb1578cfafa72a13eb8332a198960bb1e5733f4bfd559b09e","nodepublickey":"039a1de7457157317cb1578cfafa72a13eb8332a198960bb1e5733f4bfd559b09e","nickname":"ssss","url":"https://www.baidu.com","location":1,"active":true,"votes":"2","netaddress":"ssssssd","index":5,"voterate":"0.000549897313345"}],"totalvotes":"3637.04268318","totalcounts":6,"totalvoterate":"0.000003576128237"}
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
             * producers : [{"ownerpublickey":"0341315fe4e1f26ba09c5c56bf76e1e97aaee992f59407b33c4fc9d42e11634bdc","nodepublickey":"022e2fce0641869a1a8af60f735279a45d2e28dc1d4c54ef7f9872b777d718b624","nickname":"arbiter-223","url":"ela_test.org","location":112211,"active":true,"votes":"1136.02815617","netaddress":"127.0.0.1","index":0,"voterate":"0.312349415480802"},{"ownerpublickey":"03b273e27a6820b55fe5a6b7a445814f7c1db300e961661aaed3a06cbdfd3dca5d","nodepublickey":"0296e28b9bced49e175de2d2ae0e6a03724da9d00241213c988eeb65583a14f0c9","nickname":"arbiter-225","url":"ela_test.org","location":112211,"active":true,"votes":"1087.66022539","netaddress":"127.0.0.1","index":1,"voterate":"0.299050717886824"},{"ownerpublickey":"028b6e4d5c85ce549cefb8987f61f3e50ae385737eadbefc3f90f70d6d1d530472","nodepublickey":"0385ddab890e035ceb8db18d23addeaf2634e967342589c71ace3f1d42aafe26d9","nickname":"arbiter-222","url":"ela_test.org","location":112211,"active":true,"votes":"700.70423291","netaddress":"127.0.0.1","index":2,"voterate":"0.192657687563168"},{"ownerpublickey":"02fe6d1f9e1e03ef7e41bf0cb65eea4f7c6e8f5faa902455b723b997a7e6ded40b","nodepublickey":"028bb013ce8228a28823a27e93c87dd5d92df186a0c3b947e430ccf667f120310e","nickname":"arbiter-221","url":"ela_test.org","location":112211,"active":true,"votes":"644.05835174","netaddress":"127.0.0.1","index":3,"voterate":"0.177082978629461"},{"ownerpublickey":"02dfd375f7f612e3a23e1a86cfb0d4d7de4ca1482bc36294a2d06d861201cb9f03","nodepublickey":"03244fc761c06bb5e7d1590df24cb5f0dab634d85865b9c30d620e76f3218b77af","nickname":"arbiter-224","url":"ela_test.org","location":112211,"active":true,"votes":"66.59171697","netaddress":"127.0.0.1","index":4,"voterate":"0.018309303126401"},{"ownerpublickey":"039a1de7457157317cb1578cfafa72a13eb8332a198960bb1e5733f4bfd559b09e","nodepublickey":"039a1de7457157317cb1578cfafa72a13eb8332a198960bb1e5733f4bfd559b09e","nickname":"ssss","url":"https://www.baidu.com","location":1,"active":true,"votes":"2","netaddress":"ssssssd","index":5,"voterate":"0.000549897313345"}]
             * totalvotes : 3637.04268318
             * totalcounts : 6
             * totalvoterate : 0.000003576128237
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
                 * ownerpublickey : 0341315fe4e1f26ba09c5c56bf76e1e97aaee992f59407b33c4fc9d42e11634bdc
                 * nodepublickey : 022e2fce0641869a1a8af60f735279a45d2e28dc1d4c54ef7f9872b777d718b624
                 * nickname : arbiter-223
                 * url : ela_test.org
                 * location : 112211
                 * active : true
                 * votes : 1136.02815617
                 * netaddress : 127.0.0.1
                 * index : 0
                 * voterate : 0.312349415480802
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
                private String netaddress;
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

                public String getNetaddress() {
                    return netaddress;
                }

                public void setNetaddress(String netaddress) {
                    this.netaddress = netaddress;
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
                            ", netaddress='" + netaddress + '\'' +
                            ", index=" + index +
                            ", voterate='" + voterate + '\'' +
                            '}';
                }

                @Override
                public int compareTo(@NonNull ProducersBean o) {
                    return Double.compare(Double.parseDouble(o.getVoterate()), Double.parseDouble(this.getVoterate()));

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
                    dest.writeString(this.netaddress);
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
                    this.netaddress = in.readString();
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
