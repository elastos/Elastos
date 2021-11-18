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
#include "MainchainSubWallet.h"
#include "MasterWallet.h"

#include <Common/Utils.h>
#include <Common/Log.h>
#include <Common/ErrorChecker.h>
#include <WalletCore/Key.h>
#include <WalletCore/CoinInfo.h>
#include <Wallet/UTXO.h>
#include <Wallet/WalletCommon.h>
#include <Plugin/Transaction/Asset.h>
#include <Plugin/Transaction/Payload/TransferCrossChainAsset.h>
#include <Plugin/Transaction/Payload/ProducerInfo.h>
#include <Plugin/Transaction/Payload/CancelProducer.h>
#include <Plugin/Transaction/Payload/OutputPayload/PayloadVote.h>
#include <Plugin/Transaction/Payload/CRInfo.h>
#include <Plugin/Transaction/Payload/UnregisterCR.h>
#include <Plugin/Transaction/Payload/CRCProposal.h>
#include <Plugin/Transaction/Payload/CRCProposalReview.h>
#include <Plugin/Transaction/Payload/CRCProposalTracking.h>
#include <Plugin/Transaction/Payload/CRCProposalWithdraw.h>
#include <Plugin/Transaction/TransactionOutput.h>
#include <Plugin/Transaction/Attribute.h>
#include <SpvService/Config.h>
#include <Plugin/Transaction/Payload/ReturnDepositCoin.h>
#include <Plugin/Transaction/Payload/CRCouncilMemberClaimNode.h>
#include <CMakeConfig.h>

#include <vector>
#include <map>
#include <ethereum/base/BREthereumAddress.h>
#include <ethereum/base/BREthereumLogic.h>
#include <Plugin/Transaction/Payload/OutputPayload/PayloadCrossChain.h>
#include <Plugin/Transaction/Payload/TransferAsset.h>

namespace Elastos {
	namespace ElaWallet {

#define DEPOSIT_MIN_ELA 5000

		MainchainSubWallet::MainchainSubWallet(const CoinInfoPtr &info,
											   const ChainConfigPtr &config,
											   MasterWallet *parent,
											   const std::string &netType) :
                ElastosBaseSubWallet(info, config, parent, netType) {
		}

		MainchainSubWallet::~MainchainSubWallet() {
		}

		nlohmann::json MainchainSubWallet::CreateDepositTransaction(uint8_t version,
                                                                    const nlohmann::json &inputsJson,
																	const std::string &sideChainID,
																	const std::string &amount,
																	const std::string &sideChainAddress,
                                                                    const std::string &lockAddress,
																	const std::string &fee,
																	const std::string &memo) const {
			WalletPtr wallet = _walletManager->GetWallet();
			ArgInfo("{} {}", wallet->GetWalletID(), GetFunName());
			ArgInfo("version: {}", version);
			ArgInfo("inputs: {}", inputsJson.dump());
			ArgInfo("sideChainID: {}", sideChainID);
			ArgInfo("amount: {}", amount);
			ArgInfo("sideChainAddr: {}", sideChainAddress);
			ArgInfo("lockAddress: {}", lockAddress);
			ArgInfo("fee: {}", fee);
			ArgInfo("memo: {}", memo);

            UTXOSet utxos;
            UTXOFromJson(utxos, inputsJson);

            if (version != TransferCrossChainVersion && version != TransferCrossChainVersionV1)
                ErrorChecker::ThrowParamException(Error::InvalidArgument, "invalid version");
			uint8_t payloadVersion = version;
			ErrorChecker::CheckBigIntAmount(amount);
			ErrorChecker::CheckParam(sideChainID == CHAINID_MAINCHAIN, Error::InvalidArgument, "can not be mainChain");

			BigInt bgAmount, feeAmount;
			bgAmount.setDec(amount);
			feeAmount.setDec(fee);

			if (sideChainID == CHAINID_IDCHAIN || sideChainID == CHAINID_TOKENCHAIN) {
				Address addressValidate(sideChainAddress);
				ErrorChecker::CheckParam(!addressValidate.Valid(), Error::Address, "invalid standard address");
			} else if (sideChainID.find("ETH") != std::string::npos) {
				ErrorChecker::CheckParam(addressValidateString(sideChainAddress.c_str()) != ETHEREUM_BOOLEAN_TRUE, Error::Address, "invalid ethsc address");
			} else {
			    ErrorChecker::ThrowParamException(Error::InvalidArgument, "invalid chain id");
			}

			PayloadPtr payload;
			OutputArray outputs;
			Address receiveAddr(lockAddress);

			if (payloadVersion == TransferCrossChainVersion) {
				TransferInfo info(sideChainAddress, 0, bgAmount);
				payload = PayloadPtr(new TransferCrossChainAsset({info}));
				outputs.emplace_back(OutputPtr(new TransactionOutput(bgAmount + DEPOSIT_OR_WITHDRAW_FEE, receiveAddr)));
			} else if (payloadVersion == TransferCrossChainVersionV1) {
				payload = PayloadPtr(new TransferCrossChainAsset());
				OutputPayloadPtr outputPayload(new PayloadCrossChain(CrossChainOutputVersion, sideChainAddress, bgAmount, bytes_t()));
				outputs.emplace_back(OutputPtr(new TransactionOutput(bgAmount + DEPOSIT_OR_WITHDRAW_FEE, receiveAddr, Asset::GetELAAssetID(), TransactionOutput::CrossChain, outputPayload)));
			}

			TransactionPtr tx = wallet->CreateTransaction(Transaction::transferCrossChainAsset, payload, utxos, outputs, memo, feeAmount);
			tx->SetPayloadVersion(payloadVersion);

			nlohmann::json result;
			EncodeTx(result, tx);

			ArgInfo("r => {}", result.dump());
			return result;
		}

        std::string MainchainSubWallet::GetDepositAddress(const std::string &pubkey) const {
		    bytes_t pub;
		    pub.setHex(pubkey);
            Address depositAddress(PrefixDeposit, pub);
            return depositAddress.String();
		}

		nlohmann::json MainchainSubWallet::GenerateProducerPayload(
			const std::string &ownerPublicKey,
			const std::string &nodePublicKey,
			const std::string &nickName,
			const std::string &url,
			const std::string &ipAddress,
			uint64_t location,
			const std::string &payPasswd) const {

			ArgInfo("{} {}", _walletManager->GetWallet()->GetWalletID(), GetFunName());
			ArgInfo("ownerPubKey: {}", ownerPublicKey);
			ArgInfo("nodePubKey: {}", nodePublicKey);
			ArgInfo("nickName: {}", nickName);
			ArgInfo("url: {}", url);
			ArgInfo("ipAddress: {}", ipAddress);
			ArgInfo("location: {}", location);
			ArgInfo("payPasswd: *");

			ErrorChecker::CheckPassword(payPasswd, "Generate payload");

			Key verifyPubKey;
			bytes_t ownerPubKey = bytes_t(ownerPublicKey);
			verifyPubKey.SetPubKey(CTElastos, ownerPubKey);

			bytes_t nodePubKey = bytes_t(nodePublicKey);
			verifyPubKey.SetPubKey(CTElastos, nodePubKey);

			ProducerInfo pr;
			pr.SetPublicKey(ownerPubKey);
			pr.SetNodePublicKey(nodePubKey);
			pr.SetNickName(nickName);
			pr.SetUrl(url);
			pr.SetAddress(ipAddress);
			pr.SetLocation(location);

			ByteStream ostream;
			pr.SerializeUnsigned(ostream, 0);
			bytes_t prUnsigned = ostream.GetBytes();

			pr.SetSignature(_walletManager->GetWallet()->SignWithOwnerKey(prUnsigned, payPasswd));

			nlohmann::json payloadJson = pr.ToJson(0);

			ArgInfo("r => {}", payloadJson.dump());
			return payloadJson;
		}

		nlohmann::json MainchainSubWallet::GenerateCancelProducerPayload(
			const std::string &ownerPublicKey,
			const std::string &payPasswd) const {

			ArgInfo("{} {}", _walletManager->GetWallet()->GetWalletID(), GetFunName());
			ArgInfo("ownerPubKey: {}", ownerPublicKey);
			ArgInfo("payPasswd: *");

			ErrorChecker::CheckPassword(payPasswd, "Generate payload");
			size_t pubKeyLen = ownerPublicKey.size() >> 1;
			ErrorChecker::CheckParam(pubKeyLen != 33 && pubKeyLen != 65, Error::PubKeyLength,
									 "Public key length should be 33 or 65 bytes");

			CancelProducer pc;
			pc.SetPublicKey(ownerPublicKey);

			ByteStream ostream;
			pc.SerializeUnsigned(ostream, 0);
			bytes_t pcUnsigned = ostream.GetBytes();

			pc.SetSignature(_walletManager->GetWallet()->SignWithOwnerKey(pcUnsigned, payPasswd));

			nlohmann::json payloadJson = pc.ToJson(0);
			ArgInfo("r => {}", payloadJson.dump());
			return payloadJson;
		}

