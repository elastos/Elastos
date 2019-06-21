package org.elastos.wallet.ela.ui.Assets.bean;

import java.io.Serializable;
import java.util.List;

public class TransferRecordEntity implements Serializable {


    /**
     * MaxCount : 3
     * Transactions : [{"Amount":"0","ConfirmStatus":"6+","Direction":"Moved","Height":186,"Status":"Confirmed","Timestamp":1560855293,"TxHash":"965a30286ddd7df1803963c32195ea1348a5dcebf4017e4d9d47cc19f2e6d56a","Type":2},{"Amount":"500000000000","ConfirmStatus":"6+","Direction":"Deposit","Height":179,"Status":"Confirmed","Timestamp":1560855292,"TxHash":"baafdc4174d764ec4311d0dbf4a8518af875d8d2d4a77dd6c14e4c85a2df95da","Type":9},{"Amount":"600000000000","ConfirmStatus":"6+","Direction":"Received","Height":114,"Status":"Confirmed","Timestamp":1560855281,"TxHash":"5d36908dfb0de98de286327682fbc0ffce7e471acf2bc274371c31bf53b85f3d","Type":2}]
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
         * ConfirmStatus : 6+
         * Direction : Moved
         * Height : 186
         * Status : Confirmed
         * Timestamp : 1560855293
         * TxHash : 965a30286ddd7df1803963c32195ea1348a5dcebf4017e4d9d47cc19f2e6d56a
         * Type : 2
         */

        private String Amount;
        private String ConfirmStatus;
        private String Direction;
        private int Height;
        private String Status;
        private long Timestamp;
        private String TxHash;
        private int Type;

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
    }
}
