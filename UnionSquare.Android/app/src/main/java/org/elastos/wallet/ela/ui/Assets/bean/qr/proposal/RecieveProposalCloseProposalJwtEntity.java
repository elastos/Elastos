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

/**
 * 兼顾CloseProposal提建议和发提案
 */
public class RecieveProposalCloseProposalJwtEntity extends RecieveProposalAllJwtEntity {


    /**
     * iat : 1566352213
     * exp : 1580607089
     * sid : 5ec0fc6959835e0078762685
     * data : {"userdid":"did:elastos:iWWPzYbCny9Pbjdb7nCdvSdr1M1mcgvYUv","proposaltype":"closeproposal","categorydata":"","ownerpublickey":"023559273eec17bbfcedd041d2044163123a9bba34530540d864a6f3f484f7054a","drafthash":"6739263b511de00b49e08855254d46dbade53ca10c3e10babfb79b8196032464","targetproposalhash":"fdab8dae9e4f1a3dfe7127b80544bec8a4de66557fad5f8fb5f4b8bdbe555e81","signature":"0d77e801761f45628270d6f7a86b02def1f35b1c8964a4094b7947730c86b63d0ee1e8e448192d6a0ac74e1c592ee27c3b6d9b7010a5830dbff9cb506d03d148","did":"did:elastos:inDUQR73UQLFfgZocbC3PH4SFiRggffcNw"}
     */


    private String sid;
    private DataBean data;


    public String getSid() {
        return sid;
    }

    public void setSid(String sid) {
        this.sid = sid;
    }

    public DataBean getData() {
        return data;
    }

    public void setData(DataBean data) {
        this.data = data;
    }

    public static class DataBean extends RecieveProposalAllJwtEntity.DataBean{
        /**
         * userdid : did:elastos:iWWPzYbCny9Pbjdb7nCdvSdr1M1mcgvYUv
         * proposaltype : closeproposal
         * categorydata :
         * ownerpublickey : 023559273eec17bbfcedd041d2044163123a9bba34530540d864a6f3f484f7054a
         * drafthash : 6739263b511de00b49e08855254d46dbade53ca10c3e10babfb79b8196032464
         * targetproposalhash : fdab8dae9e4f1a3dfe7127b80544bec8a4de66557fad5f8fb5f4b8bdbe555e81
         * signature : 0d77e801761f45628270d6f7a86b02def1f35b1c8964a4094b7947730c86b63d0ee1e8e448192d6a0ac74e1c592ee27c3b6d9b7010a5830dbff9cb506d03d148
         * did : did:elastos:inDUQR73UQLFfgZocbC3PH4SFiRggffcNw
         */

        private String userdid;
        private String proposaltype;
        private String categorydata;
        private String ownerpublickey;
        private String drafthash;
        private String targetproposalhash;
        private String signature;
        private String did;

        public String getUserdid() {
            return userdid;
        }

        public void setUserdid(String userdid) {
            this.userdid = userdid;
        }

        public String getProposaltype() {
            return proposaltype;
        }

        public void setProposaltype(String proposaltype) {
            this.proposaltype = proposaltype;
        }

        public String getCategorydata() {
            return categorydata;
        }

        public void setCategorydata(String categorydata) {
            this.categorydata = categorydata;
        }

        public String getOwnerpublickey() {
            return ownerpublickey;
        }

        public void setOwnerpublickey(String ownerpublickey) {
            this.ownerpublickey = ownerpublickey;
        }

        public String getDrafthash() {
            return drafthash;
        }

        public void setDrafthash(String drafthash) {
            this.drafthash = drafthash;
        }

        public String getTargetproposalhash() {
            return targetproposalhash;
        }

        public void setTargetproposalhash(String targetproposalhash) {
            this.targetproposalhash = targetproposalhash;
        }

        public String getSignature() {
            return signature;
        }

        public void setSignature(String signature) {
            this.signature = signature;
        }

        public String getDid() {
            return did;
        }

        public void setDid(String did) {
            this.did = did;
        }
    }
}
