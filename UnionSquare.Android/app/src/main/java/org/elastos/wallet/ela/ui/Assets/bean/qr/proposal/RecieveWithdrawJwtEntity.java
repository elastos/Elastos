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

import com.google.gson.annotations.SerializedName;

import java.util.ArrayList;
import java.util.List;

/**
 * 兼顾提建议和提案
 */
public class RecieveWithdrawJwtEntity extends RecieveProposalFatherJwtEntity {


    /**
     * iat : 1566352213
     * exp : 1580607089
     * data : {"proposalhash":"9b6c85d10b1208d896eb7efc391d1a562ba91452a78f37acfbb1ac1ed63f1a83","amount":"10010000000","recipient":"EdB7W1rRh5KgUha9Wa676ZRmr18voCDS6k","ownerpublickey":"023559273eec17bbfcedd041d2044163123a9bba34530540d864a6f3f484f7054a","utxos":[{"txid":"e26a14c6ddb82f7e860748ec77e085bc2a82ba6d0fa5673fece0d78831f7dfcd","vout":1,"amount":"221067"},{"txid":"bd7c10d083a5e354cbcc2140c524b008458dc5e0cf2aa344be49e3ac9a74daa2","vout":0,"amount":"874111"}]}
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
         * amount : 10010000000
         * recipient : EdB7W1rRh5KgUha9Wa676ZRmr18voCDS6k
         * ownerpublickey : 023559273eec17bbfcedd041d2044163123a9bba34530540d864a6f3f484f7054a
         * utxos : [{"txid":"e26a14c6ddb82f7e860748ec77e085bc2a82ba6d0fa5673fece0d78831f7dfcd","vout":1,"amount":"221067"},{"txid":"bd7c10d083a5e354cbcc2140c524b008458dc5e0cf2aa344be49e3ac9a74daa2","vout":0,"amount":"874111"}]
         */

        private String proposalhash;
        private String amount;
        private String recipient;
        private String ownerpublickey;
        private List<UtxosBean> utxos;

        public String getProposalhash() {
            return proposalhash;
        }

        public void setProposalhash(String proposalhash) {
            this.proposalhash = proposalhash;
        }

        public String getAmount() {
            return amount;
        }

        public void setAmount(String amount) {
            this.amount = amount;
        }

        public String getRecipient() {
            return recipient;
        }

        public void setRecipient(String recipient) {
            this.recipient = recipient;
        }

        public String getOwnerpublickey() {
            return ownerpublickey;
        }

        public void setOwnerpublickey(String ownerpublickey) {
            this.ownerpublickey = ownerpublickey;
        }

        public List<UtxosBean> getUtxos() {
            return utxos;
        }

        public void setUtxos(List<UtxosBean> utxos) {
            this.utxos = utxos;
        }

        public static class UtxosBean implements Parcelable {
            /**
             * txid : e26a14c6ddb82f7e860748ec77e085bc2a82ba6d0fa5673fece0d78831f7dfcd
             * vout : 1
             * amount : 221067
             */

            private String txid;
            private int vout;
            private String amount;

            public String getTxid() {
                return txid;
            }

            public void setTxid(String txid) {
                this.txid = txid;
            }

            public int getVout() {
                return vout;
            }

            public void setVout(int vout) {
                this.vout = vout;
            }

            public String getAmount() {
                return amount;
            }

            public void setAmount(String amount) {
                this.amount = amount;
            }

            @Override
            public int describeContents() {
                return 0;
            }

            @Override
            public void writeToParcel(Parcel dest, int flags) {
                dest.writeString(this.txid);
                dest.writeInt(this.vout);
                dest.writeString(this.amount);
            }

            public UtxosBean() {
            }

            protected UtxosBean(Parcel in) {
                this.txid = in.readString();
                this.vout = in.readInt();
                this.amount = in.readString();
            }

            public static final Creator<UtxosBean> CREATOR = new Creator<UtxosBean>() {
                @Override
                public UtxosBean createFromParcel(Parcel source) {
                    return new UtxosBean(source);
                }

                @Override
                public UtxosBean[] newArray(int size) {
                    return new UtxosBean[size];
                }
            };
        }

        @Override
        public int describeContents() {
            return 0;
        }

        @Override
        public void writeToParcel(Parcel dest, int flags) {
            dest.writeString(this.proposalhash);
            dest.writeString(this.amount);
            dest.writeString(this.recipient);
            dest.writeString(this.ownerpublickey);
            dest.writeList(this.utxos);
        }

        public DataBean() {
        }

        protected DataBean(Parcel in) {
            this.proposalhash = in.readString();
            this.amount = in.readString();
            this.recipient = in.readString();
            this.ownerpublickey = in.readString();
            this.utxos = new ArrayList<UtxosBean>();
            in.readList(this.utxos, UtxosBean.class.getClassLoader());
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
