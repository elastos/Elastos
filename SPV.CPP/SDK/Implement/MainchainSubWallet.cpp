// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "MainchainSubWallet.h"
#include "MasterWallet.h"

#include <Common/Utils.h>
#include <Common/ErrorChecker.h>
#include <WalletCore/Key.h>
#include <WalletCore/CoinInfo.h>
#include <Wallet/UTXO.h>
#include <Plugin/Transaction/Asset.h>
#include <Plugin/Transaction/Payload/TransferCrossChainAsset.h>
#include <Plugin/Transaction/Payload/ProducerInfo.h>
#include <Plugin/Transaction/Payload/CancelProducer.h>
#include <Plugin/Transaction/Payload/OutputPayload/PayloadVote.h>
#include <Plugin/Transaction/Payload/CRInfo.h>
#include <Plugin/Transaction/Payload/UnregisterCR.h>
#include <Plugin/Transaction/Payload/CRCProposal.h>
#include <Plugin/Transaction/Payload/CRCProposalReview.h>
#include <Plugin/Transaction/TransactionInput.h>
#include <Plugin/Transaction/TransactionOutput.h>
#include <SpvService/Config.h>
#include <Plugin/Transaction/Payload/ReturnDepositCoin.h>
#include <CMakeConfig.h>

#include <vector>
#include <map>
#include <boost/scoped_ptr.hpp>

namespace Elastos {
	namespace ElaWallet {

#define DEPOSIT_MIN_ELA 5000

		MainchainSubWallet::MainchainSubWallet(const CoinInfoPtr &info,
											   const ChainConfigPtr &config,
											   MasterWallet *parent) :
				SubWallet(info, config, parent) {
		}

		MainchainSubWallet::~MainchainSubWallet() {

		}

		nlohmann::json MainchainSubWallet::CreateDepositTransaction(const std::string &fromAddress,
																	const std::string &lockedAddress,
																	const std::string &amount,
																	const std::string &sideChainAddress,
																	const std::string &memo) {
			ArgInfo("{} {}", _walletManager->GetWallet()->GetWalletID(), GetFunName());
			ArgInfo("fromAddr: {}", fromAddress);
			ArgInfo("lockedAddr: {}", lockedAddress);
			ArgInfo("amount: {}", amount);
			ArgInfo("sideChainAddr: {}", sideChainAddress);
			ArgInfo("memo: {}", memo);
			BigInt value;
			value.setDec(amount);

			PayloadPtr payload = nullptr;
			try {
				TransferInfo info(sideChainAddress, 0, value);
				payload = PayloadPtr(new TransferCrossChainAsset({info}));
			} catch (const nlohmann::detail::exception &e) {
				ErrorChecker::ThrowParamException(Error::JsonFormatError,
												  "Side chain message error: " + std::string(e.what()));
			}

			std::vector<OutputPtr> outputs;
			Address receiveAddr(lockedAddress);
			outputs.emplace_back(OutputPtr(new TransactionOutput(value + _config->MinFee(), receiveAddr)));

			TransactionPtr tx = CreateTx(Transaction::transferCrossChainAsset, payload, fromAddress, outputs, memo);

			nlohmann::json result;
			EncodeTx(result, tx);

			ArgInfo("r => {}", result.dump());
			return result;
		}

		TransactionPtr MainchainSubWallet::CreateVoteTx(const VoteContent &voteContent, const std::string &memo, bool max) {
			std::string m;

			if (!memo.empty())
				m = "type:text,msg:" + memo;

			TransactionPtr tx = _walletManager->GetWallet()->Vote(voteContent, m, max);

			if (_info->GetChainID() == "ELA")
				tx->SetVersion(Transaction::TxVersion::V09);

			tx->FixIndex();

			return tx;
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
			verifyPubKey.SetPubKey(ownerPubKey);

			bytes_t nodePubKey = bytes_t(nodePublicKey);
			verifyPubKey.SetPubKey(nodePubKey);

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
			const std::string &fromAddress,
			const nlohmann::json &payloadJson,
			const std::string &amount,
			const std::string &memo) {

			ArgInfo("{} {}", _walletManager->GetWallet()->GetWalletID(), GetFunName());
			ArgInfo("fromAddr: {}", fromAddress);
			ArgInfo("payload: {}", payloadJson.dump());
			ArgInfo("amount: {}", amount);
			ArgInfo("memo: {}", memo);

			BigInt bgAmount, minAmount(DEPOSIT_MIN_ELA);
			bgAmount.setDec(amount);

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
			std::string toAddress = Address(PrefixDeposit, pubkey).String();

			std::vector<OutputPtr> outputs;
			Address receiveAddr(toAddress);
			outputs.push_back(OutputPtr(new TransactionOutput(bgAmount, receiveAddr)));

			TransactionPtr tx = CreateTx(Transaction::registerProducer, payload, fromAddress, outputs, memo);

			nlohmann::json result;
			EncodeTx(result, tx);

			ArgInfo("r => {}", result.dump());
			return result;
		}

		nlohmann::json MainchainSubWallet::CreateUpdateProducerTransaction(
			const std::string &fromAddress,
			const nlohmann::json &payloadJson,
			const std::string &memo) {

			ArgInfo("{} {}", _walletManager->GetWallet()->GetWalletID(), GetFunName());
			ArgInfo("fromAddr: {}", fromAddress);
			ArgInfo("payload: {}", payloadJson.dump());
			ArgInfo("memo: {}", memo);

			PayloadPtr payload = PayloadPtr(new ProducerInfo());
			try {
				payload->FromJson(payloadJson, 0);
			} catch (const nlohmann::detail::exception &e) {
				ErrorChecker::ThrowParamException(Error::JsonFormatError,
												  "Payload format err: " + std::string(e.what()));
			}

			std::vector<OutputPtr> outputs;
			Address receiveAddr(CreateAddress());
			outputs.push_back(OutputPtr(new TransactionOutput(BigInt(0), receiveAddr)));

			TransactionPtr tx = CreateTx(Transaction::updateProducer, payload, fromAddress, outputs, memo);

			if (tx->GetOutputs().size() > 1) {
				tx->RemoveOutput(tx->GetOutputs().front());
				tx->FixIndex();
			}

			nlohmann::json result;
			EncodeTx(result, tx);

			ArgInfo("r => {}", result.dump());
			return result;
		}

