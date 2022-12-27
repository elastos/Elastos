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

package org.elastos.wallet.ela.ui.Assets.bean;

import java.io.Serializable;
import java.util.List;

public class TransferRecordDetailEntity implements Serializable {


    /**
     * MaxCount : 3
     * Transactions : [{"Amount":"0","Attribute":[{"Data":"61747472696275746573","Usage":0}],"ConfirmStatus":"6+","Direction":"Moved","Fee":10000,"Height":186,"Inputs":{"Ed8ZSxSB98roeyuRZwwekrnRqcgnfiUDeQ":"99999990000"},"Memo":"","OutputPayload":[{"Amount":"10000000000","Version":0,"VoteContent":[{"Candidates":["03d916c2072fd8fb57224e9747e0f1e36a2c117689cedf39e0132f3cb4f8ee673d"],"Type":0}]}],"Outputs":{"Ed8ZSxSB98roeyuRZwwekrnRqcgnfiUDeQ":"99999980000"},"Payload":null,"Status":"Confirmed","Timestamp":1560855293,"TxHash":"965a30286ddd7df1803963c32195ea1348a5dcebf4017e4d9d47cc19f2e6d56a","Type":2}]
     */

    private int MaxCount;
    private List<TransactionsBean> Transactions;

    public int getMaxCount() {
        return MaxCount;
    }

    public void setMaxCount(int MaxCount) {
        this.MaxCount = MaxCount;
    }

    public List<TransactionsBean> getTransactions() {
        return Transactions;
    }

    public void setTransactions(List<TransactionsBean> Transactions) {
        this.Transactions = Transactions;
    }

    public static class TransactionsBean {
        /**
         * Amount : 0
         * Attribute : [{"Data":"61747472696275746573","Usage":0}]
         * ConfirmStatus : 6+
         * Direction : Moved
         * Fee : 10000
         * Height : 186
         * Inputs : {"Ed8ZSxSB98roeyuRZwwekrnRqcgnfiUDeQ":"99999990000"}
         * Memo :
         * OutputPayload : [{"Amount":"10000000000","Version":0,"VoteContent":[{"Candidates":["03d916c2072fd8fb57224e9747e0f1e36a2c117689cedf39e0132f3cb4f8ee673d"],"Type":0}]}]
         * Outputs : {"Ed8ZSxSB98roeyuRZwwekrnRqcgnfiUDeQ":"99999980000"}
         * Payload : null
         * Status : Confirmed
         * Timestamp : 1560855293
         * TxHash : 965a30286ddd7df1803963c32195ea1348a5dcebf4017e4d9d47cc19f2e6d56a
         * Type : 2
         */

        private String Amount;
        private String ConfirmStatus;
        private String Direction;
        private int Fee;
        private int Height;
        private String Inputs;
        private String Memo;
        private String Outputs;
        private List<PayloadBean> Payload;
        private String Status;
        private long Timestamp;
        private String TxHash;
        private int Type;
        private List<AttributeBean> Attribute;
        private List<OutputPayloadBean> OutputPayload;

        public List<PayloadBean> getPayload() {
            return Payload;
        }

        public void setPayload(List<PayloadBean> payload) {
            Payload = payload;
        }

        public String getAmount() {
            return Amount;
        }

        public void setAmount(String Amount) {
            this.Amount = Amount;
        }

        public String getConfirmStatus() {
            return ConfirmStatus;
        }

        public void setConfirmStatus(String ConfirmStatus) {
            this.ConfirmStatus = ConfirmStatus;
        }

        public String getDirection() {
            return Direction;
        }

        public void setDirection(String Direction) {
            this.Direction = Direction;
        }

        public int getFee() {
            return Fee;
        }

        public void setFee(int Fee) {
            this.Fee = Fee;
        }

        public int getHeight() {
            return Height;
        }

        public void setHeight(int Height) {
            this.Height = Height;
        }

        public String getInputs() {
            return Inputs;
        }