		nlohmann::json MainchainSubWallet::CreateRegisterProducerTransaction(
			const nlohmann::json &inputsJson,
			const nlohmann::json &payloadJson,
			const std::string &amount,
			const std::string &fee,
			const std::string &memo) const {
			WalletPtr wallet = _walletManager->GetWallet();

			ArgInfo("{} {}", wallet->GetWalletID(), GetFunName());
			ArgInfo("inputs: {}", inputsJson.dump());
			ArgInfo("payload: {}", payloadJson.dump());
			ArgInfo("amount: {}", amount);
            ArgInfo("fee: {}", fee);
			ArgInfo("memo: {}", memo);

			UTXOSet utxo;
			UTXOFromJson(utxo, inputsJson);

			ErrorChecker::CheckBigIntAmount(amount);
			BigInt bgAmount, minAmount(DEPOSIT_MIN_ELA), feeAmount;
			bgAmount.setDec(amount);
			feeAmount.setDec(fee);

			minAmount *= SELA_PER_ELA;

			ErrorChecker::CheckParam(bgAmount < minAmount, Error::DepositAmountInsufficient,
									 "Producer deposit amount is insufficient");

			PayloadPtr payload = PayloadPtr(new ProducerInfo());
			try {
				payload->FromJson(payloadJson, 0);
			} catch (const nlohmann::detail::exception &e) {
				ErrorChecker::ThrowParamException(Error::JsonFormatError,
												  "Payload format err: " + std::string(e.what()));
			}

			bytes_t pubkey = static_cast<ProducerInfo *>(payload.get())->GetPublicKey();

			OutputArray outputs;
			Address receiveAddr(PrefixDeposit, pubkey);
			outputs.push_back(OutputPtr(new TransactionOutput(bgAmount, receiveAddr)));

			TransactionPtr tx = wallet->CreateTransaction(Transaction::registerProducer, payload, utxo, outputs, memo, feeAmount);

			nlohmann::json result;
			EncodeTx(result, tx);

			ArgInfo("r => {}", result.dump());
			return result;
		}

        nlohmann::json MainchainSubWallet::CreateUpdateProducerTransaction(const nlohmann::json &inputsJson,
                                                                           const nlohmann::json &payloadJson,
                                                                           const std::string &fee,
                                                                           const std::string &memo) const {

			WalletPtr wallet = _walletManager->GetWallet();
			ArgInfo("{} {}", wallet->GetWalletID(), GetFunName());
			ArgInfo("inputs: {}", inputsJson.dump());
			ArgInfo("payload: {}", payloadJson.dump());
			ArgInfo("fee: {}", fee);
			ArgInfo("memo: {}", memo);

			UTXOSet utxo;
			UTXOFromJson(utxo, inputsJson);

			PayloadPtr payload = PayloadPtr(new ProducerInfo());
			try {
				payload->FromJson(payloadJson, 0);
			} catch (const nlohmann::detail::exception &e) {
				ErrorChecker::ThrowParamException(Error::JsonFormatError,
												  "Payload format err: " + std::string(e.what()));
			}

			BigInt feeAmount;
			feeAmount.setDec(fee);

			TransactionPtr tx = wallet->CreateTransaction(Transaction::updateProducer, payload, utxo, {}, memo, feeAmount);

			nlohmann::json result;
			EncodeTx(result, tx);

			ArgInfo("r => {}", result.dump());
			return result;
		}

		nlohmann::json MainchainSubWallet::CreateCancelProducerTransaction(
			const nlohmann::json &inputsJson,
			const nlohmann::json &payloadJson,
			const std::string &fee,
			const std::string &memo) const {

			WalletPtr wallet = _walletManager->GetWallet();
			ArgInfo("{} {}", wallet->GetWalletID(), GetFunName());
			ArgInfo("inputs: {}", inputsJson.dump());
			ArgInfo("payload: {}", payloadJson.dump());
			ArgInfo("fee: {}", fee);
			ArgInfo("memo: {}", memo);

			UTXOSet utxo;
			UTXOFromJson(utxo, inputsJson);

			PayloadPtr payload = PayloadPtr(new CancelProducer());
			try {
				payload->FromJson(payloadJson, 0);
			} catch (const nlohmann::detail::exception &e) {
				ErrorChecker::ThrowParamException(Error::JsonFormatError,
												  "Payload format err: " + std::string(e.what()));
			}

			BigInt feeAmount;
			feeAmount.setDec(fee);

			TransactionPtr tx = wallet->CreateTransaction(Transaction::cancelProducer, payload, utxo, {}, memo, feeAmount);

			nlohmann::json result;
			EncodeTx(result, tx);

			ArgInfo("r => {}", result.dump());
			return result;
		}

        nlohmann::json MainchainSubWallet::CreateRetrieveDepositTransaction(const nlohmann::json &inputsJson,
                                                                            const std::string &amount,
                                                                            const std::string &fee,
                                                                            const std::string &memo) const {
            WalletPtr wallet = _walletManager->GetWallet();
			ArgInfo("{} {}", wallet->GetWalletID(), GetFunName());
			ArgInfo("inputs: {}", inputsJson.dump());
            ArgInfo("amount: {}", amount);
			ArgInfo("fee: {}", fee);
			ArgInfo("memo: {}", memo);

			UTXOSet utxo;
			UTXOFromJson(utxo, inputsJson);

			BigInt feeAmount, bgAmount;
			feeAmount.setDec(fee);
			bgAmount.setDec(amount);

            OutputArray outputs;
            Address receiveAddr = (*utxo.begin())->GetAddress();
            outputs.push_back(OutputPtr(new TransactionOutput(bgAmount - feeAmount, receiveAddr)));

			PayloadPtr payload = PayloadPtr(new ReturnDepositCoin());
			TransactionPtr tx = _walletManager->GetWallet()->CreateTransaction(
				Transaction::returnDepositCoin, payload, utxo, outputs, memo, feeAmount, true);

			nlohmann::json result;
			EncodeTx(result, tx);

			ArgInfo("r => {}", result.dump());
			return result;
		}

		std::string MainchainSubWallet::GetOwnerPublicKey() const {
			ArgInfo("{} {}", _walletManager->GetWallet()->GetWalletID(), GetFunName());
			std::string publicKey = _walletManager->GetWallet()->GetOwnerPublilcKey().getHex();
			ArgInfo("r => {}", publicKey);
			return publicKey;
		}

		std::string MainchainSubWallet::GetOwnerAddress() const {
			ArgInfo("{} {}", _walletManager->GetWallet()->GetWalletID(), GetFunName());

			std::string address = _walletManager->GetWallet()->GetOwnerAddress()->String();

			ArgInfo("r => {}", address);

			return address;
		}

        std::string MainchainSubWallet::GetOwnerDepositAddress() const {
		    WalletPtr wallet = _walletManager->GetWallet();
            ArgInfo("{} {}", wallet->GetWalletID(), GetFunName());

            AddressPtr addrPtr = wallet->GetOwnerDepositAddress();
            std::string addr = addrPtr->String();

            ArgInfo("r => {}", addr);

            return addr;
		}

        bool MainchainSubWallet::VoteAmountFromJson(BigInt &voteAmount, const nlohmann::json &j) const {
            ErrorChecker::CheckParam(!j.is_string(), Error::InvalidArgument, "stake value should be big int string");
            std::string voteAmountString = j.get<std::string>();
            ErrorChecker::CheckBigIntAmount(voteAmountString);
            voteAmount.setDec(voteAmountString);
            ErrorChecker::CheckParam(voteAmount <= 0, Error::InvalidArgument, "stake value should larger than 0");

            return true;
		}

        bool MainchainSubWallet::VoteContentFromJson(VoteContentArray &voteContents, BigInt &maxAmount, const nlohmann::json &j) const {
		    BigInt tmpAmount;

            for (nlohmann::json::const_iterator it = j.cbegin(); it != j.cend(); ++it) {
                if ((*it)["Type"].get<std::string>() == "CRC") {
                    VoteContent vc(VoteContent::Type::CRC);
                    nlohmann::json candidateVotesJson = (*it)["Candidates"];
                    for (nlohmann::json::iterator it = candidateVotesJson.begin(); it != candidateVotesJson.end(); ++it) {
                        BigInt voteAmount;
                        VoteAmountFromJson(voteAmount, it.value());

                        std::string key = it.key();
                        Address cid(key);
                        ErrorChecker::CheckParam(!cid.Valid(), Error::InvalidArgument, "invalid candidate cid");
                        bytes_t candidate = cid.ProgramHash().bytes();

                        vc.AddCandidate(CandidateVotes(candidate, voteAmount));
                    }
                    tmpAmount = vc.GetTotalVoteAmount();
                    if (tmpAmount > maxAmount)
                        maxAmount = tmpAmount;
                    voteContents.push_back(vc);
                } else if ((*it)["Type"].get<std::string>() == "CRCProposal") {
                    VoteContent vc(VoteContent::Type::CRCProposal);
                    nlohmann::json candidateVotesJson = (*it)["Candidates"];
                    for (nlohmann::json::iterator it = candidateVotesJson.begin(); it != candidateVotesJson.end(); ++it) {
                        BigInt voteAmount;
                        VoteAmountFromJson(voteAmount, it.value());

                        uint256 proposalHash;
                        proposalHash.SetHex(std::string(it.key()));
                        ErrorChecker::CheckParam(proposalHash.size() != 32, Error::InvalidArgument, "invalid proposal hash");

                        vc.AddCandidate(CandidateVotes(proposalHash.bytes(), voteAmount));
                    }
                    tmpAmount = vc.GetMaxVoteAmount();
                    if (tmpAmount > maxAmount)
                        maxAmount = tmpAmount;
                    voteContents.push_back(vc);
                } else if ((*it)["Type"].get<std::string>() == "CRCImpeachment") {
                    VoteContent vc(VoteContent::Type::CRCImpeachment);
                    nlohmann::json candidateVotesJson = (*it)["Candidates"];
                    for (nlohmann::json::iterator it = candidateVotesJson.begin(); it != candidateVotesJson.end(); ++it) {
                        BigInt voteAmount;
                        VoteAmountFromJson(voteAmount, it.value());

                        std::string key = it.key();
                        Address cid(key);
                        ErrorChecker::CheckParam(!cid.Valid(), Error::InvalidArgument, "invalid candidate cid");
                        bytes_t candidate = cid.ProgramHash().bytes();

                        vc.AddCandidate(CandidateVotes(candidate, voteAmount));
                    }
                    tmpAmount = vc.GetTotalVoteAmount();
                    if (tmpAmount > maxAmount)
                        maxAmount = tmpAmount;
                    voteContents.push_back(vc);
                } else if ((*it)["Type"].get<std::string>() == "Delegate") {
                    VoteContent vc(VoteContent::Type::Delegate);
                    nlohmann::json candidateVotesJson = (*it)["Candidates"];
                    for (nlohmann::json::iterator it = candidateVotesJson.begin(); it != candidateVotesJson.end(); ++it) {
                        BigInt voteAmount;
                        VoteAmountFromJson(voteAmount, it.value());

                        bytes_t pubkey;
                        pubkey.setHex(it.key());

                        vc.AddCandidate(CandidateVotes(pubkey, voteAmount));
                    }
                    tmpAmount = vc.GetMaxVoteAmount();
                    if (tmpAmount > maxAmount)
                        maxAmount = tmpAmount;
                    voteContents.push_back(vc);
                }
            }

            return true;
		}