		nlohmann::json MainchainSubWallet::CreateCancelProducerTransaction(
			const std::string &fromAddress,
			const nlohmann::json &payloadJson,
			const std::string &memo) {

			ArgInfo("{} {}", _walletManager->GetWallet()->GetWalletID(), GetFunName());
			ArgInfo("fromAddr: {}", fromAddress);
			ArgInfo("payload: {}", payloadJson.dump());
			ArgInfo("memo: {}", memo);

			PayloadPtr payload = PayloadPtr(new CancelProducer());
			try {
				payload->FromJson(payloadJson, 0);
			} catch (const nlohmann::detail::exception &e) {
				ErrorChecker::ThrowParamException(Error::JsonFormatError,
												  "Payload format err: " + std::string(e.what()));
			}

			std::vector<OutputPtr> outputs;
			Address receiveAddr(CreateAddress());
			outputs.push_back(OutputPtr(new TransactionOutput(BigInt(0), receiveAddr)));

			TransactionPtr tx = CreateTx(Transaction::cancelProducer, payload, fromAddress, outputs, memo);

			if (tx->GetOutputs().size() > 1) {
				tx->RemoveOutput(tx->GetOutputs().front());
				tx->FixIndex();
			}

			nlohmann::json result;
			EncodeTx(result, tx);

			ArgInfo("r => {}", result.dump());
			return result;
		}

		nlohmann::json MainchainSubWallet::CreateRetrieveDepositTransaction(
			const std::string &amount,
			const std::string &memo) {

			ArgInfo("{} {}", _walletManager->GetWallet()->GetWalletID(), GetFunName());
			ArgInfo("amount: {}", amount);
			ArgInfo("memo: {}", memo);

			BigInt bgAmount;
			bgAmount.setDec(amount);

			std::string fromAddress = _walletManager->GetWallet()->GetOwnerDepositAddress().String();

			std::vector<OutputPtr> outputs;
			Address receiveAddr(CreateAddress());
			outputs.push_back(OutputPtr(new TransactionOutput(bgAmount, receiveAddr)));

			PayloadPtr payload = PayloadPtr(new ReturnDepositCoin());
			TransactionPtr tx = CreateTx(Transaction::returnDepositCoin, payload, fromAddress, outputs, memo);

			if (tx->GetOutputs().size() > 1) {
				tx->RemoveOutput(tx->GetOutputs().back());
				tx->FixIndex();
			}

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

			std::string address = _walletManager->GetWallet()->GetOwnerAddress().String();

			ArgInfo("r => {}", address);

			return address;
		}

		nlohmann::json MainchainSubWallet::CreateVoteProducerTransaction(
			const std::string &fromAddress,
			const std::string &stake,
			const nlohmann::json &publicKeys,
			const std::string &memo) {

			ArgInfo("{} {}", _walletManager->GetWallet()->GetWalletID(), GetFunName());
			ArgInfo("fromAddr: {}", fromAddress);
			ArgInfo("stake: {}", stake);
			ArgInfo("pubkeys: {}", publicKeys.dump());
			ArgInfo("memo: {}", memo);

			bool max = false;
			BigInt bgStake;
			if (stake == "-1") {
				max = true;
				bgStake = 0;
			} else {
				bgStake.setDec(stake);
			}

			ErrorChecker::CheckJsonArray(publicKeys, 1, "Candidates public keys");
			// -1 means max
			ErrorChecker::CheckParam(bgStake <= 0 && !max, Error::Code::VoteStakeError, "Vote stake should not be zero");

			VoteContent voteContent(VoteContent::Delegate);
			for (nlohmann::json::const_iterator it = publicKeys.cbegin(); it != publicKeys.cend(); ++it) {
				if (!(*it).is_string()) {
					ErrorChecker::ThrowParamException(Error::Code::JsonFormatError,
													  "Vote produce public keys is not string");
				}
				// Check public key is valid later
				voteContent.AddCandidate(CandidateVotes((*it).get<std::string>(), bgStake));
			}

			ErrorChecker::CheckParam(voteContent.GetCandidateVotes().empty(), Error::InvalidArgument,
									 "Candidate vote list should not be empty");

			TransactionPtr tx = CreateVoteTx(voteContent, memo, max);

			nlohmann::json result;
			EncodeTx(result, tx);

			ArgInfo("r => {}", result.dump());
			return result;
		}

