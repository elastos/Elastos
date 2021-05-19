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
 * 专为接口参数准备的entity process整个过程的的签名和发交易
 */
public class ProposalProcessPayLoad {


    /**
     * ProposalHash : 7c5d2e7cfd7d4011414b5ddb3ab43e2aca247e342d064d1091644606748d7513
     * MessageHash : 0b5ee188b455ab5605cd452d7dda5c205563e1b30c56e93c6b9fda133f8cc4d4
     * Stage : 0
     * OwnerPublicKey : 02c632e27b19260d80d58a857d2acd9eb603f698445cc07ba94d52296468706331
     * NewOwnerPublicKey : 02c632e27b19260d80d58a857d2acd9eb603f698445cc07ba94d52296468706331
     * OwnerSignature : 9a24a084a6f599db9906594800b6cb077fa7995732c575d4d125c935446c93bbe594ee59e361f4d5c2142856c89c5d70c8811048bfb2f8620fbc18a06cb58109
     * NewOwnerSignature : 9a24a084a6f599db9906594800b6cb077fa7995732c575d4d125c935446c93bbe594ee59e361f4d5c2142856c89c5d70c8811048bfb2f8620fbc18a06cb58109
     * Type : 0
     * SecretaryGeneralOpinionHash : 7c5d2e7cfd7d4011414b5ddb3ab43e2aca247e342d064d1091644606748d7513
     * SecretaryGeneralSignature : 9a24a084a6f599db9906594800b6cb077fa7995732c575d4d125c935446c93bbe594ee59e361f4d5c2142856c89c5d70c8811048bfb2f8620fbc18a06cb58109
     */

    private String ProposalHash;
    private String MessageHash;
    private int Stage;
    private String OwnerPublicKey;
    private String NewOwnerPublicKey;
    private String OwnerSignature;
    private String NewOwnerSignature;
    private int Type;
    private String SecretaryGeneralOpinionHash;
    private String SecretaryGeneralSignature;

    public String getProposalHash() {
        return ProposalHash;
    }

    public void setProposalHash(String ProposalHash) {
        this.ProposalHash = ProposalHash;
    }

    public String getMessageHash() {
        return MessageHash;
    }

    public void setMessageHash(String MessageHash) {
        this.MessageHash = MessageHash;
    }

    public int getStage() {
        return Stage;
    }

    public void setStage(int Stage) {
        this.Stage = Stage;
    }

    public String getOwnerPublicKey() {
        return OwnerPublicKey;
    }

    public void setOwnerPublicKey(String OwnerPublicKey) {
        this.OwnerPublicKey = OwnerPublicKey;
    }

    public String getNewOwnerPublicKey() {
        return NewOwnerPublicKey;
    }

    public void setNewOwnerPublicKey(String NewOwnerPublicKey) {
        this.NewOwnerPublicKey = NewOwnerPublicKey;
    }

    public String getOwnerSignature() {
        return OwnerSignature;
    }

    public void setOwnerSignature(String OwnerSignature) {
        this.OwnerSignature = OwnerSignature;
    }

    public String getNewOwnerSignature() {
        return NewOwnerSignature;
    }

    public void setNewOwnerSignature(String NewOwnerSignature) {
        this.NewOwnerSignature = NewOwnerSignature;
    }

    public int getType() {
        return Type;
    }

    public void setType(int Type) {
        this.Type = Type;
    }

    public String getSecretaryGeneralOpinionHash() {
        return SecretaryGeneralOpinionHash;
    }

    public void setSecretaryGeneralOpinionHash(String SecretaryGeneralOpinionHash) {
        this.SecretaryGeneralOpinionHash = SecretaryGeneralOpinionHash;
    }

    public String getSecretaryGeneralSignature() {
        return SecretaryGeneralSignature;
    }

    public void setSecretaryGeneralSignature(String SecretaryGeneralSignature) {
        this.SecretaryGeneralSignature = SecretaryGeneralSignature;
    }
}