        nlohmann::json MainchainSubWallet::CreateVoteTransaction(const nlohmann::json &inputsJson,
                                                                 const nlohmann::json &voteContentsJson,
                                                                 const std::string &fee,
                                                                 const std::string &memo) const {
            WalletPtr wallet = _walletManager->GetWallet();
			ArgInfo("{} {}", wallet->GetWalletID(), GetFunName());
			ArgInfo("inputs: {}", inputsJson.dump());
			ArgInfo("voteContent: {}", voteContentsJson.dump());
			ArgInfo("fee: {}", fee);
			ArgInfo("memo: {}", memo);

            UTXOSet utxos;
            UTXOFromJson(utxos, inputsJson);

            BigInt outputAmount;
            VoteContentArray voteContents;
            VoteContentFromJson(voteContents, outputAmount, voteContentsJson);

            OutputPayloadPtr outputPayload(new PayloadVote(voteContents, VOTE_PRODUCER_CR_VERSION));

            OutputArray outputs;
            OutputPtr output(new TransactionOutput(TransactionOutput(outputAmount, (*utxos.begin())->GetAddress(), Asset::GetELAAssetID(), TransactionOutput::VoteOutput, outputPayload)));
            outputs.push_back(output);

            BigInt feeAmount;
            feeAmount.setDec(fee);

            PayloadPtr payload = PayloadPtr(new TransferAsset());
            TransactionPtr tx = wallet->CreateTransaction(Transaction::transferAsset,
                                                          payload, utxos, outputs, memo, feeAmount, true);

			nlohmann::json result;
			EncodeTx(result, tx);

			ArgInfo("r => {}", result.dump());
			return result;
		}

        std::string MainchainSubWallet::GetCRDepositAddress() const {
            WalletPtr wallet = _walletManager->GetWallet();
            ArgInfo("{} {}", wallet->GetWalletID(), GetFunName());

            AddressPtr addrPtr = wallet->GetCROwnerDepositAddress();
            std::string addr = addrPtr->String();

            ArgInfo("r => {}", addr);

            return addr;
		}

		nlohmann::json MainchainSubWallet::GenerateCRInfoPayload(
				const std::string &crPublicKey,
				const std::string &did,
				const std::string &nickName,
				const std::string &url,
				uint64_t location) const {
			ArgInfo("{} {}", _walletManager->GetWallet()->GetWalletID(), GetFunName());
			ArgInfo("crPublicKey: {}", crPublicKey);
			ArgInfo("did: {}", did);
			ArgInfo("nickName: {}", nickName);
			ArgInfo("url: {}", url);
			ArgInfo("location: {}", location);

			size_t pubKeyLen = crPublicKey.size() >> 1;
			ErrorChecker::CheckParam(pubKeyLen != 33 && pubKeyLen != 65, Error::PubKeyLength,
			                         "Public key length should be 33 or 65 bytes");

			bytes_t pubkey(crPublicKey);

			Address didAddress(did);
			Address address(PrefixStandard, pubkey);

			CRInfo crInfo;
			crInfo.SetCode(address.RedeemScript());
			crInfo.SetDID(didAddress.ProgramHash());
			crInfo.SetNickName(nickName);
			crInfo.SetUrl(url);
			crInfo.SetLocation(location);

			Address cid;
			cid.SetRedeemScript(PrefixIDChain, crInfo.GetCode());
			crInfo.SetCID(cid.ProgramHash());

			ByteStream ostream;
			crInfo.SerializeUnsigned(ostream, CRInfoDIDVersion);
			uint256 digest(sha256(ostream.GetBytes()));

			nlohmann::json payloadJson = crInfo.ToJson(CRInfoDIDVersion);
			payloadJson["Digest"] = digest.GetHex();

			ArgInfo("r => {}", payloadJson.dump());
			return payloadJson;
		}

		nlohmann::json MainchainSubWallet::GenerateUnregisterCRPayload(const std::string &CID) const {
			ArgInfo("{} {}", _walletManager->GetWallet()->GetWalletID(), GetFunName());
			ArgInfo("CID: {}", CID);

			Address cid(CID);
			ErrorChecker::CheckParam(!cid.Valid(), Error::InvalidArgument, "invalid crDID");

			UnregisterCR unregisterCR;
			unregisterCR.SetCID(cid.ProgramHash());

			ByteStream ostream;
			unregisterCR.SerializeUnsigned(ostream, 0);
			uint256 digest(sha256(ostream.GetBytes()));

			nlohmann::json payloadJson = unregisterCR.ToJson(0);
			payloadJson["Digest"] = digest.GetHex();

			ArgInfo("r => {}", payloadJson.dump());
			return payloadJson;
		}

		nlohmann::json MainchainSubWallet::CreateRegisterCRTransaction(
				const nlohmann::json &inputsJson,
				const nlohmann::json &payloadJSON,
				const std::string &amount,
				const std::string &fee,
				const std::string &memo) const {

			WalletPtr wallet = _walletManager->GetWallet();
			ArgInfo("{} {}", wallet->GetWalletID(), GetFunName());
			ArgInfo("inputs: {}", inputsJson.dump());
			ArgInfo("payload: {}", payloadJSON.dump());
			ArgInfo("amount: {}", amount);
			ArgInfo("fee: {}", fee);
			ArgInfo("memo: {}", memo);

			UTXOSet utxo;
			UTXOFromJson(utxo, inputsJson);

			ErrorChecker::CheckBigIntAmount(amount);
			BigInt bgAmount, minAmount(DEPOSIT_MIN_ELA), feeAmount;
			bgAmount.setDec(amount);
			feeAmount.setDec(fee);

			minAmount *= SELA_PER_ELA;

			ErrorChecker::CheckParam(bgAmount < minAmount, Error::DepositAmountInsufficient,
			                         "cr deposit amount is insufficient");

			ErrorChecker::CheckParam(payloadJSON.find("Signature") == payloadJSON.end(), Error::InvalidArgument,
			                         "Signature can not be empty");

			uint8_t payloadVersion = CRInfoDIDVersion;
			PayloadPtr payload = PayloadPtr(new CRInfo());
			try {
				payload->FromJson(payloadJSON, payloadVersion);
				ErrorChecker::CheckParam(!payload->IsValid(payloadVersion), Error::InvalidArgument, "verify signature failed");
			} catch (const nlohmann::detail::exception &e) {
				ErrorChecker::ThrowParamException(Error::JsonFormatError,
				                                  "Payload format err: " + std::string(e.what()));
			}

			bytes_t code = static_cast<CRInfo *>(payload.get())->GetCode();
			Address receiveAddr;
			receiveAddr.SetRedeemScript(PrefixDeposit, code);

			OutputArray outputs;
			outputs.push_back(OutputPtr(new TransactionOutput(bgAmount, receiveAddr)));

			TransactionPtr tx = wallet->CreateTransaction(Transaction::registerCR, payload, utxo, outputs, memo, feeAmount);
			tx->SetPayloadVersion(payloadVersion);

			nlohmann::json result;
			EncodeTx(result, tx);

			ArgInfo("r => {}", result.dump());
			return result;
		}

		nlohmann::json MainchainSubWallet::CreateUpdateCRTransaction(
				const nlohmann::json &inputsJson,
				const nlohmann::json &payloadJSON,
				const std::string &fee,
				const std::string &memo) const {
			WalletPtr wallet = _walletManager->GetWallet();
			ArgInfo("{} {}", wallet->GetWalletID(), GetFunName());
			ArgInfo("inputs: {}", inputsJson.dump());
			ArgInfo("payload: {}", payloadJSON.dump());
            ArgInfo("fee: {}", fee);
			ArgInfo("memo: {}", memo);

			UTXOSet utxo;
			UTXOFromJson(utxo, inputsJson);

			uint8_t payloadVersion = CRInfoDIDVersion;
			PayloadPtr payload = PayloadPtr(new CRInfo());
			try {
				payload->FromJson(payloadJSON, payloadVersion);
			} catch (const nlohmann::detail::exception &e) {
				ErrorChecker::ThrowParamException(Error::JsonFormatError,
				                                  "Payload format err: " + std::string(e.what()));
			}

			BigInt feeAmount;
			feeAmount.setDec(fee);

			TransactionPtr tx = wallet->CreateTransaction(Transaction::updateCR, payload, utxo, {}, memo, feeAmount);
			tx->SetPayloadVersion(payloadVersion);

			nlohmann::json result;
			EncodeTx(result, tx);

			ArgInfo("r => {}", result.dump());
			return result;

		}

