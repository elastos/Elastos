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

    public String CreateDepositTransaction(String fromAddress, String lockedAddress, String amount, String sideChainAddress, String memo) throws WalletException {
        return CreateDepositTransaction(mMainchainProxy, fromAddress, lockedAddress, amount, sideChainAddress, memo);
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

    public String CreateVoteProducerTransaction(String fromAddress, String stake, String publicKeys, String memo) throws WalletException {
        return CreateVoteProducerTransaction(mMainchainProxy, fromAddress, stake, publicKeys, memo);
    }

    public String GetVotedProducerList() throws WalletException {
        return GetVotedProducerList(mMainchainProxy);
    }

    public String GetRegisteredProducerInfo() throws WalletException {
        return GetRegisteredProducerInfo(mMainchainProxy);
    }

    public String GetOwnerAddress() throws WalletException {
        return GetOwnerAddress(mMainchainProxy);
    }

    public String GenerateCRInfoPayload(String crPublickey, String nickName, String url,
                                        long location, String payPasswd) throws WalletException {
        return GenerateCRInfoPayload(mMainchainProxy, crPublickey, nickName, url, location, payPasswd);
    }

    public String GenerateUnregisterCRPayload(String crDID, String payPasswd) throws WalletException {
        return GenerateUnregisterCRPayload(mMainchainProxy, crDID, payPasswd);
    }

    public String GetCROwnerDID() throws WalletException {
        return GetCROwnerDID(mMainchainProxy);
    }

    public String GetCROwnerPublicKey() throws WalletException {
        return GetCROwnerPublicKey(mMainchainProxy);
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

    public String CreateRetrieveCRDepositTransaction(String amount, String memo) throws WalletException {
        return CreateRetrieveCRDepositTransaction(mMainchainProxy, amount, memo);
    }

    public String CreateVoteCRTransaction(String fromAddress, String votes, String memo) throws WalletException {
        return CreateVoteCRTransaction(mMainchainProxy, fromAddress, votes, memo);
    }

    public String GetVotedCRList() throws WalletException {
        return GetVotedCRList(mMainchainProxy);
    }

    public String GetRegisteredCRInfo() throws WalletException {
        return GetRegisteredCRInfo(mMainchainProxy);
    }

    public String SponsorProposalDigest(byte type, String sponsorPublicKey, String crSponsorDID, String draftHash, String budgets, String recipient) throws WalletException {
        return SponsorProposalDigest(mMainchainProxy, type, sponsorPublicKey, crSponsorDID, draftHash, budgets, recipient);
    }

    public String CRSponsorProposalDigest(byte type, String sponsorPublicKey, String crSponsorDID, String draftHash, String budgets, String recipient, String signature) throws WalletException {
        return CRSponsorProposalDigest(mMainchainProxy, type, sponsorPublicKey, crSponsorDID, draftHash, budgets, recipient, signature);
    }

    public String CreateCRCProposalTransaction(byte type, String sponsorPublicKey, String crSponsorDID, String draftHash, String budgets, String recipient, String signature, String crSponsorSignature, String memo) throws WalletException {
        return CreateCRCProposalTransaction(mMainchainProxy, type, sponsorPublicKey, crSponsorDID, draftHash, budgets, recipient, signature, crSponsorSignature, memo);
    }

    public String CreateVoteCRCProposalTransaction(String fromAddress, String votes, String memo) throws WalletException {
        return CreateVoteCRCProposalTransaction(mMainchainProxy, fromAddress, votes, memo);
    }

    private native String CreateDepositTransaction(long proxy, String fromAddress, String lockedAddress, String amount,
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

    private native String CreateVoteProducerTransaction(long proxy, String fromAddress, String stake, String publicKeys, String memo);

    private native String GetVotedProducerList(long proxy);

    private native String GetRegisteredProducerInfo(long proxy);

    private native String GetOwnerAddress(long proxy);

    private native String GenerateCRInfoPayload(long proxy, String crPublickey, String nickName, String url, long location, String payPasswd);

    private native String GenerateUnregisterCRPayload(long proxy, String crDID, String payPasswd);

    private native String GetCROwnerDID(long proxy);

    private native String GetCROwnerPublicKey(long proxy);

    private native String CreateRegisterCRTransaction(long proxy, String fromAddress, String payload, String amount, String memo);

    private native String CreateUpdateCRTransaction(long proxy, String fromAddress, String payload, String memo);

    private native String CreateUnregisterCRTransaction(long proxy, String fromAddress, String payload, String memo);

    private native String CreateRetrieveCRDepositTransaction(long Proxy, String amount, String memo);

    private native String CreateVoteCRTransaction(long Proxy, String fromAddress, String votes, String memo);

    private native String GetVotedCRList(long Proxy);

    private native String GetRegisteredCRInfo(long Proxy);

    private native String SponsorProposalDigest(long Proxy, byte type, String sponsorPublicKey, String crSponsorDID, String draftHash, String budgets, String recipient);

    private native String CRSponsorProposalDigest(long Proxy, byte type, String sponsorPublicKey, String crSponsorDID, String draftHash, String budgets, String recipient, String signature);

    private native String CreateCRCProposalTransaction(long Proxy, byte type, String sponsorPublicKey, String crSponsorDID, String draftHash, String budgets, String recipient, String signature, String crSponsorSignature, String memo);

    private native String CreateVoteCRCProposalTransaction(long Proxy, String fromAddress, String votes, String memo);

}
