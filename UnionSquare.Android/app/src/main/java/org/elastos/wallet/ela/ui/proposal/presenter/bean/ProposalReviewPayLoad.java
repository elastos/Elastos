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
