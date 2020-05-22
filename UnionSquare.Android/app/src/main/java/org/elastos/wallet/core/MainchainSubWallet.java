// Copyright (c) 2012-2019 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

package org.elastos.wallet.core;

/**
 * MainchainSubWallet jni
 */
public class MainchainSubWallet extends SubWallet {
    private long mMainchainProxy;

    public MainchainSubWallet(long proxy) {
        super(proxy);
        mMainchainProxy = proxy;
    }

    public String CreateDepositTransaction(String fromAddress, String sideChainID, String amount, String sideChainAddress, String memo) throws WalletException {
        return CreateDepositTransaction(mMainchainProxy, fromAddress, sideChainID, amount, sideChainAddress, memo);
    }

    public String GenerateProducerPayload(String publicKey, String nodePublicKey, String nickName, String url, String IPAddress, long location, String payPasswd) throws WalletException {
        return GenerateProducerPayload(mMainchainProxy, publicKey, nodePublicKey, nickName, url, IPAddress, location, payPasswd);
    }

    public String GenerateCancelProducerPayload(String publicKey, String payPasswd) throws WalletException {
        return GenerateCancelProducerPayload(mMainchainProxy, publicKey, payPasswd);
    }

    public String CreateRegisterProducerTransaction(String fromAddress, String payloadJson, String amount, String memo) throws WalletException {
        return CreateRegisterProducerTransaction(mMainchainProxy, fromAddress, payloadJson, amount, memo);
    }

    public String CreateUpdateProducerTransaction(String fromAddress, String payloadJson, String memo) throws WalletException {
        return CreateUpdateProducerTransaction(mMainchainProxy, fromAddress, payloadJson, memo);
    }

    public String CreateCancelProducerTransaction(String fromAddress, String payloadJson, String memo) throws WalletException {
        return CreateCancelProducerTransaction(mMainchainProxy, fromAddress, payloadJson, memo);
    }

    public String CreateRetrieveDepositTransaction(String amount, String memo) throws WalletException {
        return CreateRetrieveDepositTransaction(mMainchainProxy, amount, memo);
    }

    public String GetOwnerPublicKey() throws WalletException {
        return GetOwnerPublicKey(mMainchainProxy);
    }

    public String CreateVoteProducerTransaction(String fromAddress, String stake, String publicKeys, String memo, String invalidCandidates) throws WalletException {
        return CreateVoteProducerTransaction(mMainchainProxy, fromAddress, stake, publicKeys, memo, invalidCandidates);
    }

    public String GetVotedProducerList() throws WalletException {
        return GetVotedProducerList(mMainchainProxy);
    }

    public String GetVoteInfo(String type) throws WalletException {
        return GetVoteInfo(mMainchainProxy, type);
    }

    public String GetRegisteredProducerInfo() throws WalletException {
        return GetRegisteredProducerInfo(mMainchainProxy);
    }

    public String GetOwnerAddress() throws WalletException {
        return GetOwnerAddress(mMainchainProxy);
    }

    public String GenerateCRInfoPayload(String crPublickey, String did, String nickName, String url,
                                        long location) throws WalletException {
        return GenerateCRInfoPayload(mMainchainProxy, crPublickey, did, nickName, url, location);
    }

    public String GenerateUnregisterCRPayload(String cID) throws WalletException {
        return GenerateUnregisterCRPayload(mMainchainProxy, cID);
    }

    public String CreateRegisterCRTransaction(String fromAddress, String payload, String amount, String memo) throws WalletException {
        return CreateRegisterCRTransaction(mMainchainProxy, fromAddress, payload, amount, memo);
    }

    public String CreateUpdateCRTransaction(String fromAddress, String payload, String memo) throws WalletException {
        return CreateUpdateCRTransaction(mMainchainProxy, fromAddress, payload, memo);
    }

    public String CreateUnregisterCRTransaction(String fromAddress, String payload, String memo) throws WalletException {
        return CreateUnregisterCRTransaction(mMainchainProxy, fromAddress, payload, memo);
    }

    public String CreateRetrieveCRDepositTransaction(String crPublickey, String amount, String memo) throws WalletException {
        return CreateRetrieveCRDepositTransaction(mMainchainProxy, crPublickey, amount, memo);
    }

    public String CreateVoteCRTransaction(String fromAddress, String votes, String memo, String invalidCandidates) throws WalletException {
        return CreateVoteCRTransaction(mMainchainProxy, fromAddress, votes, memo, invalidCandidates);
    }

    public String GetVotedCRList() throws WalletException {
        return GetVotedCRList(mMainchainProxy);
    }

    public String GetRegisteredCRInfo() throws WalletException {
        return GetRegisteredCRInfo(mMainchainProxy);
    }

    public String ProposalOwnerDigest(String payload) throws WalletException {
        return ProposalOwnerDigest(mMainchainProxy, payload);
    }

    public String ProposalCRCouncilMemberDigest(String payload) throws WalletException {
        return ProposalCRCouncilMemberDigest(mMainchainProxy, payload);
    }

    public String CalculateProposalHash(String payload) throws WalletException {
        return CalculateProposalHash(mMainchainProxy, payload);
    }

