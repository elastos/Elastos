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

package org.elastos.wallet.ela.ui.proposal.bean;

import com.google.gson.annotations.SerializedName;

import org.elastos.wallet.ela.rxjavahelp.BaseEntity;

import java.util.List;

public class SuggestBean extends BaseEntity {


    /**
     * code : 1
     * data : {"id":1,"title":"title","did":"did","type":"changeproposalowner","targetProposalNum":"4","targetProposalTitle":"targetProposalTitle","targetProposalHash":"026bbb5b2ba09e5199dd4a6673062101be3d200d9","draftHash":"026bbb5b2ba09e5199dd4a6673062101be3d200d9","newOwnerDID":"imYEsSUYiikGTAuQXCQGPfH4YoTjuu6Gck","newAddress":"EHYXFf27EoMQ6HfZUuSKiLEsWJV9BMGFWM","newSecretaryDID":"imYEsSUYiikGTAuQXCQGPfH4YoTjuu6Gck","closeProposalNum":"4","didName":"did name","abs":"this is sample abstract","address":"http://localhost:3001/suggestion/5ea53e2bb0461a06630c0227","createdAt":1589271912,"fund":[{"type":"ADVANCE","amount":"1"},{"type":"CONDITIONED","amount":"1"},{"type":"CONDITIONED","amount":"1"},{"type":"CONDITIONED","amount":"1"},{"type":"COMPLETION","amount":"1"}],"fundAmount":"123"}
     */


    private DataBean data;



    public DataBean getData() {
        return data;
    }

    public void setData(DataBean data) {
        this.data = data;
    }

    public static class DataBean {
        /**
         * id : 1
         * title : title
         * did : did
         * type : changeproposalowner
         * targetProposalNum : 4
         * targetProposalTitle : targetProposalTitle
         * targetProposalHash : 026bbb5b2ba09e5199dd4a6673062101be3d200d9
         * draftHash : 026bbb5b2ba09e5199dd4a6673062101be3d200d9
         * newOwnerDID : imYEsSUYiikGTAuQXCQGPfH4YoTjuu6Gck
         * newAddress : EHYXFf27EoMQ6HfZUuSKiLEsWJV9BMGFWM
         * newSecretaryDID : imYEsSUYiikGTAuQXCQGPfH4YoTjuu6Gck
         * closeProposalNum : 4
         * didName : did name
         * abs : this is sample abstract
         * address : http://localhost:3001/suggestion/5ea53e2bb0461a06630c0227
         * createdAt : 1589271912
         * fund : [{"type":"ADVANCE","amount":"1"},{"type":"CONDITIONED","amount":"1"},{"type":"CONDITIONED","amount":"1"},{"type":"CONDITIONED","amount":"1"},{"type":"COMPLETION","amount":"1"}]
         * fundAmount : 123
         */

        private int id;
        private String title;
        private String did;
        private String type;
        private String targetProposalNum;
        private String targetProposalTitle;
        private String targetProposalHash;
        private String draftHash;
        private String newOwnerDID;
        private String newAddress;
        private String newSecretaryDID;
        private String closeProposalNum;
        private String didName;
        private String abs;
        private String address;
        private int createdAt;
        private String fundAmount;
        private List<FundBean> fund;

        public int getId() {
            return id;
        }

        public void setId(int id) {
            this.id = id;
        }

        public String getTitle() {
            return title;
        }

        public void setTitle(String title) {
            this.title = title;
        }

        public String getDid() {
            return did;
        }

        public void setDid(String did) {
            this.did = did;
        }

        public String getType() {
            return type;
        }

        public void setType(String type) {
            this.type = type;
        }

        public String getTargetProposalNum() {
            return targetProposalNum;
        }

        public void setTargetProposalNum(String targetProposalNum) {
            this.targetProposalNum = targetProposalNum;
        }

        public String getTargetProposalTitle() {
            return targetProposalTitle;
        }

        public void setTargetProposalTitle(String targetProposalTitle) {
            this.targetProposalTitle = targetProposalTitle;
        }

        public String getTargetProposalHash() {
            return targetProposalHash;
        }

        public void setTargetProposalHash(String targetProposalHash) {
            this.targetProposalHash = targetProposalHash;
        }

        public String getDraftHash() {
            return draftHash;
        }

        public void setDraftHash(String draftHash) {
            this.draftHash = draftHash;
        }

        public String getNewOwnerDID() {
            return newOwnerDID;
        }

        public void setNewOwnerDID(String newOwnerDID) {
            this.newOwnerDID = newOwnerDID;
        }

        public String getNewAddress() {
            return newAddress;
        }

        public void setNewAddress(String newAddress) {
            this.newAddress = newAddress;
        }

        public String getNewSecretaryDID() {
            return newSecretaryDID;
        }

        public void setNewSecretaryDID(String newSecretaryDID) {
            this.newSecretaryDID = newSecretaryDID;
        }

        public String getCloseProposalNum() {
            return closeProposalNum;
        }

        public void setCloseProposalNum(String closeProposalNum) {
            this.closeProposalNum = closeProposalNum;
        }

        public String getDidName() {
            return didName;
        }

        public void setDidName(String didName) {
            this.didName = didName;
        }

        public String getAbs() {
            return abs;
        }

        public void setAbs(String abs) {
            this.abs = abs;
        }

        public String getAddress() {
            return address;
        }

        public void setAddress(String address) {
            this.address = address;
        }

        public int getCreatedAt() {
            return createdAt;
        }

        public void setCreatedAt(int createdAt) {
            this.createdAt = createdAt;
        }

        public String getFundAmount() {
            return fundAmount;
        }

        public void setFundAmount(String fundAmount) {
            this.fundAmount = fundAmount;
        }

        public List<FundBean> getFund() {
            return fund;
        }

        public void setFund(List<FundBean> fund) {
            this.fund = fund;
        }

        public static class FundBean {
            /**
             * type : ADVANCE
             * amount : 1
             */

            private String type;
            private String amount;

            public String getType() {
                return type;
            }

            public void setType(String type) {
                this.type = type;
            }

            public String getAmount() {
                return amount;
            }

            public void setAmount(String amount) {
                this.amount = amount;
            }
        }
    }
}