		nlohmann::json MainchainSubWallet::GetVotedProducerList() const {
			ArgInfo("{} {}", _walletManager->GetWallet()->GetWalletID(), GetFunName());

			WalletPtr wallet = _walletManager->GetWallet();
			UTXOArray utxos = wallet->GetVoteUTXO();
			nlohmann::json j;
			std::map<std::string, BigInt> votedList;

			for (size_t i = 0; i < utxos.size(); ++i) {
				const OutputPtr &output = utxos[i]->Output();
				if (output->GetType() != TransactionOutput::VoteOutput) {
					continue;
				}

				const PayloadVote *pv = dynamic_cast<const PayloadVote *>(output->GetPayload().get());
				if (pv == nullptr) {
					continue;
				}

				BigInt stake = output->Amount();
				uint8_t version = pv->Version();
				const std::vector<VoteContent> &voteContents = pv->GetVoteContent();
				std::for_each(voteContents.cbegin(), voteContents.cend(),
							  [&votedList, &stake, &version](const VoteContent &vc) {
								  if (vc.GetType() == VoteContent::Type::Delegate) {
									  std::for_each(vc.GetCandidateVotes().cbegin(), vc.GetCandidateVotes().cend(),
													[&votedList, &stake, &version](const CandidateVotes &cvs) {
														std::string c = cvs.GetCandidate().getHex();
														BigInt votes;

														if (version == VOTE_PRODUCER_CR_VERSION)
															votes = cvs.GetVotes();
														else
															votes = stake;

														if (votedList.find(c) != votedList.end()) {
															votedList[c] += votes;
														} else {
															votedList[c] = votes;
														}
													});
								  }
							  });

			}

			for (std::map<std::string, BigInt>::iterator it = votedList.begin(); it != votedList.end(); ++it)
				j[(*it).first] = (*it).second.getDec();

			ArgInfo("r => {}", j.dump());

			return j;
		}

		nlohmann::json MainchainSubWallet::GetRegisteredProducerInfo() const {
			ArgInfo("{} {}", _walletManager->GetWallet()->GetWalletID(), GetFunName());

			std::vector<TransactionPtr> allTxs = _walletManager->GetWallet()->GetAllTransactions();
			nlohmann::json j;

			j["Status"] = "Unregistered";
			j["Info"] = nlohmann::json();
			for (size_t i = 0; i < allTxs.size(); ++i) {
				if (allTxs[i]->GetBlockHeight() == TX_UNCONFIRMED) {
					continue;
				}

				if (allTxs[i]->GetTransactionType() == Transaction::registerProducer ||
				    allTxs[i]->GetTransactionType() == Transaction::updateProducer) {
					const ProducerInfo *pinfo = dynamic_cast<const ProducerInfo *>(allTxs[i]->GetPayload());
					if (pinfo) {
						nlohmann::json info;

						info["OwnerPublicKey"] = pinfo->GetPublicKey().getHex();
						info["NodePublicKey"] = pinfo->GetNodePublicKey().getHex();
						info["NickName"] = pinfo->GetNickName();
						info["URL"] = pinfo->GetUrl();
						info["Location"] = pinfo->GetLocation();
						info["Address"] = pinfo->GetAddress();

						j["Status"] = "Registered";
						j["Info"] = info;
					}
				} else if (allTxs[i]->GetTransactionType() == Transaction::cancelProducer) {
					const CancelProducer *pc = dynamic_cast<const CancelProducer *>(allTxs[i]->GetPayload());
					if (pc) {
						uint32_t lastBlockHeight = _walletManager->GetWallet()->LastBlockHeight();

						nlohmann::json info;

						info["Confirms"] = allTxs[i]->GetConfirms(lastBlockHeight);

						j["Status"] = "Canceled";
						j["Info"] = info;
					}
				} else if (allTxs[i]->GetTransactionType() == Transaction::returnDepositCoin) {
					j["Status"] = "ReturnDeposit";
					j["Info"] = nlohmann::json();
				}
			}

			ArgInfo("r => {}", j.dump());
			return j;
		}

		std::string MainchainSubWallet::GetCROwnerDID() const {
			ArgInfo("{} {}", _walletManager->GetWallet()->GetWalletID(), GetFunName());
			bytes_t pubKey = _walletManager->GetWallet()->GetCROwnerPublicKey();
			std::string addr = Address(PrefixIDChain, pubKey).String();

			ArgInfo("r => {}", addr);
			return addr;
		}

		std::string MainchainSubWallet::GetCROwnerPublicKey() const {
			ArgInfo("{} {}", _walletManager->GetWallet()->GetWalletID(), GetFunName());
			std::string pubkey = _walletManager->GetWallet()->GetCROwnerPublicKey().getHex();
			ArgInfo("r => {}", pubkey);
			return pubkey;
		}

		nlohmann::json MainchainSubWallet::GenerateCRInfoPayload(
				const std::string &crPublicKey,
				const std::string &nickName,
				const std::string &url,
				uint64_t location,
				const std::string &payPasswd) const {
			ArgInfo("{} {}", _walletManager->GetWallet()->GetWalletID(), GetFunName());
			ArgInfo("crPublicKey: {}", crPublicKey);
			ArgInfo("nickName: {}", nickName);
			ArgInfo("url: {}", url);
			ArgInfo("location: {}", location);
			ArgInfo("payPasswd: *");

			ErrorChecker::CheckPassword(payPasswd, "Generate payload");
			size_t pubKeyLen = crPublicKey.size() >> 1;
			ErrorChecker::CheckParam(pubKeyLen != 33 && pubKeyLen != 65, Error::PubKeyLength,
			                         "Public key length should be 33 or 65 bytes");

			bytes_t pubkey(crPublicKey);

			Address address(PrefixStandard, pubkey);

			CRInfo crInfo;
			crInfo.SetCode(address.RedeemScript());
			crInfo.SetNickName(nickName);
			crInfo.SetUrl(url);
			crInfo.SetLocation(location);

			Address did;
			did.SetRedeemScript(PrefixIDChain, crInfo.GetCode());
			crInfo.SetDID(did.ProgramHash());

			ByteStream ostream;
			crInfo.SerializeUnsigned(ostream, 0);
			bytes_t prUnsigned = ostream.GetBytes();

			crInfo.SetSignature(_walletManager->GetWallet()->SignWithCROwnerKey(prUnsigned, payPasswd));

			nlohmann::json payloadJson = crInfo.ToJson(0);

			ArgInfo("r => {}", payloadJson.dump());
			return payloadJson;
		}