		nlohmann::json MainchainSubWallet::CreateUnregisterCRTransaction(
				const nlohmann::json &inputsJson,
				const nlohmann::json &payloadJSON,
				const std::string &fee,
				const std::string &memo) const {
			WalletPtr wallet = _walletManager->GetWallet();
			ArgInfo("{} {}", wallet->GetWalletID(), GetFunName());
			ArgInfo("inputs: {}", inputsJson.dump());
			ArgInfo("payload: {}", payloadJSON.dump());
			ArgInfo("fee: {}", fee);
			ArgInfo("memo: {}", memo);

			UTXOSet utxo;
			UTXOFromJson(utxo, inputsJson);

			ErrorChecker::CheckParam(payloadJSON.find("Signature") == payloadJSON.end() ||
			                         payloadJSON["Signature"].get<std::string>() == "",
			                         Error::InvalidArgument, "invalied signature");

			PayloadPtr payload = PayloadPtr(new UnregisterCR());
			try {
				payload->FromJson(payloadJSON, 0);
			} catch (const nlohmann::detail::exception &e) {
				ErrorChecker::ThrowParamException(Error::JsonFormatError,
				                                  "Payload format err: " + std::string(e.what()));
			}

			BigInt feeAmount;
			feeAmount.setDec(fee);

			TransactionPtr tx = wallet->CreateTransaction(Transaction::unregisterCR, payload, utxo, {}, memo, feeAmount);

			nlohmann::json result;
			EncodeTx(result, tx);

			ArgInfo("r => {}", result.dump());
			return result;
		}

		nlohmann::json MainchainSubWallet::CreateRetrieveCRDepositTransaction(
				const nlohmann::json &inputsJson,
				const std::string &amount,
				const std::string &fee,
				const std::string &memo) const {
            WalletPtr wallet = _walletManager->GetWallet();
			ArgInfo("{} {}", wallet->GetWalletID(), GetFunName());
			ArgInfo("inputs: {}", inputsJson.dump());
            ArgInfo("amount: {}", amount);
			ArgInfo("fee: {}", fee);
			ArgInfo("memo: {}", memo);

			UTXOSet utxo;
			UTXOFromJson(utxo, inputsJson);

			BigInt feeAmount, bgAmount;
			feeAmount.setDec(fee);
			bgAmount.setDec(amount);

            OutputArray outputs;
            Address receiveAddr = (*utxo.begin())->GetAddress();
            outputs.push_back(OutputPtr(new TransactionOutput(bgAmount - feeAmount, receiveAddr)));

			PayloadPtr payload = PayloadPtr(new ReturnDepositCoin());
			TransactionPtr tx = wallet->CreateTransaction(Transaction::returnCRDepositCoin, payload, utxo, outputs, memo, feeAmount, true);

			nlohmann::json result;
			EncodeTx(result, tx);
			ArgInfo("r => {}", result.dump());
			return result;
		}

		std::string MainchainSubWallet::CRCouncilMemberClaimNodeDigest(const nlohmann::json &payload) const {
			WalletPtr wallet = _walletManager->GetWallet();
			ArgInfo("{} {}", wallet->GetWalletID(), GetFunName());
			ArgInfo("payload: {}", payload.dump());


			uint8_t version = CRCouncilMemberClaimNodeVersion;
			CRCouncilMemberClaimNode p;
			try {
				p.FromJsonUnsigned(payload, version);
			} catch (const std::exception &e) {
				ErrorChecker::ThrowParamException(Error::InvalidArgument, "from json");
			}

			if (!p.IsValidUnsigned(version)) {
				ErrorChecker::ThrowParamException(Error::InvalidArgument, "invalid payload");
			}

			std::string digest = p.DigestUnsigned(version).GetHex();

			ArgInfo("r => {}", digest);
			return digest;
		}

		nlohmann::json MainchainSubWallet::CreateCRCouncilMemberClaimNodeTransaction(const nlohmann::json &inputsJson,
                                                                                     const nlohmann::json &payloadJson,
                                                                                     const std::string &fee,
                                                                                     const std::string &memo) const {
			WalletPtr wallet = _walletManager->GetWallet();
			ArgInfo("{} {}", wallet->GetWalletID(), GetFunName());
			ArgInfo("inputs: {}", inputsJson.dump());
			ArgInfo("payload: {}", payloadJson.dump());
            ArgInfo("fee: {}", fee);
			ArgInfo("memo: {}", memo);

			UTXOSet utxo;
			UTXOFromJson(utxo, inputsJson);

			uint8_t version = CRCProposalDefaultVersion;
			PayloadPtr payload(new CRCouncilMemberClaimNode());
			try {
				payload->FromJson(payloadJson, version);
			} catch (const std::exception &e) {
				ErrorChecker::ThrowParamException(Error::InvalidArgument, "from json");
			}

			if (!payload->IsValid(version))
				ErrorChecker::ThrowParamException(Error::InvalidArgument, "invalid payload");

			BigInt feeAmount;
			feeAmount.setDec(fee);

			TransactionPtr tx = wallet->CreateTransaction(Transaction::crCouncilMemberClaimNode, payload, utxo, {}, memo, feeAmount);
			tx->SetPayloadVersion(version);

			nlohmann::json result;
			EncodeTx(result, tx);
			ArgInfo("r => {}", result.dump());

			return result;
		}

		std::string MainchainSubWallet::ProposalOwnerDigest(const nlohmann::json &payload) const {
			ArgInfo("{} {}", _walletManager->GetWallet()->GetWalletID(), GetFunName());
			ArgInfo("payload: {}", payload.dump());

			CRCProposal proposal;
			uint8_t version = CRCProposalDefaultVersion;
			try {
				if (payload.contains(JsonKeyDraftData)) {
					version = CRCProposalVersion01;
				} else {
					version = CRCProposalDefaultVersion;
				}
				proposal.FromJsonNormalOwnerUnsigned(payload, version);
			} catch (const nlohmann::json::exception &e) {
				ErrorChecker::ThrowParamException(Error::InvalidArgument, "convert from json");
			}

			ErrorChecker::CheckParam(!proposal.IsValidNormalOwnerUnsigned(version),
									 Error::InvalidArgument, "invalid payload");

			std::string digest = proposal.DigestNormalOwnerUnsigned(version).GetHex();

			ArgInfo("r => {}", digest);
			return digest;
		}

		std::string MainchainSubWallet::ProposalCRCouncilMemberDigest(const nlohmann::json &payload) const {
			ArgInfo("{} {}", _walletManager->GetWallet()->GetWalletID(), GetFunName());
			ArgInfo("payload: {}", payload.dump());

			CRCProposal proposal;
			uint8_t version = CRCProposalDefaultVersion;
			try {
				if (payload.contains(JsonKeyDraftData)) {
					version = CRCProposalVersion01;
				} else {
					version = CRCProposalDefaultVersion;
				}
				proposal.FromJsonNormalCRCouncilMemberUnsigned(payload, version);
			} catch (const nlohmann::json::exception &e) {
				ErrorChecker::ThrowParamException(Error::InvalidArgument, "convert from json");
			}

			ErrorChecker::CheckParam(!proposal.IsValidNormalCRCouncilMemberUnsigned(version),
									 Error::InvalidArgument, "invalid payload");

			std::string digest = proposal.DigestNormalCRCouncilMemberUnsigned(version).GetHex();

			ArgInfo("r => {}", digest);
			return digest;
		}

		std::string MainchainSubWallet::CalculateProposalHash(const nlohmann::json &payload) const {
			ArgInfo("{} {}", _walletManager->GetWallet()->GetWalletID(), GetFunName());
			ArgInfo("payload: {}", payload.dump());

			PayloadPtr p = PayloadPtr(new CRCProposal());
			uint8_t version = CRCProposalDefaultVersion;
			try {
				if (payload.contains(JsonKeyDraftData)) {
					version = CRCProposalVersion01;
				} else {
					version = CRCProposalDefaultVersion;
				}
				p->FromJson(payload, version);
			} catch (const nlohmann::json::exception &e) {
				ErrorChecker::ThrowParamException(Error::InvalidArgument, "convert from json");
			}

			ErrorChecker::CheckParam(!p->IsValid(version), Error::InvalidArgument, "invalid payload");

			ByteStream stream;
			p->Serialize(stream, version);
			uint256 hash(sha256_2(stream.GetBytes()));
			std::string hashString = hash.GetHex();

			ArgInfo("r => {}", hashString);

			return hashString;
		}

        nlohmann::json MainchainSubWallet::CreateProposalTransaction(const nlohmann::json &inputsJson,
                                                                     const nlohmann::json &payload,
                                                                     const std::string &fee,
                                                                     const std::string &memo) const {
			WalletPtr wallet = _walletManager->GetWallet();
			ArgInfo("{} {}", wallet->GetWalletID(), GetFunName());
            ArgInfo("inputs: {}", inputsJson.dump());
			ArgInfo("payload: {}", payload.dump());
            ArgInfo("fee: {}", fee);
			ArgInfo("memo: {}", memo);

			UTXOSet utxo;
			UTXOFromJson(utxo, inputsJson);

			PayloadPtr p = PayloadPtr(new CRCProposal());
			uint8_t version = CRCProposalDefaultVersion;
			try {
				if (payload.contains(JsonKeyDraftData)) {
					version = CRCProposalVersion01;
				} else {
					version = CRCProposalDefaultVersion;
				}
				p->FromJson(payload, version);
			} catch (const nlohmann::json::exception &e) {
				ErrorChecker::ThrowParamException(Error::InvalidArgument, "convert from json");
			}

			ErrorChecker::CheckParam(!p->IsValid(version), Error::InvalidArgument, "invalid payload");

			BigInt feeAmount;
			feeAmount.setDec(fee);

			TransactionPtr tx = wallet->CreateTransaction(Transaction::crcProposal, p, utxo, {}, memo, feeAmount);
			tx->SetPayloadVersion(version);

			nlohmann::json result;
			EncodeTx(result, tx);

			ArgInfo("r => {}", result.dump());
			return result;
		}

