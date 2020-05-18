package org.elastos.wallet.ela.ui.proposal.presenter.bean;

/**
 * 专为接口参数准备的entity
 */
public class ProposalReviewPayLoad {


    /**
     * ProposalHash : a3d0eaa466df74983b5d7c543de6904f4c9418ead5ffd6d25814234a96db37b0
     * VoteResult : 1
     * OpinionHash : a3d0eaa466df74983b5d7c543de6904f4c9418ead5ffd6d25814234a96db37b0
     * DID : icwTktC5M6fzySQ5yU7bKAZ6ipP623apFY
     * Signature : ff0ff9f45478f8f9fcd50b15534c9a60810670c3fb400d831cd253370c42a0af79f7f4015ebfb4a3791f5e45aa1c952d40408239dead3d23a51314b339981b76
     */

    private String ProposalHash;
    private int VoteResult;
    private String OpinionHash;
    private String DID;
    private String Signature;

    public String getProposalHash() {
        return ProposalHash;
    }

    public void setProposalHash(String ProposalHash) {
        this.ProposalHash = ProposalHash;
    }

    public int getVoteResult() {
        return VoteResult;
    }

    public void setVoteResult(int VoteResult) {
        this.VoteResult = VoteResult;
    }

    public String getOpinionHash() {
        return OpinionHash;
    }

    public void setOpinionHash(String OpinionHash) {
        this.OpinionHash = OpinionHash;
    }

    public String getDID() {
        return DID;
    }

    public void setDID(String DID) {
        this.DID = DID;
    }

    public String getSignature() {
        return Signature;
    }

    public void setSignature(String Signature) {
        this.Signature = Signature;
    }
}