		nlohmann::json MainchainSubWallet::GenerateUnregisterCRPayload(
				const std::string &crDID,
				const std::string &payPasswd) const {
			ArgInfo("{} {}", _walletManager->GetWallet()->GetWalletID(), GetFunName());
			ArgInfo("crDID: {}", crDID);
			ArgInfo("payPasswd: *");

			ErrorChecker::CheckPassword(payPasswd, "Generate payload");
			Address address(crDID);
			ErrorChecker::CheckParam(!address.Valid(), Error::InvalidArgument, "invalid crDID");

			UnregisterCR unregisterCR;
			unregisterCR.SetDID(address.ProgramHash());

			ByteStream ostream;
			unregisterCR.SerializeUnsigned(ostream, 0);
			bytes_t prUnsigned = ostream.GetBytes();

			unregisterCR.SetSignature(_walletManager->GetWallet()->SignWithCROwnerKey(prUnsigned, payPasswd));

			nlohmann::json payloadJson = unregisterCR.ToJson(0);

			ArgInfo("r => {}", payloadJson.dump());
			return payloadJson;
		}

		nlohmann::json MainchainSubWallet::CreateRegisterCRTransaction(
				const std::string &fromAddress,
				const nlohmann::json &payloadJSON,
				const std::string &amount,
				const std::string &memo) {

			ArgInfo("{} {}", _walletManager->GetWallet()->GetWalletID(), GetFunName());
			ArgInfo("fromAddr: {}", fromAddress);
			ArgInfo("payload: {}", payloadJSON.dump());
			ArgInfo("amount: {}", amount);
			ArgInfo("memo: {}", memo);

			BigInt bgAmount, minAmount(DEPOSIT_MIN_ELA);
			bgAmount.setDec(amount);

			minAmount *= SELA_PER_ELA;

			ErrorChecker::CheckParam(bgAmount < minAmount, Error::DepositAmountInsufficient,
			                         "cr deposit amount is insufficient");

			PayloadPtr payload = PayloadPtr(new CRInfo());
			try {
				payload->FromJson(payloadJSON, 0);
			} catch (const nlohmann::detail::exception &e) {
				ErrorChecker::ThrowParamException(Error::JsonFormatError,
				                                  "Payload format err: " + std::string(e.what()));
			}

			bytes_t code = static_cast<CRInfo *>(payload.get())->GetCode();
			Address receiveAddr;
			receiveAddr.SetRedeemScript(PrefixDeposit, code);

			std::vector<OutputPtr> outputs;
			outputs.push_back(OutputPtr(new TransactionOutput(bgAmount, receiveAddr)));

			TransactionPtr tx = CreateTx(Transaction::registerCR, payload, fromAddress, outputs, memo);

			nlohmann::json result;
			EncodeTx(result, tx);

			ArgInfo("r => {}", result.dump());
			return result;
		}

		nlohmann::json MainchainSubWallet::CreateUpdateCRTransaction(
				const std::string &fromAddress,
				const nlohmann::json &payloadJSON,
				const std::string &memo) {
			ArgInfo("{} {}", _walletManager->GetWallet()->GetWalletID(), GetFunName());
			ArgInfo("fromAddr: {}", fromAddress);
			ArgInfo("payload: {}", payloadJSON.dump());
			ArgInfo("memo: {}", memo);

			PayloadPtr payload = PayloadPtr(new CRInfo());
			try {
				payload->FromJson(payloadJSON, 0);
			} catch (const nlohmann::detail::exception &e) {
				ErrorChecker::ThrowParamException(Error::JsonFormatError,
				                                  "Payload format err: " + std::string(e.what()));
			}

			std::vector<OutputPtr> outputs;
			Address receiveAddr(CreateAddress());
			outputs.push_back(OutputPtr(new TransactionOutput(BigInt(0), receiveAddr)));

			TransactionPtr tx = CreateTx(Transaction::updateCR, payload, fromAddress, outputs, memo);

			if (tx->GetOutputs().size() > 1) {
				tx->RemoveOutput(tx->GetOutputs().front());
				tx->FixIndex();
			}

			nlohmann::json result;
			EncodeTx(result, tx);

			ArgInfo("r => {}", result.dump());
			return result;

		}

		nlohmann::json MainchainSubWallet::CreateUnregisterCRTransaction(
				const std::string &fromAddress,
				const nlohmann::json &payloadJSON,
				const std::string &memo) {
			ArgInfo("{} {}", _walletManager->GetWallet()->GetWalletID(), GetFunName());
			ArgInfo("fromAddr: {}", fromAddress);
			ArgInfo("payload: {}", payloadJSON.dump());
			ArgInfo("memo: {}", memo);

			PayloadPtr payload = PayloadPtr(new UnregisterCR());
			try {
				payload->FromJson(payloadJSON, 0);
			} catch (const nlohmann::detail::exception &e) {
				ErrorChecker::ThrowParamException(Error::JsonFormatError,
				                                  "Payload format err: " + std::string(e.what()));
			}

			std::vector<OutputPtr> outputs;
			Address receiveAddr(CreateAddress());
			outputs.push_back(OutputPtr(new TransactionOutput(BigInt(0), receiveAddr)));

			TransactionPtr tx = CreateTx(Transaction::unregisterCR, payload, fromAddress, outputs, memo);

			if (tx->GetOutputs().size() > 1) {
				tx->RemoveOutput(tx->GetOutputs().front());
				tx->FixIndex();
			}

			nlohmann::json result;
			EncodeTx(result, tx);

			ArgInfo("r => {}", result.dump());
			return result;
		}