        public void setInputs(String Inputs) {
            this.Inputs = Inputs;
        }

        public String getMemo() {
            return Memo;
        }

        public void setMemo(String Memo) {
            this.Memo = Memo;
        }

        public String getOutputs() {
            return Outputs;
        }

        public void setOutputs(String Outputs) {
            this.Outputs = Outputs;
        }


        public String getStatus() {
            return Status;
        }

        public void setStatus(String Status) {
            this.Status = Status;
        }

        public long getTimestamp() {
            return Timestamp;
        }

        public void setTimestamp(long Timestamp) {
            this.Timestamp = Timestamp;
        }

        public String getTxHash() {
            return TxHash;
        }

        public void setTxHash(String TxHash) {
            this.TxHash = TxHash;
        }

        public int getType() {
            return Type;
        }

        public void setType(int Type) {
            this.Type = Type;
        }

        public List<AttributeBean> getAttribute() {
            return Attribute;
        }

        public void setAttribute(List<AttributeBean> Attribute) {
            this.Attribute = Attribute;
        }

        public List<OutputPayloadBean> getOutputPayload() {
            return OutputPayload;
        }

        public void setOutputPayload(List<OutputPayloadBean> OutputPayload) {
            this.OutputPayload = OutputPayload;
        }


        public static class PayloadBean {

            /**
             * CrossChainAddress : EKZ4YMJgYY4GajBQBhWck5C2nqdLPuJ12e
             * CrossChainAmount : 339980000
             * OutputIndex : 0
             */

            private String CrossChainAddress;
            private String CrossChainAmount;
            private int OutputIndex;

            public String getCrossChainAddress() {
                return CrossChainAddress;
            }

            public void setCrossChainAddress(String CrossChainAddress) {
                this.CrossChainAddress = CrossChainAddress;
            }

            public String getCrossChainAmount() {
                return CrossChainAmount;
            }

            public void setCrossChainAmount(String CrossChainAmount) {
                this.CrossChainAmount = CrossChainAmount;
            }

            public int getOutputIndex() {
                return OutputIndex;
            }

            public void setOutputIndex(int OutputIndex) {
                this.OutputIndex = OutputIndex;
            }
        }

        public static class AttributeBean {
            /**
             * Data : 61747472696275746573
             * Usage : 0
             */

            private String Data;
            private int Usage;

            public String getData() {
                return Data;
            }

            public void setData(String Data) {
                this.Data = Data;
            }

            public int getUsage() {
                return Usage;
            }

            public void setUsage(int Usage) {
                this.Usage = Usage;
            }
        }

        public static class OutputPayloadBean {
            /**
             * Amount : 10000000000
             * Version : 0
             * VoteContent : [{"Candidates":["03d916c2072fd8fb57224e9747e0f1e36a2c117689cedf39e0132f3cb4f8ee673d"],"Type":0}]
             */

            private String Amount;
            private int Version;
            private List<VoteContentBean> VoteContent;

            public String getAmount() {
                return Amount;
            }

            public void setAmount(String Amount) {
                this.Amount = Amount;
            }

            public int getVersion() {
                return Version;
            }

            public void setVersion(int Version) {
                this.Version = Version;
            }

            public List<VoteContentBean> getVoteContent() {
                return VoteContent;
            }

            public void setVoteContent(List<VoteContentBean> VoteContent) {
                this.VoteContent = VoteContent;
            }

            public static class VoteContentBean {
                /**
                 * Candidates : ["03d916c2072fd8fb57224e9747e0f1e36a2c117689cedf39e0132f3cb4f8ee673d"]
                 * Type : 0
                 */

                private int Type;
                private List<String> Candidates;

                public int getType() {
                    return Type;
                }

                public void setType(int Type) {
                    this.Type = Type;
                }

                public List<String> getCandidates() {
                    return Candidates;
                }

                public void setCandidates(List<String> Candidates) {
                    this.Candidates = Candidates;
                }
            }
        }
    }
}
