package org.elastos.wallet.ela.ui.proposal.presenter.bean;

import java.util.List;

/**
 * 专为接口参数准备的entity
 */
public class ProposalOwnerDigestPayLoad {
    /**
     * Type : 0
     * CategoryData : testdata
     * OwnerPublicKey : 031f7a5a6bf3b2450cd9da4048d00a8ef1cb4912b5057535f65f3cc0e0c36f13b4
     * DraftHash : a3d0eaa466df74983b5d7c543de6904f4c9418ead5ffd6d25814234a96db37b0
     * Budgets : [{"Type":0,"Stage":0,"Amount":"300"},{"Type":1,"Stage":1,"Amount":"33"},{"Type":2,"Stage":2,"Amount":"344"}]
     * Recipient : EPbdmxUVBzfNrVdqJzZEySyWGYeuKAeKqv
     */

    private int Type;
    private String CategoryData;
    private String OwnerPublicKey;
    private String DraftHash;
    private String Recipient;
    private List<BudgetsBean> Budgets;

    public int getType() {
        return Type;
    }

    public void setType(int Type) {
        this.Type = Type;
    }

    public String getCategoryData() {
        return CategoryData;
    }

    public void setCategoryData(String CategoryData) {
        this.CategoryData = CategoryData;
    }

    public String getOwnerPublicKey() {
        return OwnerPublicKey;
    }

    public void setOwnerPublicKey(String OwnerPublicKey) {
        this.OwnerPublicKey = OwnerPublicKey;
    }

    public String getDraftHash() {
        return DraftHash;
    }

    public void setDraftHash(String DraftHash) {
        this.DraftHash = DraftHash;
    }

    public String getRecipient() {
        return Recipient;
    }

    public void setRecipient(String Recipient) {
        this.Recipient = Recipient;
    }

    public List<BudgetsBean> getBudgets() {
        return Budgets;
    }

    public void setBudgets(List<BudgetsBean> Budgets) {
        this.Budgets = Budgets;
    }

    public static class BudgetsBean {
        /**
         * Type : 0
         * Stage : 0
         * Amount : 300
         */

        private int Type;
        private int Stage;
        private String Amount;

        public int getType() {
            return Type;
        }

        public void setType(int Type) {
            this.Type = Type;
        }

        public int getStage() {
            return Stage;
        }

        public void setStage(int Stage) {
            this.Stage = Stage;
        }

        public String getAmount() {
            return Amount;
        }

        public void setAmount(String Amount) {
            this.Amount = Amount;
        }
    }
}