		nlohmann::json MainchainSubWallet::CreateRetrieveCRDepositTransaction(
				const std::string &amount,
				const std::string &memo) {
			ArgInfo("{} {}", _walletManager->GetWallet()->GetWalletID(), GetFunName());
			ArgInfo("amount: {}", amount);
			ArgInfo("memo: {}", memo);

			BigInt bgAmount;
			bgAmount.setDec(amount);

			Address fromAddress = Address(PrefixDeposit, _walletManager->GetWallet()->GetCROwnerPublicKey());

			std::vector<OutputPtr> outputs;
			Address receiveAddr(CreateAddress());
			outputs.push_back(OutputPtr(new TransactionOutput(bgAmount, receiveAddr)));

			PayloadPtr payload = PayloadPtr(new ReturnDepositCoin());
			TransactionPtr tx = CreateTx(Transaction::returnCRDepositCoin, payload, fromAddress.String(), outputs, memo);

			if (tx->GetOutputs().size() > 1) {
				tx->RemoveOutput(tx->GetOutputs().back());
				tx->FixIndex();
			}

			nlohmann::json result;
			EncodeTx(result, tx);
			ArgInfo("r => {}", result.dump());
			return result;
		}

		nlohmann::json MainchainSubWallet::CreateVoteCRTransaction(
				const std::string &fromAddress,
				const nlohmann::json &votes,
				const std::string &memo) {
			ArgInfo("{} {}", _walletManager->GetWallet()->GetWalletID(), GetFunName());
			ArgInfo("fromAddr: {}", fromAddress);
			ArgInfo("votes: {}", votes.dump());
			ArgInfo("memo: {}", memo);

			ErrorChecker::CheckParam(!votes.is_object(), Error::Code::JsonFormatError, "votes is error json format");

			BigInt bgStake = 0;

			VoteContent voteContent(VoteContent::CRC);
			std::vector<CandidateVotes> candidates;
			std::string key;
			bytes_t candidate;
			BigInt value;
			for (nlohmann::json::const_iterator it = votes.cbegin(); it != votes.cend(); ++it) {
				ErrorChecker::CheckParam(!it.value().is_string(), Error::InvalidArgument, "stake value should be big int string");

				key = it.key();
				Address didAddress(key);
				ErrorChecker::CheckParam(!didAddress.Valid(), Error::InvalidArgument, "invalid candidate did");
				candidate = didAddress.ProgramHash().bytes();

				value.setDec(it.value().get<std::string>());
				ErrorChecker::CheckParam(value <= 0, Error::InvalidArgument, "stake value should larger than 0");

				voteContent.AddCandidate(CandidateVotes(candidate, value));
			}

			TransactionPtr tx = CreateVoteTx(voteContent, memo, false);

			nlohmann::json result;
			EncodeTx(result, tx);

			ArgInfo("r => {}", result.dump());

			return result;
		}

		nlohmann::json MainchainSubWallet::GetVotedCRList() const {
			ArgInfo("{} {}", _walletManager->GetWallet()->GetWalletID(), GetFunName());

			WalletPtr wallet = _walletManager->GetWallet();
			UTXOArray utxos = wallet->GetVoteUTXO();
			nlohmann::json j;
			std::map<std::string, BigInt> votedList;

			for (size_t i = 0; i < utxos.size(); ++i) {
				const OutputPtr &output = utxos[i]->Output();
				if (output->GetType() != TransactionOutput::VoteOutput) {
					continue;
				}

				const PayloadVote *pv = dynamic_cast<const PayloadVote *>(output->GetPayload().get());
				if (pv == nullptr) {
					continue;
				}

				const std::vector<VoteContent> &voteContents = pv->GetVoteContent();
				std::for_each(voteContents.cbegin(), voteContents.cend(),
				              [&votedList](const VoteContent &vc) {
					              if (vc.GetType() == VoteContent::Type::CRC) {
						              std::for_each(vc.GetCandidateVotes().cbegin(), vc.GetCandidateVotes().cend(),
						                            [&votedList](const CandidateVotes &cv) {
							                            std::string did = Address(uint168(cv.GetCandidate())).String();
							                            if (votedList.find(did) != votedList.end()) {
								                            votedList[did] += cv.GetVotes();
							                            } else {
								                            votedList[did] = cv.GetVotes();
							                            }
						                            });
					              }
				              });

			}

			for (std::map<std::string, BigInt>::iterator it = votedList.begin(); it != votedList.end(); ++it)
				j[(*it).first] = (*it).second.getDec();

			ArgInfo("r => {}", j.dump());

			return j;
		}

