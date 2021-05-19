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


public class ProposalSecretarygeneralDigestPayLoad {


    /**
     * CategoryData : testdata
     * OwnerPublicKey : 031f7a5a6bf3b2450cd9da4048d00a8ef1cb4912b5057535f65f3cc0e0c36f13b4
     * DraftHash : a3d0eaa466df74983b5d7c543de6904f4c9418ead5ffd6d25814234a96db37b0
     * SecretaryGeneralPublicKey : ...
     * SecretaryGeneralDID : ...
     * Signature : ...
     * SecretaryGeneralSignature : ...
     * CRCouncilMemberDID : ...
     */

    private String CategoryData;
    private String OwnerPublicKey;
    private String DraftHash;
    private String SecretaryGeneralPublicKey;
    private String SecretaryGeneralDID;
    private String Signature;
    private String SecretaryGeneralSignature;
    private String CRCouncilMemberDID;
    private String CRCouncilMemberSignature;

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

    public String getSecretaryGeneralPublicKey() {
        return SecretaryGeneralPublicKey;
    }

    public void setSecretaryGeneralPublicKey(String SecretaryGeneralPublicKey) {
        this.SecretaryGeneralPublicKey = SecretaryGeneralPublicKey;
    }

    public String getSecretaryGeneralDID() {
        return SecretaryGeneralDID;
    }

    public void setSecretaryGeneralDID(String SecretaryGeneralDID) {
        this.SecretaryGeneralDID = SecretaryGeneralDID;
    }

    public String getSignature() {
        return Signature;
    }

    public void setSignature(String Signature) {
        this.Signature = Signature;
    }

    public String getSecretaryGeneralSignature() {
        return SecretaryGeneralSignature;
    }

    public void setSecretaryGeneralSignature(String SecretaryGeneralSignature) {
        this.SecretaryGeneralSignature = SecretaryGeneralSignature;
    }

    public String getCRCouncilMemberDID() {
        return CRCouncilMemberDID;
    }

    public void setCRCouncilMemberDID(String CRCouncilMemberDID) {
        this.CRCouncilMemberDID = CRCouncilMemberDID;
    }

    public String getCRCouncilMemberSignature() {
        return CRCouncilMemberSignature;
    }

    public void setCRCouncilMemberSignature(String CRCouncilMemberSignature) {
        this.CRCouncilMemberSignature = CRCouncilMemberSignature;
    }
}
