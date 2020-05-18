package org.elastos.wallet.ela.ui.Assets.bean.qr.proposal;

/**
 * 兼顾提建议和提案
 */
public class RecieveReviewJwtEntity extends RecieveProposalFatherJwtEntity {


    /**
     * iat : 1566352213
     * exp : 1580607089
     * data : {"proposalhash":"9b6c85d10b1208d896eb7efc391d1a562ba91452a78f37acfbb1ac1ed63f1a83","voteresult":"approve","opinionhash":"fc44baa69cfcca1c6f5bb9451555034a0244e50b511dadc3454f77ab24da1a27","DID":"did:elastos:inDUQR73UQLFfgZocbC3PH4SFiRggffcNw"}
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
         * proposalhash : 9b6c85d10b1208d896eb7efc391d1a562ba91452a78f37acfbb1ac1ed63f1a83
         * voteresult : approve
         * opinionhash : fc44baa69cfcca1c6f5bb9451555034a0244e50b511dadc3454f77ab24da1a27
         * DID : did:elastos:inDUQR73UQLFfgZocbC3PH4SFiRggffcNw
         */

        private String proposalhash;
        private String voteresult;
        private String opinionhash;
        private String DID;

        public String getProposalhash() {
            return proposalhash;
        }

        public void setProposalhash(String proposalhash) {
            this.proposalhash = proposalhash;
        }

        public String getVoteresult() {
            return voteresult;
        }

        public void setVoteresult(String voteresult) {
            this.voteresult = voteresult;
        }

        public String getOpinionhash() {
            return opinionhash;
        }

        public void setOpinionhash(String opinionhash) {
            this.opinionhash = opinionhash;
        }

        public String getDID() {
            return DID;
        }

        public void setDID(String DID) {
            this.DID = DID;
        }
    }
}