		nlohmann::json MainchainSubWallet::GetRegisteredCRInfo() const {
			ArgInfo("{} {}", _walletManager->GetWallet()->GetWalletID(), GetFunName());

			std::vector<TransactionPtr> allTxs = _walletManager->GetWallet()->GetAllTransactions();
			nlohmann::json j;

			j["Status"] = "Unregistered";
			j["Info"] = nlohmann::json();
			for (size_t i = 0; i < allTxs.size(); ++i) {
				if (allTxs[i]->GetBlockHeight() == TX_UNCONFIRMED) {
					continue;
				}

				if (allTxs[i]->GetTransactionType() == Transaction::registerCR ||
					allTxs[i]->GetTransactionType() == Transaction::updateCR) {
					const CRInfo *pinfo = dynamic_cast<const CRInfo *>(allTxs[i]->GetPayload());
					if (pinfo) {
						nlohmann::json info;
						ByteStream stream(pinfo->GetCode());
						bytes_t pubKey;
						stream.ReadVarBytes(pubKey);
						Address did(pinfo->GetDID());

						info["CROwnerPublicKey"] = pubKey.getHex();
						info["CROwnerDID"] = did.String();
						info["NickName"] = pinfo->GetNickName();
						info["Url"] = pinfo->GetUrl();
						info["Location"] = pinfo->GetLocation();

						j["Status"] = "Registered";
						j["Info"] = info;
					}
				} else if (allTxs[i]->GetTransactionType() == Transaction::unregisterCR) {
					const UnregisterCR *pc = dynamic_cast<const UnregisterCR *>(allTxs[i]->GetPayload());
					if (pc) {
						uint32_t lastBlockHeight = _walletManager->GetWallet()->LastBlockHeight();

						nlohmann::json info;

						info["Confirms"] = allTxs[i]->GetConfirms(lastBlockHeight);

						j["Status"] = "Canceled";
						j["Info"] = info;
					}
				} else if (allTxs[i]->GetTransactionType() == Transaction::returnCRDepositCoin) {
					j["Status"] = "ReturnDeposit";
					j["Info"] = nlohmann::json();
				}
			}

			ArgInfo("r => {}", j.dump());
			return j;
		}

		nlohmann::json MainchainSubWallet::GetVoteInfo(const std::string &type) const {
			ArgInfo("{} {}", _walletManager->GetWallet()->GetWalletID(), GetFunName());

			WalletPtr wallet = _walletManager->GetWallet();
			UTXOArray utxos = wallet->GetVoteUTXO();
			nlohmann::json jinfo;
			time_t timestamp;

			for (UTXOArray::iterator u = utxos.begin(); u != utxos.end(); ++u) {
				const OutputPtr &output = (*u)->Output();
				if (output->GetType() != TransactionOutput::VoteOutput) {
					continue;
				}

				TransactionPtr tx = wallet->TransactionForHash((*u)->Hash());
				assert(tx != nullptr);
				timestamp = tx->GetTimestamp();

				const PayloadVote *pv = dynamic_cast<const PayloadVote *>(output->GetPayload().get());
				if (pv == nullptr) {
					continue;
				}

				const std::vector<VoteContent> &voteContents = pv->GetVoteContent();
				std::for_each(voteContents.cbegin(), voteContents.cend(),
							  [&jinfo, &type, &timestamp](const VoteContent &vc) {
								  nlohmann::json j;
								  if (type.empty() || type == vc.GetTypeString()) {
									  if (vc.GetType() == VoteContent::CRC)
										  j["Amount"] = vc.GetTotalVoteAmount().getDec();
									  else if (vc.GetType() == VoteContent::Delegate)
										  j["Amount"] = vc.GetMaxVoteAmount().getDec();
									  j["Type"] = vc.GetTypeString();
									  j["Timestamp"] = timestamp;
									  j["Expiry"] = nlohmann::json();
									  if (!type.empty()) {
										  nlohmann::json candidateVotes;
										  std::for_each(vc.GetCandidateVotes().cbegin(), vc.GetCandidateVotes().cend(),
														[&vc, &candidateVotes](const CandidateVotes &cv) {
															std::string c;
															if (vc.GetType() == VoteContent::CRC) {
																c = Address(uint168(cv.GetCandidate())).String();
															} else {
																c = cv.GetCandidate().getHex();
															}
															candidateVotes[c] = cv.GetVotes().getDec();
														});
										  j["Votes"] = candidateVotes;
									  }
									  jinfo.push_back(j);
								  }
							  });

			}

			ArgInfo("r => {}", jinfo.dump());

			return jinfo;
		}

		std::string MainchainSubWallet::SponsorProposalDigest(uint8_t type,
		                                                      const std::string &sponsorPublicKey,
		                                                      const std::string &draftHash,
		                                                      const nlohmann::json &budgets,
		                                                      const std::string &recipient) const {
			ArgInfo("{} {}", _walletManager->GetWallet()->GetWalletID(), GetFunName());
			ArgInfo("type: {}", type);
			ArgInfo("sponsorPublicKey: {}", sponsorPublicKey);
			ArgInfo("draftHash: {}", draftHash);
			ArgInfo("budgets: {}", budgets.dump());
			ArgInfo("recipient: {}", recipient);

			PayloadPtr payload = GenerateCRCProposalPayload(type, sponsorPublicKey, "", draftHash, budgets,
			                                                   recipient);
			CRCProposal *crcProposal = static_cast<CRCProposal *>(payload.get());

			ByteStream stream;
			crcProposal->SerializeUnsigned(stream, 0);
			uint256 digest(sha256(stream.GetBytes()));

			ArgInfo("r => {}", digest.GetHex());
			return digest.GetHex();
		}

		std::string MainchainSubWallet::CRSponsorProposalDigest(uint8_t type,
		                                                        const std::string &sponsorPublicKey,
		                                                        const std::string &crSponsorDID,
		                                                        const std::string &draftHash,
		                                                        const nlohmann::json &budgets,
		                                                        const std::string &recipient,
		                                                        const std::string &sponsorSignature) const {
			ArgInfo("{} {}", _walletManager->GetWallet()->GetWalletID(), GetFunName());
			ArgInfo("type: {}", type);
			ArgInfo("sponsorPublicKey: {}", sponsorPublicKey);
			ArgInfo("crSponsorDID: {}", crSponsorDID);
			ArgInfo("draftHash: {}", draftHash);
			ArgInfo("budgets: {}", budgets.dump());
			ArgInfo("recipient: {}", recipient);
			ArgInfo("sponsorSignature: {}", sponsorSignature);

			ErrorChecker::CheckParam(sponsorSignature.empty(), Error::InvalidArgument, "invalid sponsorSignature");

			PayloadPtr payload = GenerateCRCProposalPayload(type, sponsorPublicKey, crSponsorDID, draftHash, budgets,
			                                                recipient, sponsorSignature);
			CRCProposal *crcProposal = static_cast<CRCProposal *>(payload.get());

			ByteStream stream;
			crcProposal->SerializeSponsorSigned(stream, 0);
			uint256 digest(sha256(stream.GetBytes()));

			ArgInfo("r => {}", digest.GetHex());
			return digest.GetHex();
		}

