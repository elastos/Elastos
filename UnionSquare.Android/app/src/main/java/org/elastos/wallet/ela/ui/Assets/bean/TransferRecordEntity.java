package org.elastos.wallet.ela.ui.Assets.bean;

import java.io.Serializable;
import java.util.List;

public class TransferRecordEntity implements Serializable {


    /**
     * MaxCount : 1
     * Transactions : [{"Amount":1000000000,"ConfirmStatus":"6+","Direction":"Received","Fee":0,"Inputs":{},"Outputs":{"ET6v23Mo7z1KDKMmmUSPPT6jqcGPZkcQip":1000000000},"Remark":"","Status":"Confirmed","Timestamp":1547608219,"TxHash":"7759cc78df959cd7473f98db6ae0a8f53632ed0bad3aca5995b6f532d9bc7e44","Type":2}]
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
         * Amount : 1000000000
         * ConfirmStatus : 6+
         * Direction : Received
         * Fee : 0
         * Inputs : {}
         * Outputs : {"ET6v23Mo7z1KDKMmmUSPPT6jqcGPZkcQip":1000000000}
         * Remark :
         * Status : Confirmed
         * Timestamp : 1547608219
         * TxHash : 7759cc78df959cd7473f98db6ae0a8f53632ed0bad3aca5995b6f532d9bc7e44
         * Type : 2
         */

        private long Amount;
        private String ConfirmStatus;
        private String Direction;
        private long Fee;
        private String Inputs;
        private String Outputs;
        private String Remark;
        private String Status;
        private long Timestamp;
        private String TxHash;
        private int Type;

        public int getHeight() {
            return Height;
        }

        public void setHeight(int height) {
            Height = height;
        }

        private int Height;

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

        public long getFee() {
            return Fee;
        }

        public void setFee(long Fee) {
            this.Fee = Fee;
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

        public String getInputs() {
            return Inputs;
        }

        public void setInputs(String inputs) {
            Inputs = inputs;
        }

        public String getOutputs() {
            return Outputs;
        }

        public void setOutputs(String outputs) {
            Outputs = outputs;
        }
    }
}
