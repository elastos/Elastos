package org.elastos.wallet.ela.ui.Assets.bean;

import java.io.Serializable;
import java.util.List;

public class TransferRecordDetailEntity implements Serializable {


    /**
     * MaxCount : 20
     * Transactions : [{"Amount":0,"Attribute":[{"Data":"31343734323333323437","Usage":0}],"ConfirmStatus":"0","Direction":"Moved","Fee":10000,"Height":2147483647,"Inputs":{"EP3A2iJ4o7ywmUD7b12Q75ZDYvPmFaoJm2":1000000000,"EfK7oTYL3LevMwkwiJNKF8zRby8Cd3zDy3":9099810000},"OutputPayload":[{"Amount":10000000000,"Version":0,"VoteContent":[{"Candidates":["0216e8d9f61d7ae87624ac4e8ba54848abd830c62b6820ba8e83815507a1f998c4"],"Type":0}]}],"Outputs":{"EfK7oTYL3LevMwkwiJNKF8zRby8Cd3zDy3":10099800000},"Payload":null,"Remark":"","Status":"Pending","Timestamp":1551434018,"TxHash":"a2dd2723aada24c9f472946763711db3180282ff4b69f0c14039d5418d63a07e","Type":2}]
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
         * Attribute : [{"Data":"31343734323333323437","Usage":0}]
         * ConfirmStatus : 0
         * Direction : Moved
         * Fee : 10000
         * Height : 2147483647
         * Inputs : {"EP3A2iJ4o7ywmUD7b12Q75ZDYvPmFaoJm2":1000000000,"EfK7oTYL3LevMwkwiJNKF8zRby8Cd3zDy3":9099810000}
         * OutputPayload : [{"Amount":10000000000,"Version":0,"VoteContent":[{"Candidates":["0216e8d9f61d7ae87624ac4e8ba54848abd830c62b6820ba8e83815507a1f998c4"],"Type":0}]}]
         * Outputs : {"EfK7oTYL3LevMwkwiJNKF8zRby8Cd3zDy3":10099800000}
         * Payload : null
         * Remark :
         * Status : Pending
         * Timestamp : 1551434018
         * TxHash : a2dd2723aada24c9f472946763711db3180282ff4b69f0c14039d5418d63a07e
         * Type : 2
         */

        private long Amount;
        private String ConfirmStatus;
        private String Direction;
        private int Fee;
        private int Height;
        private String Inputs;
        private String Outputs;
        private String Payload;
        private String Remark;
        private String Status;
        private long Timestamp;
        private String TxHash;
        private int Type;
        private List<AttributeBean> Attribute;
        private List<OutputPayloadBean> OutputPayload;

        public long getAmount() {
            return Amount;
        }

        public void setAmount(long Amount) {
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

        public String getOutputs() {
            return Outputs;
        }

        public void setOutputs(String Outputs) {
            this.Outputs = Outputs;
        }

        public String getPayload() {
            return Payload;
        }

        public void setPayload(String Payload) {
            this.Payload = Payload;
        }

        public String getRemark() {
            return Remark;
        }

        public void setRemark(String Remark) {
            this.Remark = Remark;
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



        public static class AttributeBean {
            /**
             * Data : 31343734323333323437
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
             * VoteContent : [{"Candidates":["0216e8d9f61d7ae87624ac4e8ba54848abd830c62b6820ba8e83815507a1f998c4"],"Type":0}]
             */

            private double Amount;
            private int Version;
            private List<VoteContentBean> VoteContent;

            public double getAmount() {
                return Amount;
            }

            public void setAmount(double Amount) {
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
                 * Candidates : ["0216e8d9f61d7ae87624ac4e8ba54848abd830c62b6820ba8e83815507a1f998c4"]
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