		nlohmann::json MainchainSubWallet::CreateCRCProposalTransaction(uint8_t type,
		                                                                const std::string &sponsorPublicKey,
		                                                                const std::string &crSponsorDID,
		                                                                const std::string &draftHash,
		                                                                const nlohmann::json &budgets,
		                                                                const std::string &recipient,
		                                                                const std::string &sponsorSignature,
		                                                                const std::string &crSponsorSignature,
		                                                                const std::string &memo) {
			ArgInfo("{} {}", _walletManager->GetWallet()->GetWalletID(), GetFunName());
			ArgInfo("type: {}", type);
			ArgInfo("sponsorPublicKey: {}", sponsorPublicKey);
			ArgInfo("crSponsorDID: {}", crSponsorDID);
			ArgInfo("draftHash: {}", draftHash);
			ArgInfo("budgets: {}", budgets.dump());
			ArgInfo("recipient: {}", recipient);
			ArgInfo("sponsorSignature: {}", sponsorSignature);
			ArgInfo("crSponsorSignature: {}", crSponsorSignature);
			ArgInfo("memo: {}", memo);

			ErrorChecker::CheckParam(sponsorSignature.empty(), Error::InvalidArgument, "invalid sponsorSignature");
			ErrorChecker::CheckParam(crSponsorSignature.empty(), Error::InvalidArgument, "invalid crSponsorSignature");

			PayloadPtr payload = GenerateCRCProposalPayload(type, sponsorPublicKey, crSponsorDID, draftHash, budgets,
			                                                recipient, sponsorSignature, crSponsorSignature);
			CRCProposal *crcProposal = static_cast<CRCProposal *>(payload.get());

			Address receiveAddr(CreateAddress());
			std::vector<OutputPtr> outputs;
			outputs.push_back(OutputPtr(new TransactionOutput(BigInt(0), receiveAddr)));

			TransactionPtr tx = CreateTx(Transaction::crcProposal, payload, "", outputs, memo);

			if (tx->GetOutputs().size() > 1) {
				tx->RemoveOutput(tx->GetOutputs().front());
				tx->FixIndex();
			}

			nlohmann::json result;
			EncodeTx(result, tx);

			ArgInfo("r => {}", result.dump());
			return result;
		}

		PayloadPtr MainchainSubWallet::GenerateCRCProposalPayload(uint8_t type,
		                                                          const std::string &sponsorPublicKey,
		                                                          const std::string &crSponsorDID,
		                                                          const std::string &draftHash,
		                                                          const nlohmann::json &budgets,
		                                                          const std::string &recipient,
		                                                          const std::string &sponsorSignature,
		                                                          const std::string &crSponsorSignature) const {
			ErrorChecker::CheckParam(type >= CRCProposal::maxType, Error::InvalidArgument, "type is invalid");

			Key verifyPubKey;
			bytes_t publicKey(sponsorPublicKey);
			verifyPubKey.SetPubKey(publicKey);

			uint256 proposalHash(draftHash);
			ErrorChecker::CheckParam(proposalHash.GetHex() != draftHash, Error::InvalidArgument, "invalid draftHash");

			Address receiptAddress(recipient);
			ErrorChecker::CheckParam(!receiptAddress.Valid(), Error::InvalidArgument, "invalid recipient");

			PayloadPtr payload = PayloadPtr(new CRCProposal());
			CRCProposal *crcProposal = static_cast<CRCProposal *>(payload.get());
			crcProposal->SetTpye(CRCProposal::CRCProposalType(type));
			crcProposal->SetDraftHash(proposalHash);
			crcProposal->SetRecipient(receiptAddress.ProgramHash());

			std::vector<uint64_t> budgetList;
			for (size_t i = 0; i < budgets.size(); ++i) {
				std::string amount = budgets[i].get<std::string>();
				int64_t value = strtoll(amount.c_str(), NULL, 10);
				ErrorChecker::CheckParam(value < 0, Error::InvalidArgument, "invalid budgets");
				budgetList.push_back(value);
			}
			crcProposal->SetBudgets(budgetList);

			if (!sponsorSignature.empty()) {
				crcProposal->SetSignature(sponsorSignature);
			}

			if (!crSponsorDID.empty()) {
				Address did(crSponsorDID);
				ErrorChecker::CheckParam(!did.Valid() || !did.IsIDAddress(), Error::InvalidArgument,
				                         "crSponsorDID is invalid");

				crcProposal->SetCRSponsorDID(did.ProgramHash());
			}

			if (!crSponsorSignature.empty()) {
				crcProposal->SetCRSignature(crSponsorSignature);
			}

			return payload;
		}

