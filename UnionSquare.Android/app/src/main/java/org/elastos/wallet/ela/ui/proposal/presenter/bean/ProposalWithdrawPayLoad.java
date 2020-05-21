package org.elastos.wallet.ela.ui.proposal.presenter.bean;

/**
 * 专为接口参数准备的entity
 */
public class ProposalWithdrawPayLoad {


    /**
     * ProposalHash : 7c5d2e7cfd7d4011414b5ddb3ab43e2aca247e342d064d1091644606748d7513
     * OwnerPublicKey : 02c632e27b19260d80d58a857d2acd9eb603f698445cc07ba94d52296468706331
     * Signature : 9a24a084a6f599db9906594800b6cb077fa7995732c575d4d125c935446c93bbe594ee59e361f4d5c2142856c89c5d70c8811048bfb2f8620fbc18a06cb58109
     */

    private String ProposalHash;
    private String OwnerPublicKey;
    private String Signature;

    public String getProposalHash() {
        return ProposalHash;
    }

    public void setProposalHash(String ProposalHash) {
        this.ProposalHash = ProposalHash;
    }

    public String getOwnerPublicKey() {
        return OwnerPublicKey;
    }

    public void setOwnerPublicKey(String OwnerPublicKey) {
        this.OwnerPublicKey = OwnerPublicKey;
    }

    public String getSignature() {
        return Signature;
    }

    public void setSignature(String Signature) {
        this.Signature = Signature;
    }
}