		std::string MainchainSubWallet::ProposalReviewDigest(const nlohmann::json &payload) const {
			ArgInfo("{} {}", _walletManager->GetWallet()->GetWalletID(), GetFunName());
			ArgInfo("payload: {}", payload.dump());

			CRCProposalReview proposalReview;
			uint8_t version = CRCProposalReviewDefaultVersion;
			try {
				if (payload.contains(JsonKeyOpinionData)) {
					version = CRCProposalReviewVersion01;
				} else {
					version = CRCProposalReviewDefaultVersion;
				}
				proposalReview.FromJsonUnsigned(payload, version);
			} catch (const nlohmann::json::exception &e) {
				ErrorChecker::ThrowParamException(Error::InvalidArgument, "convert from json");
			}

			if (!proposalReview.IsValidUnsigned(version)) {
				ErrorChecker::ThrowParamException(Error::InvalidArgument, "invalid payload");
			}

			std::string digest = proposalReview.DigestUnsigned(version).GetHex();

			ArgInfo("r => {}", digest);
			return digest;
		}

		nlohmann::json MainchainSubWallet::CreateProposalReviewTransaction(const nlohmann::json &inputsJson,
                                                                           const nlohmann::json &payload,
																		   const std::string &fee,
																		   const std::string &memo) const {
			WalletPtr wallet = _walletManager->GetWallet();
			ArgInfo("{} {}", wallet->GetWalletID(), GetFunName());
			ArgInfo("inputs: {}", inputsJson.dump());
			ArgInfo("payload: {}", payload.dump());
            ArgInfo("fee: {}", fee);
			ArgInfo("memo: {}", memo);

			UTXOSet utxo;
			UTXOFromJson(utxo, inputsJson);

			PayloadPtr p = PayloadPtr(new CRCProposalReview());
			uint8_t version = CRCProposalReviewDefaultVersion;
			try {
				if (payload.contains(JsonKeyOpinionData))
					version = CRCProposalReviewVersion01;
				else
					version = CRCProposalReviewDefaultVersion;
				p->FromJson(payload, version);
			} catch (const nlohmann::json::exception &e) {
				ErrorChecker::ThrowParamException(Error::InvalidArgument, "convert from json");
			}

			if (!p->IsValid(version))
				ErrorChecker::ThrowParamException(Error::InvalidArgument, "invalid payload");

			BigInt feeAmount;
			feeAmount.setDec(fee);

			TransactionPtr tx = wallet->CreateTransaction(Transaction::crcProposalReview, p, utxo, {}, memo, feeAmount);
			tx->SetPayloadVersion(version);

			nlohmann::json result;
			EncodeTx(result, tx);

			ArgInfo("r => {}", result.dump());
			return result;
		}

		std::string MainchainSubWallet::ProposalTrackingOwnerDigest(const nlohmann::json &payload) const {
			ArgInfo("{} {}", _walletManager->GetWallet()->GetWalletID(), GetFunName());
			ArgInfo("payload: {}", payload.dump());

			uint8_t version = CRCProposalTrackingDefaultVersion;
			CRCProposalTracking proposalTracking;
			try {
				if (payload.contains(JsonKeyMessageData))
					version = CRCProposalTrackingVersion01;
				else
					version = CRCProposalTrackingDefaultVersion;
				proposalTracking.FromJsonOwnerUnsigned(payload, version);
			} catch (const nlohmann::json::exception &e) {
				ErrorChecker::ThrowParamException(Error::InvalidArgument, "convert from json");
			}

			if (!proposalTracking.IsValidOwnerUnsigned(version)) {
				ErrorChecker::ThrowParamException(Error::InvalidArgument, "invalid payload");
			}
			std::string digest = proposalTracking.DigestOwnerUnsigned(version).GetHex();

			ArgInfo("r => {}", digest);
			return digest;
		}

		std::string MainchainSubWallet::ProposalTrackingNewOwnerDigest(const nlohmann::json &payload) const {
			ArgInfo("{} {}", _walletManager->GetWallet()->GetWalletID(), GetFunName());
			ArgInfo("payload: {}", payload.dump());

			uint8_t version = CRCProposalTrackingDefaultVersion;
			CRCProposalTracking proposalTracking;
			try {
				if (payload.contains(JsonKeyMessageData))
					version = CRCProposalTrackingVersion01;
				else
					version = CRCProposalTrackingDefaultVersion;
				proposalTracking.FromJsonNewOwnerUnsigned(payload, version);
			} catch (const nlohmann::json::exception &e) {
				ErrorChecker::ThrowParamException(Error::InvalidArgument, "convert from json");
			}

			if (!proposalTracking.IsValidNewOwnerUnsigned(version)) {
				ErrorChecker::ThrowParamException(Error::InvalidArgument, "invalid payload");
			}

			std::string digest = proposalTracking.DigestNewOwnerUnsigned(version).GetHex();

			ArgInfo("r => {}", digest);
			return digest;
		}

		std::string MainchainSubWallet::ProposalTrackingSecretaryDigest(const nlohmann::json &payload) const {
			ArgInfo("{} {}", _walletManager->GetWallet()->GetWalletID(), GetFunName());
			ArgInfo("payload: {}", payload.dump());

			uint8_t version = CRCProposalTrackingDefaultVersion;
			CRCProposalTracking proposalTracking;
			try {
				if (payload.contains(JsonKeyMessageData) && payload.contains(JsonKeySecretaryGeneralOpinionData))
					version = CRCProposalTrackingVersion01;
				else
					version = CRCProposalTrackingDefaultVersion;
				proposalTracking.FromJsonSecretaryUnsigned(payload, version);
			} catch (const nlohmann::json::exception &e) {
				ErrorChecker::ThrowParamException(Error::InvalidArgument, "convert from json");
			}

			if (!proposalTracking.IsValidSecretaryUnsigned(version)) {
				ErrorChecker::ThrowParamException(Error::InvalidArgument, "invalid payload");
			}

			std::string digest = proposalTracking.DigestSecretaryUnsigned(version).GetHex();

			ArgInfo("r => {}", digest);
			return digest;
		}

		nlohmann::json
        MainchainSubWallet::CreateProposalTrackingTransaction(const nlohmann::json &inputsJson,
                                                              const nlohmann::json &payload,
                                                              const std::string &fee,
                                                              const std::string &memo) const {
			WalletPtr wallet = _walletManager->GetWallet();
			ArgInfo("{} {}", wallet->GetWalletID(), GetFunName());
			ArgInfo("inputs: {}", inputsJson.dump());
            ArgInfo("payload: {}", payload.dump());
            ArgInfo("fee: {}", fee);
			ArgInfo("memo: {}", memo);

			UTXOSet utxo;
			UTXOFromJson(utxo, inputsJson);

			uint8_t version = CRCProposalTrackingDefaultVersion;
			PayloadPtr p(new CRCProposalTracking());
			try {
				if (payload.contains(JsonKeyMessageData) && payload.contains(JsonKeySecretaryGeneralOpinionData))
					version = CRCProposalTrackingVersion01;
				else
					version = CRCProposalTrackingDefaultVersion;
				p->FromJson(payload, version);
			} catch (const nlohmann::json::exception &e) {
				ErrorChecker::ThrowParamException(Error::InvalidArgument, "convert from json");
			}

			if (!p->IsValid(version))
				ErrorChecker::ThrowParamException(Error::InvalidArgument, "invalid payload");

			BigInt feeAmount;
			feeAmount.setDec(fee);

			TransactionPtr tx = wallet->CreateTransaction(Transaction::crcProposalTracking, p, utxo, {}, memo, feeAmount);
			tx->SetPayloadVersion(version);

			nlohmann::json result;
			EncodeTx(result, tx);
			ArgInfo("r => {}", result.dump());

			return result;
		}

		std::string MainchainSubWallet::ProposalSecretaryGeneralElectionDigest(
			const nlohmann::json &payload) const {
			ArgInfo("{} {}", _walletManager->GetWallet()->GetWalletID(), GetFunName());
			ArgInfo("payload: {}", payload.dump());

			uint8_t version = CRCProposalDefaultVersion;
			CRCProposal proposal;
			try {
				if (payload.contains(JsonKeyDraftData))
					version = CRCProposalVersion01;
				else
					version = CRCProposalDefaultVersion;
				nlohmann::json payloadFixed = payload;
				payloadFixed[JsonKeyType] = CRCProposal::secretaryGeneralElection;
				proposal.FromJsonSecretaryElectionUnsigned(payloadFixed, version);
			} catch (const nlohmann::json::exception &e) {
				ErrorChecker::ThrowParamException(Error::InvalidArgument, "from json");
			}

			if (!proposal.IsValidSecretaryElectionUnsigned(version)) {
				ErrorChecker::ThrowParamException(Error::InvalidArgument, "invalid payload");
			}

			std::string digest = proposal.DigestSecretaryElectionUnsigned(version).GetHex();

			ArgInfo("r => {}", digest);
			return digest;
		}

		std::string MainchainSubWallet::ProposalSecretaryGeneralElectionCRCouncilMemberDigest(
			const nlohmann::json &payload) const {
			ArgInfo("{} {}", _walletManager->GetWallet()->GetWalletID(), GetFunName());
			ArgInfo("payload: {}", payload.dump());

			uint8_t version = CRCProposalDefaultVersion;
			CRCProposal proposal;
			try {
				if (payload.contains(JsonKeyDraftData))
					version = CRCProposalVersion01;
				else
					version = CRCProposalDefaultVersion;
				nlohmann::json payloadFixed = payload;
				payloadFixed[JsonKeyType] = CRCProposal::secretaryGeneralElection;
				proposal.FromJsonSecretaryElectionCRCouncilMemberUnsigned(payloadFixed, version);
			} catch (const nlohmann::json::exception &e) {
				ErrorChecker::ThrowParamException(Error::InvalidArgument, "from json");
			}

			if (!proposal.IsValidSecretaryElectionCRCouncilMemberUnsigned(version)) {
				ErrorChecker::ThrowParamException(Error::InvalidArgument, "invalid payload");
			}

			std::string digest = proposal.DigestSecretaryElectionCRCouncilMemberUnsigned(version).GetHex();

			ArgInfo("r => {}", digest);
			return digest;
		}