    public String CreateProposalTransaction(String payload, String memo) throws WalletException {
        return CreateProposalTransaction(mMainchainProxy, payload, memo);
    }

    public String CreateVoteCRCProposalTransaction(String fromAddress, String votes, String memo, String invalidCandidates) throws WalletException {
        return CreateVoteCRCProposalTransaction(mMainchainProxy, fromAddress, votes, memo, invalidCandidates);
    }

    public String CreateImpeachmentCRCTransaction(String fromAddress, String votes, String memo, String invalidCandidates) throws WalletException {
        return CreateImpeachmentCRCTransaction(mMainchainProxy, fromAddress, votes, memo, invalidCandidates);
    }

    public String ProposalReviewDigest(String payload) throws WalletException {
        return ProposalReviewDigest(mMainchainProxy, payload);
    }

    public String CreateProposalReviewTransaction( String payload, String memo) throws WalletException {
        return CreateProposalReviewTransaction(mMainchainProxy, payload, memo);
    }

    public String ProposalTrackingOwnerDigest(String payload) {
        return ProposalTrackingOwnerDigest(mMainchainProxy, payload);
    }

    public String ProposalTrackingNewOwnerDigest(String payload) {
        return ProposalTrackingNewOwnerDigest(mMainchainProxy,payload);
    }

    public String ProposalTrackingSecretaryDigest(String payload) {
        return ProposalTrackingSecretaryDigest(mMainchainProxy,payload);
    }

    public String CreateProposalTrackingTransaction(String payload, String memo) {
        return CreateProposalTrackingTransaction(mMainchainProxy, payload, memo);
    }

    public String ProposalWithdrawDigest(String payload) {
        return ProposalWithdrawDigest(mMainchainProxy, payload);
    }

    public String CreateProposalWithdrawTransaction(String recipient, String amount, String utxo, String payload, String memo) {
        return CreateProposalWithdrawTransaction(mMainchainProxy, recipient, amount, utxo, payload, memo);
    }

    private native String CreateDepositTransaction(long proxy, String fromAddress, String sideChainID, String amount,
                                                   String sideChainAddress, String memo);

    private native String GenerateProducerPayload(long proxy, String publicKey, String nodePublicKey, String nickName,
                                                  String url, String IPAddress, long location, String payPasswd);

    private native String GenerateCancelProducerPayload(long proxy, String publicKey, String payPasswd);

    private native String CreateRegisterProducerTransaction(long proxy, String fromAddress, String payloadJson, String amount,
                                                            String memo);

    private native String CreateUpdateProducerTransaction(long proxy, String fromAddress, String payloadJson, String memo);

    private native String CreateCancelProducerTransaction(long proxy, String fromAddress, String payloadJson, String memo);

    private native String CreateRetrieveDepositTransaction(long proxy, String amount, String memo);

    private native String GetOwnerPublicKey(long proxy);

    private native String CreateVoteProducerTransaction(long proxy, String fromAddress, String stake, String publicKeys, String memo, String invalidCandidates);

    private native String GetVotedProducerList(long proxy);

    private native String GetRegisteredProducerInfo(long proxy);

    private native String GetOwnerAddress(long proxy);

    private native String GenerateCRInfoPayload(long proxy, String crPublickey, String did, String nickName, String url, long location);

    private native String GenerateUnregisterCRPayload(long proxy, String cID);

    private native String CreateRegisterCRTransaction(long proxy, String fromAddress, String payload, String amount, String memo);

    private native String CreateUpdateCRTransaction(long proxy, String fromAddress, String payload, String memo);

    private native String CreateUnregisterCRTransaction(long proxy, String fromAddress, String payload, String memo);

    private native String CreateRetrieveCRDepositTransaction(long Proxy, String crPublickey, String amount, String memo);

    private native String CreateVoteCRTransaction(long Proxy, String fromAddress, String votes, String memo, String invalidCandidates);

    private native String GetVotedCRList(long Proxy);

    private native String GetRegisteredCRInfo(long Proxy);

    private native String ProposalOwnerDigest(long Proxy, String payload);

    private native String ProposalCRCouncilMemberDigest(long Proxy, String payload);

    private native String CalculateProposalHash(long Proxy, String payload);

    private native String CreateProposalTransaction(long Proxy, String payload, String memo);

    private native String CreateVoteCRCProposalTransaction(long Proxy, String fromAddress, String votes, String memo, String invalidCandidates);

    private native String CreateImpeachmentCRCTransaction(long Proxy, String fromAddress, String votes, String memo, String invalidCandidates);

    private native String GetVoteInfo(long Proxy, String type);

    private native String ProposalReviewDigest(long Proxy, String payload);

    private native String CreateProposalReviewTransaction(long Proxy, String payload, String memo);

    private native String ProposalTrackingOwnerDigest(long Proxy, String payload);

    private native String ProposalTrackingNewOwnerDigest(long Proxy, String payload);

    private native String ProposalTrackingSecretaryDigest(long Proxy, String payload);

    private native String CreateProposalTrackingTransaction(long Proxy, String payload, String memo);

    private native String ProposalWithdrawDigest(long Proxy, String payload);

    private native String CreateProposalWithdrawTransaction(long Proxy, String recipient, String amount, String utxo, String payload, String memo);

}
