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

package org.elastos.wallet.ela.ui.proposal.presenter.bean;

import java.util.List;

/**
 * 专为接口参数准备的entity
 */
public class ProposalNormalDigestPayLoad {

    /**
     * Signature : ff0ff9f45478f8f9fcd50b15534c9a60810670c3fb400d831cd253370c42a0af79f7f4015ebfb4a3791f5e45aa1c952d40408239dead3d23a51314b339981b76
     * CRCouncilMemberDID : icwTktC5M6fzySQ5yU7bKAZ6ipP623apFY
     */
    private int Type;
    private String CategoryData;
    private String OwnerPublicKey;
    private String DraftHash;
    private String Recipient;
    private List<BudgetsBean> Budgets;
    private String Signature;
    private String CRCouncilMemberDID;

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


    public String getCRCouncilMemberSignature() {
        return CRCouncilMemberSignature;
    }

    public void setCRCouncilMemberSignature(String CRCouncilMemberSignature) {
        this.CRCouncilMemberSignature = CRCouncilMemberSignature;
    }

    private String CRCouncilMemberSignature;

    public String getSignature() {
        return Signature;
    }

    public void setSignature(String Signature) {
        this.Signature = Signature;
    }

    public String getCRCouncilMemberDID() {
        return CRCouncilMemberDID;
    }

    public void setCRCouncilMemberDID(String CRCouncilMemberDID) {
        this.CRCouncilMemberDID = CRCouncilMemberDID;
    }
}