		nlohmann::json MainchainSubWallet::CreateSecretaryGeneralElectionTransaction(
                const nlohmann::json &inputsJson,
                const nlohmann::json &payload,
                const std::string &fee,
                const std::string &memo) const {
			WalletPtr wallet = _walletManager->GetWallet();
			ArgInfo("{} {}", wallet->GetWalletID(), GetFunName());
            ArgInfo("inputs: {}", inputsJson.dump());
			ArgInfo("payload: {}", payload.dump());
			ArgInfo("fee: {}", fee);
            ArgInfo("memo: {}", memo);

            UTXOSet utxo;
            UTXOFromJson(utxo, inputsJson);

			uint8_t version = CRCProposalDefaultVersion;
			PayloadPtr p(new CRCProposal());
			try {
				if (payload.contains(JsonKeyDraftData))
					version = CRCProposalVersion01;
				else
					version = CRCProposalDefaultVersion;
				nlohmann::json payloadFixed = payload;
				payloadFixed[JsonKeyType] = CRCProposal::secretaryGeneralElection;
				p->FromJson(payloadFixed, version);
			} catch (const nlohmann::json::exception &e) {
				ErrorChecker::ThrowParamException(Error::InvalidArgument, "from json");
			}

			if (!p->IsValid(version))
				ErrorChecker::ThrowParamException(Error::InvalidArgument, "invalid payload");

			BigInt feeAmount;
			feeAmount.setDec(fee);

			TransactionPtr tx = wallet->CreateTransaction(Transaction::crcProposal, p, utxo, {}, memo, feeAmount);
			tx->SetPayloadVersion(version);

			nlohmann::json result;
			EncodeTx(result, tx);
			ArgInfo("r => {}", result.dump());

			return result;
		}

		//////////////////////////////////////////////////
		/*             Proposal Change Owner            */
		//////////////////////////////////////////////////
		std::string MainchainSubWallet::ProposalChangeOwnerDigest(const nlohmann::json &payload) const {
			ArgInfo("{} {}", _walletManager->GetWallet()->GetWalletID(), GetFunName());
			ArgInfo("payload: {}", payload.dump());

			uint8_t version = CRCProposalDefaultVersion;
			CRCProposal proposal;
			try {
				if (payload.contains(JsonKeyDraftData))
					version = CRCProposalVersion01;
				else
					version = CRCProposalDefaultVersion;
				nlohmann::json payloadFixed = payload;
				payloadFixed[JsonKeyType] = CRCProposal::changeProposalOwner;
				proposal.FromJsonChangeOwnerUnsigned(payloadFixed, version);
			} catch (const nlohmann::json::exception &e) {
				ErrorChecker::ThrowParamException(Error::InvalidArgument, "from json");
			}

			if (!proposal.IsValidChangeOwnerUnsigned(version)) {
				ErrorChecker::ThrowParamException(Error::InvalidArgument, "invalid payload");
			}

			std::string digest = proposal.DigestChangeOwnerUnsigned(version).GetHex();

			ArgInfo("r => {}", digest);
			return digest;
		}

		std::string MainchainSubWallet::ProposalChangeOwnerCRCouncilMemberDigest(const nlohmann::json &payload) const {
			ArgInfo("{} {}", _walletManager->GetWallet()->GetWalletID(), GetFunName());
			ArgInfo("payload: {}", payload.dump());

			uint8_t version = CRCProposalDefaultVersion;
			CRCProposal proposal;
			try {
				if (payload.contains(JsonKeyDraftData))
					version = CRCProposalVersion01;
				else
					version = CRCProposalDefaultVersion;
				nlohmann::json payloadFixed = payload;
				payloadFixed[JsonKeyType] = CRCProposal::changeProposalOwner;
				proposal.FromJsonChangeOwnerCRCouncilMemberUnsigned(payloadFixed, version);
			} catch (const nlohmann::json::exception &e) {
				ErrorChecker::ThrowParamException(Error::InvalidArgument, "from json");
			}

			if (!proposal.IsValidChangeOwnerCRCouncilMemberUnsigned(version)) {
				ErrorChecker::ThrowParamException(Error::InvalidArgument, "invalid payload");
			}

			std::string digest = proposal.DigestChangeOwnerCRCouncilMemberUnsigned(version).GetHex();

			ArgInfo("r => {}", digest);
			return digest;
		}

		nlohmann::json MainchainSubWallet::CreateProposalChangeOwnerTransaction(
		        const nlohmann::json &inputsJson,
                const nlohmann::json &payload,
                const std::string &fee,
                const std::string &memo) const {
			WalletPtr wallet = _walletManager->GetWallet();
			ArgInfo("{} {}", wallet->GetWalletID(), GetFunName());
			ArgInfo("inputs: {}", inputsJson.dump());
            ArgInfo("payload: {}", payload.dump());
			ArgInfo("fee: {}", fee);
            ArgInfo("memo: {}", memo);

            UTXOSet utxo;
            UTXOFromJson(utxo, inputsJson);

			uint8_t version = CRCProposalDefaultVersion;
			PayloadPtr p(new CRCProposal());
			try {
				if (payload.contains(JsonKeyDraftData))
					version = CRCProposalVersion01;
				else
					version = CRCProposalDefaultVersion;
				nlohmann::json payloadFixed = payload;
				payloadFixed[JsonKeyType] = CRCProposal::changeProposalOwner;
				p->FromJson(payloadFixed, version);
			} catch (const nlohmann::json::exception &e) {
				ErrorChecker::ThrowParamException(Error::InvalidArgument, "from json");
			}

			if (!p->IsValid(version)) {
				ErrorChecker::ThrowParamException(Error::InvalidArgument, "invalid payload");
			}

			BigInt feeAmount;
			feeAmount.setDec(fee);

			TransactionPtr tx = wallet->CreateTransaction(Transaction::crcProposal, p, utxo, {}, memo, feeAmount);
			tx->SetPayloadVersion(version);

			nlohmann::json result;
			EncodeTx(result, tx);
			ArgInfo("r => {}", result.dump());

			return result;
		}

		//////////////////////////////////////////////////
		/*           Proposal Terminate Proposal        */
		//////////////////////////////////////////////////
		std::string MainchainSubWallet::TerminateProposalOwnerDigest(const nlohmann::json &payload) const {
			ArgInfo("{} {}", _walletManager->GetWallet()->GetWalletID(), GetFunName());
			ArgInfo("payload: {}", payload.dump());

			uint8_t version = CRCProposalDefaultVersion;
			CRCProposal proposal;
			try {
				if (payload.contains(JsonKeyDraftData))
					version = CRCProposalVersion01;
				else
					version = CRCProposalDefaultVersion;
				nlohmann::json payloadFixed = payload;
				payloadFixed[JsonKeyType] = CRCProposal::terminateProposal;
				proposal.FromJsonTerminateProposalOwnerUnsigned(payloadFixed, version);
			} catch (const nlohmann::json::exception &e) {
				ErrorChecker::ThrowParamException(Error::InvalidArgument, "from json");
			}

			if (!proposal.IsValidTerminateProposalOwnerUnsigned(version)) {
				ErrorChecker::ThrowParamException(Error::InvalidArgument, "invalid payload");
			}

			std::string digest = proposal.DigestTerminateProposalOwnerUnsigned(version).GetHex();

			ArgInfo("r => {}", digest);
			return digest;
		}

		std::string MainchainSubWallet::TerminateProposalCRCouncilMemberDigest(const nlohmann::json &payload) const {
			ArgInfo("{} {}", _walletManager->GetWallet()->GetWalletID(), GetFunName());
			ArgInfo("payload: {}", payload.dump());

			uint8_t version = CRCProposalDefaultVersion;
			CRCProposal proposal;
			try {
				if (payload.contains(JsonKeyDraftData))
					version = CRCProposalVersion01;
				else
					version = CRCProposalDefaultVersion;
				nlohmann::json payloadFixed = payload;
				payloadFixed[JsonKeyType] = CRCProposal::terminateProposal;
				proposal.FromJsonTerminateProposalCRCouncilMemberUnsigned(payloadFixed, version);
			} catch (const nlohmann::json::exception &e) {
				ErrorChecker::ThrowParamException(Error::InvalidArgument, "from json");
			}

			if (!proposal.IsValidTerminateProposalCRCouncilMemberUnsigned(version)) {
				ErrorChecker::ThrowParamException(Error::InvalidArgument, "invalid payload");
			}

			std::string digest = proposal.DigestTerminateProposalCRCouncilMemberUnsigned(version).GetHex();

			ArgInfo("r => {}", digest);
			return digest;
		}

		nlohmann::json MainchainSubWallet::CreateTerminateProposalTransaction(
                const nlohmann::json &inputsJson,
                const nlohmann::json &payload,
                const std::string &fee,
                const std::string &memo) const {
			WalletPtr wallet = _walletManager->GetWallet();
			ArgInfo("{} {}", wallet->GetWalletID(), GetFunName());
			ArgInfo("inputs: {}", inputsJson.dump());
            ArgInfo("payload: {}", payload.dump());
			ArgInfo("fee: {}", fee);
            ArgInfo("memo: {}", memo);

            UTXOSet utxo;
            UTXOFromJson(utxo, inputsJson);

			uint8_t version = CRCProposalDefaultVersion;
			PayloadPtr p(new CRCProposal());
			try {
				if (payload.contains(JsonKeyDraftData))
					version = CRCProposalVersion01;
				else
					version = CRCProposalDefaultVersion;
				nlohmann::json payloadFixed = payload;
				payloadFixed[JsonKeyType] = CRCProposal::terminateProposal;
				p->FromJson(payloadFixed, version);
			} catch (const nlohmann::json::exception &e) {
				ErrorChecker::ThrowParamException(Error::InvalidArgument, "from json");
			}

			if (!p->IsValid(version))
				ErrorChecker::ThrowParamException(Error::InvalidArgument, "invalid payload");

			BigInt feeAmount;
			feeAmount.setDec(fee);

			TransactionPtr tx = wallet->CreateTransaction(Transaction::crcProposal, p, utxo, {}, memo, feeAmount);
			tx->SetPayloadVersion(version);

			nlohmann::json result;
			EncodeTx(result, tx);
			ArgInfo("r => {}", result.dump());

			return result;
		}

