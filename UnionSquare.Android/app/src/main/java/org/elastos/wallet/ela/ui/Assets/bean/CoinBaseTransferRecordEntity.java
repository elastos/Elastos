package org.elastos.wallet.ela.ui.Assets.bean;

import java.io.Serializable;
import java.util.List;

public class CoinBaseTransferRecordEntity implements Serializable {
    /**
     * MaxCount : 133
     * Transactions : [{"Address":"ELZostBoNmEDz3tqRN56XHzUBLgNNaeUbi","Amount":"1142694064","ConfirmStatus":"9","Direction":"Received","Height":1904,"Spent":false,"Status":"Pending","Timestamp":1561011089,"TxHash":"859f3d05600925c009f045b744e7e62c2e507e53fc55f09e3b4bc2c715195096","Type":0}]
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
         * Address : ELZostBoNmEDz3tqRN56XHzUBLgNNaeUbi
         * Amount : 1142694064
         * ConfirmStatus : 9
         * Direction : Received
         * Height : 1904
         * Spent : false
         * Status : Pending
         * Timestamp : 1561011089
         * TxHash : 859f3d05600925c009f045b744e7e62c2e507e53fc55f09e3b4bc2c715195096
         * Type : 0
         */

        private String Address;
        private String Amount;
        private String ConfirmStatus;
        private String Direction;
        private int Height;
        private boolean Spent;
        private String Status;
        private int Timestamp;
        private String TxHash;
        private int Type;

        public String getAddress() {
            return Address;
        }

        public void setAddress(String Address) {
            this.Address = Address;
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

        public int getHeight() {
            return Height;
        }

        public void setHeight(int Height) {
            this.Height = Height;
        }

        public boolean isSpent() {
            return Spent;
        }

        public void setSpent(boolean Spent) {
            this.Spent = Spent;
        }

        public String getStatus() {
            return Status;
        }

        public void setStatus(String Status) {
            this.Status = Status;
        }

        public int getTimestamp() {
            return Timestamp;
        }

        public void setTimestamp(int Timestamp) {
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
    }
}
