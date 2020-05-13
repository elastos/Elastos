package org.elastos.wallet.ela.ui.proposal.presenter.bean;

/**
 * 专为接口参数准备的entity
 */
public class ProposalCRCouncialMenberDigestPayLoad extends ProposalOwnerDigestPayLoad{

    /**
     * Signature : ff0ff9f45478f8f9fcd50b15534c9a60810670c3fb400d831cd253370c42a0af79f7f4015ebfb4a3791f5e45aa1c952d40408239dead3d23a51314b339981b76
     * CRCouncilMemberDID : icwTktC5M6fzySQ5yU7bKAZ6ipP623apFY
     */

    private String Signature;
    private String CRCouncilMemberDID;

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