        //////////////////////////////////////////////////
        /*              Reserve Custom ID               */
        //////////////////////////////////////////////////
        std::string MainchainSubWallet::ReserveCustomIDOwnerDigest(const nlohmann::json &payload) const {
            ArgInfo("{} {}", _walletManager->GetWallet()->GetWalletID(), GetFunName());
            ArgInfo("payload: {}", payload.dump());

            uint8_t version = CRCProposalDefaultVersion;
            CRCProposal proposal;
            try {
                if (payload.contains(JsonKeyDraftData))
                    version = CRCProposalVersion01;
                else
                    version = CRCProposalDefaultVersion;
                nlohmann::json payloadFixed = payload;
                payloadFixed[JsonKeyType] = CRCProposal::reserveCustomID;
                proposal.FromJsonReserveCustomIDOwnerUnsigned(payloadFixed, version);
            } catch (const nlohmann::json::exception &e) {
                ErrorChecker::ThrowParamException(Error::InvalidArgument, "from json");
            }

            if (!proposal.IsValidReserveCustomIDOwnerUnsigned(version)) {
                ErrorChecker::ThrowParamException(Error::InvalidArgument, "invalid payload");
            }

            std::string digest = proposal.DigestReserveCustomIDOwnerUnsigned(version).GetHex();

            ArgInfo("r => {}", digest);
            return digest;
        }

        std::string MainchainSubWallet::ReserveCustomIDCRCouncilMemberDigest(const nlohmann::json &payload) const {
            ArgInfo("{} {}", _walletManager->GetWallet()->GetWalletID(), GetFunName());
            ArgInfo("payload: {}", payload.dump());

            uint8_t version = CRCProposalDefaultVersion;
            CRCProposal proposal;
            try {
                if (payload.contains(JsonKeyDraftData))
                    version = CRCProposalVersion01;
                else
                    version = CRCProposalDefaultVersion;
                nlohmann::json payloadFixed = payload;
                payloadFixed[JsonKeyType] = CRCProposal::reserveCustomID;
                proposal.FromJsonReserveCustomIDCRCouncilMemberUnsigned(payloadFixed, version);
            } catch (const nlohmann::json::exception &e) {
                ErrorChecker::ThrowParamException(Error::InvalidArgument, "from json");
            }

            if (!proposal.IsValidReserveCustomIDCRCouncilMemberUnsigned(version)) {
                ErrorChecker::ThrowParamException(Error::InvalidArgument, "invalid payload");
            }

            std::string digest = proposal.DigestReserveCustomIDCRCouncilMemberUnsigned(version).GetHex();

            ArgInfo("r => {}", digest);
            return digest;
        }

        nlohmann::json MainchainSubWallet::CreateReserveCustomIDTransaction(
                const nlohmann::json &inputsJson,
                const nlohmann::json &payload,
                const std::string &fee,
                const std::string &memo) const {
            WalletPtr wallet = _walletManager->GetWallet();
            ArgInfo("{} {}", wallet->GetWalletID(), GetFunName());
            ArgInfo("inputs: {}", inputsJson.dump());
            ArgInfo("payload: {}", payload.dump());
            ArgInfo("fee: {}", fee);
            ArgInfo("memo: {}", memo);

            UTXOSet utxo;
            UTXOFromJson(utxo, inputsJson);

            uint8_t version = CRCProposalDefaultVersion;
            PayloadPtr p(new CRCProposal());
            try {
                if (payload.contains(JsonKeyDraftData))
                    version = CRCProposalVersion01;
                else
                    version = CRCProposalDefaultVersion;
                nlohmann::json payloadFixed = payload;
                payloadFixed[JsonKeyType] = CRCProposal::reserveCustomID;
                p->FromJson(payloadFixed, version);
            } catch (const nlohmann::json::exception &e) {
                ErrorChecker::ThrowParamException(Error::InvalidArgument, "from json");
            }

            if (!p->IsValid(version))
                ErrorChecker::ThrowParamException(Error::InvalidArgument, "invalid payload");

            BigInt feeAmount;
            feeAmount.setDec(fee);

            TransactionPtr tx = wallet->CreateTransaction(Transaction::crcProposal, p, utxo, {}, memo, feeAmount);
            tx->SetPayloadVersion(version);

            nlohmann::json result;
            EncodeTx(result, tx);
            ArgInfo("r => {}", result.dump());

            return result;
        }

        //////////////////////////////////////////////////
        /*               Receive Custom ID              */
        //////////////////////////////////////////////////
        std::string MainchainSubWallet::ReceiveCustomIDOwnerDigest(const nlohmann::json &payload) const {
            ArgInfo("{} {}", _walletManager->GetWallet()->GetWalletID(), GetFunName());
            ArgInfo("payload: {}", payload.dump());

            uint8_t version = CRCProposalDefaultVersion;
            CRCProposal proposal;
            try {
                if (payload.contains(JsonKeyDraftData))
                    version = CRCProposalVersion01;
                else
                    version = CRCProposalDefaultVersion;
                nlohmann::json payloadFixed = payload;
                payloadFixed[JsonKeyType] = CRCProposal::receiveCustomID;
                proposal.FromJsonReceiveCustomIDOwnerUnsigned(payloadFixed, version);
            } catch (const nlohmann::json::exception &e) {
                ErrorChecker::ThrowParamException(Error::InvalidArgument, "from json");
            }

            if (!proposal.IsValidReceiveCustomIDOwnerUnsigned(version)) {
                ErrorChecker::ThrowParamException(Error::InvalidArgument, "invalid payload");
            }

            std::string digest = proposal.DigestReceiveCustomIDOwnerUnsigned(version).GetHex();

            ArgInfo("r => {}", digest);
            return digest;
        }

        std::string MainchainSubWallet::ReceiveCustomIDCRCouncilMemberDigest(const nlohmann::json &payload) const {
            ArgInfo("{} {}", _walletManager->GetWallet()->GetWalletID(), GetFunName());
            ArgInfo("payload: {}", payload.dump());

            uint8_t version = CRCProposalDefaultVersion;
            CRCProposal proposal;
            try {
                if (payload.contains(JsonKeyDraftData))
                    version = CRCProposalVersion01;
                else
                    version = CRCProposalDefaultVersion;
                nlohmann::json payloadFixed = payload;
                payloadFixed[JsonKeyType] = CRCProposal::receiveCustomID;
                proposal.FromJsonReceiveCustomIDCRCouncilMemberUnsigned(payloadFixed, version);
            } catch (const nlohmann::json::exception &e) {
                ErrorChecker::ThrowParamException(Error::InvalidArgument, "from json");
            }

            if (!proposal.IsValidReceiveCustomIDCRCouncilMemberUnsigned(version)) {
                ErrorChecker::ThrowParamException(Error::InvalidArgument, "invalid payload");
            }

            std::string digest = proposal.DigestReceiveCustomIDCRCouncilMemberUnsigned(version).GetHex();

            ArgInfo("r => {}", digest);
            return digest;
        }

        nlohmann::json MainchainSubWallet::CreateReceiveCustomIDTransaction(
                const nlohmann::json &inputsJson,
                const nlohmann::json &payload,
                const std::string &fee,
                const std::string &memo) const {
            WalletPtr wallet = _walletManager->GetWallet();
            ArgInfo("{} {}", wallet->GetWalletID(), GetFunName());
            ArgInfo("inputs: {}", inputsJson.dump());
            ArgInfo("payload: {}", payload.dump());
            ArgInfo("fee: {}", fee);
            ArgInfo("memo: {}", memo);

            UTXOSet utxo;
            UTXOFromJson(utxo, inputsJson);

            uint8_t version = CRCProposalDefaultVersion;
            PayloadPtr p(new CRCProposal());
            try {
                if (payload.contains(JsonKeyDraftData))
                    version = CRCProposalVersion01;
                else
                    version = CRCProposalDefaultVersion;
                nlohmann::json payloadFixed = payload;
                payloadFixed[JsonKeyType] = CRCProposal::receiveCustomID;
                p->FromJson(payloadFixed, version);
            } catch (const nlohmann::json::exception &e) {
                ErrorChecker::ThrowParamException(Error::InvalidArgument, "from json");
            }

            if (!p->IsValid(version))
                ErrorChecker::ThrowParamException(Error::InvalidArgument, "invalid payload");

            BigInt feeAmount;
            feeAmount.setDec(fee);

            TransactionPtr tx = wallet->CreateTransaction(Transaction::crcProposal, p, utxo, {}, memo, feeAmount);
            tx->SetPayloadVersion(version);

            nlohmann::json result;
            EncodeTx(result, tx);
            ArgInfo("r => {}", result.dump());

            return result;
        }

        //////////////////////////////////////////////////
        /*              Change Custom ID Fee            */
        //////////////////////////////////////////////////
        std::string MainchainSubWallet::ChangeCustomIDFeeOwnerDigest(const nlohmann::json &payload) const {
            ArgInfo("{} {}", _walletManager->GetWallet()->GetWalletID(), GetFunName());
            ArgInfo("payload: {}", payload.dump());

            uint8_t version = CRCProposalDefaultVersion;
            CRCProposal proposal;
            try {
                if (payload.contains(JsonKeyDraftData))
                    version = CRCProposalVersion01;
                else
                    version = CRCProposalDefaultVersion;
                nlohmann::json payloadFixed = payload;
                payloadFixed[JsonKeyType] = CRCProposal::changeCustomIDFee;
                proposal.FromJsonChangeCustomIDFeeOwnerUnsigned(payloadFixed, version);
            } catch (const nlohmann::json::exception &e) {
                ErrorChecker::ThrowParamException(Error::InvalidArgument, "from json");
            }

            if (!proposal.IsValidChangeCustomIDFeeOwnerUnsigned(version)) {
                ErrorChecker::ThrowParamException(Error::InvalidArgument, "invalid payload");
            }

            std::string digest = proposal.DigestChangeCustomIDFeeOwnerUnsigned(version).GetHex();

            ArgInfo("r => {}", digest);
            return digest;
        }