		nlohmann::json MainchainSubWallet::GenerateCRCProposalReview(const std::string &proposalHash,
		                                                             uint8_t voteResult,
		                                                             const std::string &did) const {
			ArgInfo("{} {}", _walletManager->GetWallet()->GetWalletID(), GetFunName());
			ArgInfo("proposalHash: {}", proposalHash);
			ArgInfo("voteResult: {}", voteResult);
			ArgInfo("did: {}", did);

			ErrorChecker::CheckParam(proposalHash.size() != 64, Error::InvalidArgument, "invalid proposalHash");
			ErrorChecker::CheckParam(voteResult > 2, Error::InvalidArgument, "invalid voteResult");

			Address address(did);
			ErrorChecker::CheckParam(!address.Valid(), Error::InvalidArgument, "invalid crDID value");

			CRCProposalReview review;

			uint256 hash(proposalHash);
			review.SetProposalHash(hash);
			review.SetResult((CRCProposalReview::VoteResult) voteResult);
			review.SetCRDID(address.ProgramHash());

			ByteStream byteStream;
			review.SerializeUnsigned(byteStream, 0);

			nlohmann::json result = review.ToJson(0);
			uint256 digest(sha256(byteStream.GetBytes()));
			result["Digest"] = digest.GetHex();

			ArgInfo("r => {}", result.dump());
			return result;
		}

		nlohmann::json MainchainSubWallet::CreateCRCProposalReviewTransaction(const nlohmann::json &proposalReview,
		                                                                      const std::string &memo) {
			ArgInfo("{} {}", _walletManager->GetWallet()->GetWalletID(), GetFunName());
			ArgInfo("proposalReview: {}", proposalReview.dump());
			ArgInfo("memo: {}", memo);

			ErrorChecker::CheckParam(proposalReview.find("Signature") == proposalReview.end(), Error::InvalidArgument, "no signed proposal review");
			std::string signature = proposalReview["Signature"].get<std::string>();
			ErrorChecker::CheckParam(signature.empty(), Error::InvalidArgument, "no signed proposal review");

			PayloadPtr payload = PayloadPtr(new CRCProposalReview());
			try {
				payload->FromJson(proposalReview, 0);
			} catch (const nlohmann::detail::exception &e) {
				ErrorChecker::ThrowParamException(Error::JsonFormatError,
				                                  "Payload format err: " + std::string(e.what()));
			}

			std::vector<OutputPtr> outputs;
			Address receiveAddr(CreateAddress());
			outputs.push_back(OutputPtr(new TransactionOutput(0, receiveAddr)));

			TransactionPtr tx = CreateTx(Transaction::crcProposalReview, payload, "", outputs, memo);

			if (tx->GetOutputs().size() > 1) {
				tx->RemoveOutput(tx->GetOutputs().front());
				tx->FixIndex();
			}

			nlohmann::json result;
			EncodeTx(result, tx);

			ArgInfo("r => {}", result.dump());
			return result;
		}

		nlohmann::json MainchainSubWallet::CreateVoteCRCProposalTransaction(const std::string &fromAddress,
		                                                                    const nlohmann::json &votes,
		                                                                    const std::string &memo) {
			ArgInfo("{} {}", _walletManager->GetWallet()->GetWalletID(), GetFunName());
			ArgInfo("fromAddr: {}", fromAddress);
			ArgInfo("votes: {}", votes.dump());
			ArgInfo("memo: {}", memo);

			ErrorChecker::CheckParam(!votes.is_object(), Error::Code::JsonFormatError, "votes is error json format");

			BigInt bgStake = 0;

			VoteContent voteContent(VoteContent::CRCProposal);
			std::vector<CandidateVotes> candidates;
			bytes_t candidate;
			BigInt value;
			for (nlohmann::json::const_iterator it = votes.cbegin(); it != votes.cend(); ++it) {
				ErrorChecker::CheckParam(!it.value().is_string(), Error::InvalidArgument, "stake value should be big int string");

				candidate = it.key();
				ErrorChecker::CheckParam(candidate.size() != 32, Error::InvalidArgument, "invalid proposal hash");

				value.setDec(it.value().get<std::string>());
				ErrorChecker::CheckParam(value <= 0, Error::InvalidArgument, "stake value should larger than 0");

				voteContent.AddCandidate(CandidateVotes(candidate, value));
			}

			TransactionPtr tx = CreateVoteTx(voteContent, memo, false);

			nlohmann::json result;
			EncodeTx(result, tx);

			ArgInfo("r => {}", result.dump());

			return result;
		}

		nlohmann::json MainchainSubWallet::CreateImpeachmentCRCTransaction(const std::string &fromAddress,
		                                                                   const nlohmann::json &votes,
		                                                                   const std::string &memo) {
			ArgInfo("{} {}", _walletManager->GetWallet()->GetWalletID(), GetFunName());
			ArgInfo("fromAddr: {}", fromAddress);
			ArgInfo("votes: {}", votes.dump());
			ArgInfo("memo: {}", memo);

			ErrorChecker::CheckParam(!votes.is_object(), Error::Code::JsonFormatError, "votes is error json format");

			BigInt bgStake = 0;

			VoteContent voteContent(VoteContent::CRCImpeachment);
			std::vector<CandidateVotes> candidates;
			std::string key;
			bytes_t candidate;
			BigInt value;
			for (nlohmann::json::const_iterator it = votes.cbegin(); it != votes.cend(); ++it) {
				ErrorChecker::CheckParam(!it.value().is_string(), Error::InvalidArgument, "stake value should be big int string");

				key = it.key();
				Address didAddress(key);
				ErrorChecker::CheckParam(!didAddress.Valid(), Error::InvalidArgument, "invalid candidate did");
				candidate = didAddress.ProgramHash().bytes();

				value.setDec(it.value().get<std::string>());
				ErrorChecker::CheckParam(value <= 0, Error::InvalidArgument, "stake value should larger than 0");

				voteContent.AddCandidate(CandidateVotes(candidate, value));
			}

			TransactionPtr tx = CreateVoteTx(voteContent, memo, false);

			nlohmann::json result;
			EncodeTx(result, tx);

			ArgInfo("r => {}", result.dump());

			return result;
		}

	}
}