        std::string MainchainSubWallet::ChangeCustomIDFeeCRCouncilMemberDigest(const nlohmann::json &payload) const {
            ArgInfo("{} {}", _walletManager->GetWallet()->GetWalletID(), GetFunName());
            ArgInfo("payload: {}", payload.dump());

            uint8_t version = CRCProposalDefaultVersion;
            CRCProposal proposal;
            try {
                if (payload.contains(JsonKeyDraftData))
                    version = CRCProposalVersion01;
                else
                    version = CRCProposalDefaultVersion;
                nlohmann::json payloadFixed = payload;
                payloadFixed[JsonKeyType] = CRCProposal::changeCustomIDFee;
                proposal.FromJsonChangeCustomIDFeeCRCouncilMemberUnsigned(payloadFixed, version);
            } catch (const nlohmann::json::exception &e) {
                ErrorChecker::ThrowParamException(Error::InvalidArgument, "from json");
            }

            if (!proposal.IsValidChangeCustomIDFeeCRCouncilMemberUnsigned(version)) {
                ErrorChecker::ThrowParamException(Error::InvalidArgument, "invalid payload");
            }

            std::string digest = proposal.DigestChangeCustomIDFeeCRCouncilMemberUnsigned(version).GetHex();

            ArgInfo("r => {}", digest);
            return digest;
        }

        nlohmann::json MainchainSubWallet::CreateChangeCustomIDFeeTransaction(
                const nlohmann::json &inputs,
                const nlohmann::json &payload,
                const std::string &fee,
                const std::string &memo) const {
            WalletPtr wallet = _walletManager->GetWallet();
            ArgInfo("{} {}", GetSubWalletID(), GetFunName());
            ArgInfo("inputs: {}", inputs.dump());
            ArgInfo("payload: {}", payload.dump());
            ArgInfo("fee: {}", fee);
            ArgInfo("memo: {}", memo);

            UTXOSet utxo;
            UTXOFromJson(utxo, inputs);

            uint8_t version = CRCProposalDefaultVersion;
            PayloadPtr p(new CRCProposal());
            try {
                if (payload.contains(JsonKeyDraftData))
                    version = CRCProposalVersion01;
                else
                    version = CRCProposalDefaultVersion;
                nlohmann::json payloadFixed = payload;
                payloadFixed[JsonKeyType] = CRCProposal::changeCustomIDFee;
                p->FromJson(payloadFixed, version);
            } catch (const nlohmann::json::exception &e) {
                ErrorChecker::ThrowParamException(Error::InvalidArgument, "from json");
            }

            if (!p->IsValid(version))
                ErrorChecker::ThrowParamException(Error::InvalidArgument, "invalid payload");

            BigInt feeAmount;
            feeAmount.setDec(fee);

            TransactionPtr tx = wallet->CreateTransaction(Transaction::crcProposal, p, utxo, {}, memo, feeAmount);
            tx->SetPayloadVersion(version);

            nlohmann::json result;
            EncodeTx(result, tx);
            ArgInfo("r => {}", result.dump());

            return result;
        }

		std::string MainchainSubWallet::ProposalWithdrawDigest(const nlohmann::json &payload) const {
			ArgInfo("{} {}", _walletManager->GetWallet()->GetWalletID(), GetFunName());
			ArgInfo("payload: {}", payload.dump());

			uint8_t version = CRCProposalWithdrawVersion_01;
			CRCProposalWithdraw proposalWithdraw;
			try {
				proposalWithdraw.FromJsonUnsigned(payload, version);
			} catch (const std::exception &e) {
				ErrorChecker::ThrowParamException(Error::InvalidArgument, "convert from json");
			}

			if (!proposalWithdraw.IsValidUnsigned(version))
				ErrorChecker::ThrowParamException(Error::InvalidArgument, "invalid payload");

			std::string digest = proposalWithdraw.DigestUnsigned(version).GetHex();

			ArgInfo("r => {}", digest);
			return digest;
		}

		nlohmann::json MainchainSubWallet::CreateProposalWithdrawTransaction(
		        const nlohmann::json &inputsJson,
                const nlohmann::json &payload,
                const std::string &fee,
                const std::string &memo) const {
			WalletPtr wallet = _walletManager->GetWallet();
			ArgInfo("{} {}", wallet->GetWalletID(), GetFunName());
			ArgInfo("inputs: {}", inputsJson.dump());
            ArgInfo("payload: {}", payload.dump());
			ArgInfo("fee: {}", fee);
            ArgInfo("memo: {}", memo);

            UTXOSet utxo;
            UTXOFromJson(utxo, inputsJson);

			uint8_t version = CRCProposalWithdrawVersion_01;
			PayloadPtr p(new CRCProposalWithdraw());
			try {
				p->FromJson(payload, version);
			} catch (const std::exception &e) {
				ErrorChecker::ThrowParamException(Error::InvalidArgument, "from json");
			}

			if (!p->IsValid(version))
				ErrorChecker::ThrowParamException(Error::InvalidArgument, "invalid payload");

			BigInt feeAmount;
			feeAmount.setDec(fee);

			TransactionPtr tx = wallet->CreateTransaction(Transaction::crcProposalWithdraw, p, utxo, {}, memo, feeAmount);
			tx->SetPayloadVersion(version);

			nlohmann::json result;
			EncodeTx(result, tx);
			ArgInfo("r => {}", result.dump());

			return result;
		}

        std::string MainchainSubWallet::RegisterSidechainOwnerDigest(const nlohmann::json &payload) const {
            ArgInfo("{} {}", _walletManager->GetWallet()->GetWalletID(), GetFunName());
            ArgInfo("payload: {}", payload.dump());

            uint8_t version = CRCProposalDefaultVersion;
            CRCProposal proposal;
            try {
                if (payload.contains(JsonKeyDraftData))
                    version = CRCProposalVersion01;
                else
                    version = CRCProposalDefaultVersion;
                nlohmann::json payloadFixed = payload;
                payloadFixed[JsonKeyType] = CRCProposal::registerSideChain;
                proposal.FromJsonRegisterSidechainUnsigned(payloadFixed, version);
            } catch (const nlohmann::json::exception &e) {
                ErrorChecker::ThrowParamException(Error::InvalidArgument, "from json");
            }

            if (!proposal.IsValidRegisterSidechainUnsigned(version)) {
                ErrorChecker::ThrowParamException(Error::InvalidArgument, "invalid payload");
            }

            std::string digest = proposal.DigestRegisterSidechainUnsigned(version).GetHex();

            ArgInfo("r => {}", digest);
            return digest;
        }

        std::string MainchainSubWallet::RegisterSidechainCRCouncilMemberDigest(const nlohmann::json &payload) const {
            ArgInfo("{} {}", _walletManager->GetWallet()->GetWalletID(), GetFunName());
            ArgInfo("payload: {}", payload.dump());

            uint8_t version = CRCProposalDefaultVersion;
            CRCProposal proposal;
            try {
                if (payload.contains(JsonKeyDraftData))
                    version = CRCProposalVersion01;
                else
                    version = CRCProposalDefaultVersion;
                nlohmann::json payloadFixed = payload;
                payloadFixed[JsonKeyType] = CRCProposal::registerSideChain;
                proposal.FromJsonRegisterSidechainCRCouncilMemberUnsigned(payloadFixed, version);
            } catch (const nlohmann::json::exception &e) {
                ErrorChecker::ThrowParamException(Error::InvalidArgument, "from json");
            }

            if (!proposal.IsValidRegisterSidechainCRCouncilMemberUnsigned(version)) {
                ErrorChecker::ThrowParamException(Error::InvalidArgument, "invalid payload");
            }

            std::string digest = proposal.DigestRegisterSidechainCRCouncilMemberUnsigned(version).GetHex();

            ArgInfo("r => {}", digest);
            return digest;
        }

        nlohmann::json MainchainSubWallet::CreateRegisterSidechainTransaction(
                const nlohmann::json &inputs,
                const nlohmann::json &payload,
                const std::string &fee,
                const std::string &memo) const {
            WalletPtr wallet = _walletManager->GetWallet();
            ArgInfo("{} {}", GetSubWalletID(), GetFunName());
            ArgInfo("inputs: {}", inputs.dump());
            ArgInfo("payload: {}", payload.dump());
            ArgInfo("fee: {}", fee);
            ArgInfo("memo: {}", memo);

            UTXOSet utxo;
            UTXOFromJson(utxo, inputs);

            uint8_t version = CRCProposalDefaultVersion;
            PayloadPtr p(new CRCProposal());
            try {
                if (payload.contains(JsonKeyDraftData))
                    version = CRCProposalVersion01;
                else
                    version = CRCProposalDefaultVersion;
                nlohmann::json payloadFixed = payload;
                payloadFixed[JsonKeyType] = CRCProposal::registerSideChain;
                p->FromJson(payloadFixed, version);
            } catch (const nlohmann::json::exception &e) {
                ErrorChecker::ThrowParamException(Error::InvalidArgument, "from json");
            }

            if (!p->IsValid(version))
                ErrorChecker::ThrowParamException(Error::InvalidArgument, "invalid payload");

            BigInt feeAmount;
            feeAmount.setDec(fee);

            TransactionPtr tx = wallet->CreateTransaction(Transaction::crcProposal, p, utxo, {}, memo, feeAmount);
            tx->SetPayloadVersion(version);

            nlohmann::json result;
            EncodeTx(result, tx);
            ArgInfo("r => {}", result.dump());

            return result;
        }

	}
}
