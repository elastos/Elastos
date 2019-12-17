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
#include <Plugin/Transaction/Payload/CRCProposalTracking.h>
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
											   MasterWallet *parent,
											   const std::string &netType) :
				SubWallet(info, config, parent, netType) {
			InitData();
		}

		MainchainSubWallet::~MainchainSubWallet() {
			_crList.clear();
		}

		void MainchainSubWallet::InitData() {
			bytes_t types;

			types.push_back(Transaction::registerCR);
			types.push_back(Transaction::unregisterCR);
			types.push_back(Transaction::updateCR);
			types.push_back(Transaction::returnCRDepositCoin);
			types.push_back(Transaction::registerProducer);
			types.push_back(Transaction::cancelProducer);
			types.push_back(Transaction::updateProducer);
			types.push_back(Transaction::returnDepositCoin);

			std::vector<TransactionPtr> list = _walletManager->GetWallet()->GetTransactions(types);

			for (std::vector<TransactionPtr>::iterator it = list.begin(); it != list.end(); ++it) {
				TransactionPtr tx = *it;
				uint8_t type = tx->GetTransactionType();
				if (type == Transaction::registerCR || type == Transaction::unregisterCR ||
				    type == Transaction::updateCR || type == Transaction::returnCRDepositCoin) {
					_crList.push_back(tx);
				} else if (type == Transaction::registerProducer || type == Transaction::cancelProducer ||
				           type == Transaction::updateProducer || type == Transaction::returnDepositCoin) {
					_producerList.push_back(tx);
				} else {
					Log::warn("contains tx we don't want");
				}
			}
		}

		nlohmann::json MainchainSubWallet::CreateDepositTransaction(const std::string &fromAddress,
																	const std::string &sideChainID,
																	const std::string &amount,
																	const std::string &sideChainAddress,
																	const std::string &memo) {
			ArgInfo("{} {}", _walletManager->GetWallet()->GetWalletID(), GetFunName());
			ArgInfo("fromAddr: {}", fromAddress);
			ArgInfo("sideChainID: {}", sideChainID);
			ArgInfo("amount: {}", amount);
			ArgInfo("sideChainAddr: {}", sideChainAddress);
			ArgInfo("memo: {}", memo);

			ErrorChecker::CheckBigIntAmount(amount);
			ErrorChecker::CheckParam(sideChainID == CHAINID_MAINCHAIN, Error::InvalidArgument, "can not be mainChain");

			BigInt value;
			value.setDec(amount);

			TransferInfo info(sideChainAddress, 0, value);
			PayloadPtr payload = PayloadPtr(new TransferCrossChainAsset({info}));

			ChainConfigPtr configPtr =  _parent->GetChainConfig(sideChainID);
			std::vector<OutputPtr> outputs;
			Address receiveAddr(configPtr->GenesisAddress());
			outputs.emplace_back(OutputPtr(new TransactionOutput(value + _config->MinFee(), receiveAddr)));
			AddressPtr fromAddr(new Address(fromAddress));

			TransactionPtr tx = CreateTx(Transaction::transferCrossChainAsset, payload, fromAddr, outputs, memo);

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

			ErrorChecker::CheckBigIntAmount(amount);
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

			std::vector<OutputPtr> outputs;
			Address receiveAddr(PrefixDeposit, pubkey);
			outputs.push_back(OutputPtr(new TransactionOutput(bgAmount, receiveAddr)));
			AddressPtr fromAddr(new Address(fromAddress));

			TransactionPtr tx = CreateTx(Transaction::registerProducer, payload, fromAddr, outputs, memo);

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
			AddressPtr receiveAddr = _walletManager->GetWallet()->GetReceiveAddress();
			outputs.push_back(OutputPtr(new TransactionOutput(BigInt(0), *receiveAddr)));
			AddressPtr fromAddr(new Address(fromAddress));

			TransactionPtr tx = CreateTx(Transaction::updateProducer, payload, fromAddr, outputs, memo);

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
			AddressPtr receiveAddr = _walletManager->GetWallet()->GetReceiveAddress();
			outputs.push_back(OutputPtr(new TransactionOutput(BigInt(0), *receiveAddr)));
			AddressPtr fromAddr(new Address(fromAddress));

			TransactionPtr tx = CreateTx(Transaction::cancelProducer, payload, fromAddr, outputs, memo);

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

			ErrorChecker::CheckBigIntAmount(amount);
			BigInt bgAmount;
			bgAmount.setDec(amount);

			AddressPtr fromAddress = _walletManager->GetWallet()->GetOwnerDepositAddress();

			std::vector<OutputPtr> outputs;
			AddressPtr receiveAddr = _walletManager->GetWallet()->GetReceiveAddress();
			outputs.push_back(OutputPtr(new TransactionOutput(bgAmount, *receiveAddr)));

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

			std::string address = _walletManager->GetWallet()->GetOwnerAddress()->String();

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
			WalletPtr wallet = _walletManager->GetWallet();
			ArgInfo("{} {}", wallet->GetWalletID(), GetFunName());

			nlohmann::json j, info;

			j["Status"] = "Unregistered";
			j["Info"] = nlohmann::json();
			for (std::vector<TransactionPtr>::const_iterator it = _producerList.cbegin(); it != _producerList.cend(); ++it) {
				TransactionPtr tx = (*it);
				if (tx->GetBlockHeight() == TX_UNCONFIRMED) {
					continue;
				}

				if (tx->GetTransactionType() == Transaction::registerProducer ||
				    tx->GetTransactionType() == Transaction::updateProducer) {
					const ProducerInfo *pinfo = dynamic_cast<const ProducerInfo *>(tx->GetPayload());
					if (pinfo) {
						info["OwnerPublicKey"] = pinfo->GetPublicKey().getHex();
						info["NodePublicKey"] = pinfo->GetNodePublicKey().getHex();
						info["NickName"] = pinfo->GetNickName();
						info["URL"] = pinfo->GetUrl();
						info["Location"] = pinfo->GetLocation();
						info["Address"] = pinfo->GetAddress();
						info["Confirms"] = tx->GetConfirms(wallet->LastBlockHeight());

						j["Status"] = "Registered";
						j["Info"] = info;
					}
				} else if (tx->GetTransactionType() == Transaction::cancelProducer) {
					const CancelProducer *pc = dynamic_cast<const CancelProducer *>(tx->GetPayload());
					if (pc) {
						info["Confirms"] = tx->GetConfirms(wallet->LastBlockHeight());

						j["Status"] = "Canceled";
						j["Info"] = info;
					}
				} else if (tx->GetTransactionType() == Transaction::returnDepositCoin) {
					info["Confirms"] = tx->GetConfirms(wallet->LastBlockHeight());

					j["Status"] = "ReturnDeposit";
					j["Info"] = info;
				}
			}

			ArgInfo("r => {}", j.dump());
			return j;
		}

		nlohmann::json MainchainSubWallet::GenerateCRInfoPayload(
				const std::string &crPublicKey,
				const std::string &nickName,
				const std::string &url,
				uint64_t location) const {
			ArgInfo("{} {}", _walletManager->GetWallet()->GetWalletID(), GetFunName());
			ArgInfo("crPublicKey: {}", crPublicKey);
			ArgInfo("nickName: {}", nickName);
			ArgInfo("url: {}", url);
			ArgInfo("location: {}", location);

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
			uint256 digest(sha256(ostream.GetBytes()));

			nlohmann::json payloadJson = crInfo.ToJson(0);
			payloadJson["Digest"] = digest.GetHex();

			ArgInfo("r => {}", payloadJson.dump());
			return payloadJson;
		}

		nlohmann::json MainchainSubWallet::GenerateUnregisterCRPayload(const std::string &crDID) const {
			ArgInfo("{} {}", _walletManager->GetWallet()->GetWalletID(), GetFunName());
			ArgInfo("crDID: {}", crDID);

			Address address(crDID);
			ErrorChecker::CheckParam(!address.Valid(), Error::InvalidArgument, "invalid crDID");

			UnregisterCR unregisterCR;
			unregisterCR.SetDID(address.ProgramHash());

			ByteStream ostream;
			unregisterCR.SerializeUnsigned(ostream, 0);
			uint256 digest(sha256(ostream.GetBytes()));

			nlohmann::json payloadJson = unregisterCR.ToJson(0);
			payloadJson["Digest"] = digest.GetHex();

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

			ErrorChecker::CheckBigIntAmount(amount);
			BigInt bgAmount, minAmount(DEPOSIT_MIN_ELA);
			bgAmount.setDec(amount);

			minAmount *= SELA_PER_ELA;

			ErrorChecker::CheckParam(bgAmount < minAmount, Error::DepositAmountInsufficient,
			                         "cr deposit amount is insufficient");

			ErrorChecker::CheckParam(payloadJSON.find("Signature") == payloadJSON.end(), Error::InvalidArgument,
			                         "Signature can not be empty");

			PayloadPtr payload = PayloadPtr(new CRInfo());
			try {
				payload->FromJson(payloadJSON, 0);
				ErrorChecker::CheckParam(!payload->IsValid(), Error::InvalidArgument, "verify signature failed");
			} catch (const nlohmann::detail::exception &e) {
				ErrorChecker::ThrowParamException(Error::JsonFormatError,
				                                  "Payload format err: " + std::string(e.what()));
			}

			bytes_t code = static_cast<CRInfo *>(payload.get())->GetCode();
			Address receiveAddr;
			receiveAddr.SetRedeemScript(PrefixDeposit, code);
			AddressPtr fromAddr(new Address(fromAddress));

			std::vector<OutputPtr> outputs;
			outputs.push_back(OutputPtr(new TransactionOutput(bgAmount, receiveAddr)));

			TransactionPtr tx = CreateTx(Transaction::registerCR, payload, fromAddr, outputs, memo);

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
			AddressPtr receiveAddr = _walletManager->GetWallet()->GetReceiveAddress();
			outputs.push_back(OutputPtr(new TransactionOutput(BigInt(0), *receiveAddr)));
			AddressPtr fromAddr(new Address(fromAddress));

			TransactionPtr tx = CreateTx(Transaction::updateCR, payload, fromAddr, outputs, memo);

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

			std::vector<OutputPtr> outputs;
			AddressPtr receiveAddr = _walletManager->GetWallet()->GetReceiveAddress();
			outputs.push_back(OutputPtr(new TransactionOutput(BigInt(0), *receiveAddr)));
			AddressPtr fromAddr(new Address(fromAddress));

			TransactionPtr tx = CreateTx(Transaction::unregisterCR, payload, fromAddr, outputs, memo);

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
				const std::string &crPublicKey,
				const std::string &amount,
				const std::string &memo) {
			ArgInfo("{} {}", _walletManager->GetWallet()->GetWalletID(), GetFunName());
			ArgInfo("pubkey: {}", crPublicKey);
			ArgInfo("amount: {}", amount);
			ArgInfo("memo: {}", memo);

			ErrorChecker::CheckBigIntAmount(amount);
			BigInt bgAmount;
			bgAmount.setDec(amount);

			AddressPtr fromAddress(new Address(PrefixDeposit, bytes_t(crPublicKey)));
			ErrorChecker::CheckParam(!fromAddress->Valid(), Error::InvalidArgument, "invalid crPublicKey");

			std::vector<OutputPtr> outputs;
			AddressPtr receiveAddr = _walletManager->GetWallet()->GetReceiveAddress();
			outputs.push_back(OutputPtr(new TransactionOutput(bgAmount, *receiveAddr)));

			PayloadPtr payload = PayloadPtr(new ReturnDepositCoin());
			TransactionPtr tx = CreateTx(Transaction::returnCRDepositCoin, payload, fromAddress, outputs, memo);

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
				std::string voteAmount = it.value().get<std::string>();
				ErrorChecker::CheckBigIntAmount(voteAmount);

				key = it.key();
				Address didAddress(key);
				ErrorChecker::CheckParam(!didAddress.Valid(), Error::InvalidArgument, "invalid candidate did");
				candidate = didAddress.ProgramHash().bytes();

				value.setDec(voteAmount);
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
			WalletPtr wallet = _walletManager->GetWallet();
			ArgInfo("{} {}", wallet->GetWalletID(), GetFunName());

			nlohmann::json j, info;

			j["Status"] = "Unregistered";
			j["Info"] = nlohmann::json();
			for (std::vector<TransactionPtr>::const_iterator it = _crList.cbegin(); it != _crList.cend(); ++it) {
				TransactionPtr tx = *it;
				if (tx->GetBlockHeight() == TX_UNCONFIRMED) {
					continue;
				}

				if (tx->GetTransactionType() == Transaction::registerCR ||
				    tx->GetTransactionType() == Transaction::updateCR) {
					const CRInfo *pinfo = dynamic_cast<const CRInfo *>(tx->GetPayload());
					if (pinfo) {
						ByteStream stream(pinfo->GetCode());
						bytes_t pubKey;
						stream.ReadVarBytes(pubKey);
						Address did(pinfo->GetDID());

						info["CROwnerPublicKey"] = pubKey.getHex();
						info["CROwnerDID"] = did.String();
						info["NickName"] = pinfo->GetNickName();
						info["Url"] = pinfo->GetUrl();
						info["Location"] = pinfo->GetLocation();
						info["Confirms"] = tx->GetConfirms(wallet->LastBlockHeight());

						j["Status"] = "Registered";
						j["Info"] = info;
					}
				} else if (tx->GetTransactionType() == Transaction::unregisterCR) {
					const UnregisterCR *pc = dynamic_cast<const UnregisterCR *>(tx->GetPayload());
					if (pc) {
						info["Confirms"] = tx->GetConfirms(wallet->LastBlockHeight());

						j["Status"] = "Canceled";
						j["Info"] = info;

					}
				} else if (tx->GetTransactionType() == Transaction::returnCRDepositCoin) {
					info["Confirms"] = tx->GetConfirms(wallet->LastBlockHeight());

					j["Status"] = "ReturnDeposit";
					j["Info"] = info;
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
									  jinfo.push_back(j);
								  }
							  });

			}

			ArgInfo("r => {}", jinfo.dump());

			return jinfo;
		}

		nlohmann::json MainchainSubWallet::SponsorProposalDigest(uint8_t type,
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

			nlohmann::json result;

			PayloadPtr payload = GenerateCRCProposalPayload(type, sponsorPublicKey, "", draftHash, budgets,
			                                                   recipient);
			CRCProposal *crcProposal = static_cast<CRCProposal *>(payload.get());
			result = crcProposal->ToJson(0);

			ByteStream stream;
			crcProposal->SerializeUnsigned(stream, 0);
			uint256 digest(sha256(stream.GetBytes()));

			result["Digest"] = digest.GetHex();
			result.erase("CRSignature");
			result.erase("CRSponsorDID");

			ArgInfo("r => {}", result.dump());
			return result;
		}

		nlohmann::json MainchainSubWallet::CRSponsorProposalDigest(const nlohmann::json &sponsorSignedProposal,
		                                                           const std::string &crSponsorDID) const {
			ArgInfo("{} {}", _walletManager->GetWallet()->GetWalletID(), GetFunName());
			ArgInfo("sponsorSignedProposal: {}", sponsorSignedProposal.dump());
			ArgInfo("crSponsorDID: {}", crSponsorDID);

			std::string sponsorSignature = sponsorSignedProposal["Signature"].get<std::string>();
			ErrorChecker::CheckParam(sponsorSignature.empty(), Error::InvalidArgument, "invalid sponsorSignature");

			Address did(crSponsorDID);
			ErrorChecker::CheckParam(!did.Valid() || !did.IsIDAddress(), Error::InvalidArgument,
			                         "crSponsorDID is invalid");


			CRCProposal crcProposal;
			crcProposal.FromJson(sponsorSignedProposal, 0);
			crcProposal.SetCRSponsorDID(did.ProgramHash());

			nlohmann::json result = crcProposal.ToJson(0);

			ByteStream stream;
			crcProposal.SerializeSponsorSigned(stream, 0);
			uint256 digest(sha256(stream.GetBytes()));

			result["Digest"] = digest.GetHex();

			ArgInfo("r => {}", result.dump());
			return result;
		}

		nlohmann::json MainchainSubWallet::CreateCRCProposalTransaction(nlohmann::json crSignedProposal,
		                                                                const std::string &memo) {
			ArgInfo("{} {}", _walletManager->GetWallet()->GetWalletID(), GetFunName());
			ArgInfo("crSignedProposal: {}", crSignedProposal.dump());
			ArgInfo("memo: {}", memo);

			std::string sponsorSignature = crSignedProposal["Signature"].get<std::string>();
			ErrorChecker::CheckParam(sponsorSignature.empty(), Error::InvalidArgument, "invalid sponsorSignature");

			std::string crSponsorSignature = crSignedProposal["CRSignature"].get<std::string>();
			ErrorChecker::CheckParam(crSponsorSignature.empty(), Error::InvalidArgument, "invalid crSponsorSignature");

			PayloadPtr payload = PayloadPtr(new CRCProposal());
			payload->FromJson(crSignedProposal, 0);

			AddressPtr receiveAddr = _walletManager->GetWallet()->GetReceiveAddress();
			std::vector<OutputPtr> outputs;
			outputs.push_back(OutputPtr(new TransactionOutput(BigInt(0), *receiveAddr)));
			AddressPtr fromAddr(new Address(""));

			TransactionPtr tx = CreateTx(Transaction::crcProposal, payload, fromAddr, outputs, memo);

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
			ErrorChecker::CheckParam(!budgets.is_array(), Error::InvalidArgument, "invalid budgets");

			Address receiptAddress(recipient);
			ErrorChecker::CheckParam(!receiptAddress.Valid(), Error::InvalidArgument, "invalid recipient");

			PayloadPtr payload = PayloadPtr(new CRCProposal());
			CRCProposal *crcProposal = static_cast<CRCProposal *>(payload.get());
			crcProposal->SetTpye(CRCProposal::CRCProposalType(type));
			crcProposal->SetDraftHash(proposalHash);
			crcProposal->SetRecipient(receiptAddress.ProgramHash());
			crcProposal->SetSponsorPublicKey(publicKey);

			std::vector<BigInt> budgetList;
			for (nlohmann::json::const_iterator  it = budgets.cbegin(); it != budgets.cend(); ++it) {
				ErrorChecker::CheckBigIntAmount((*it).get<std::string>());
				BigInt amount;
				amount.setDec((*it).get<std::string>());
				ErrorChecker::CheckParam(amount < 0, Error::InvalidArgument, "invalid budgets");
				budgetList.push_back(amount);
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
			AddressPtr receiveAddr = _walletManager->GetWallet()->GetReceiveAddress();
			outputs.push_back(OutputPtr(new TransactionOutput(0, *receiveAddr)));
			AddressPtr fromAddr(new Address(""));

			TransactionPtr tx = CreateTx(Transaction::crcProposalReview, payload, fromAddr, outputs, memo);

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
				ErrorChecker::CheckBigIntAmount(it.value().get<std::string>());

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

		nlohmann::json MainchainSubWallet::LeaderProposalTrackDigest(uint8_t type,
		                                                             const std::string &proposalHash,
		                                                             const std::string &documentHash,
		                                                             uint8_t stage,
		                                                             const std::string &appropriation,
		                                                             const std::string &leaderPubKey,
		                                                             const std::string &newLeaderPubKey) const {
			ArgInfo("{} {}", _walletManager->GetWallet()->GetWalletID(), GetFunName());
			ArgInfo("type: {}", type);
			ArgInfo("proposalHash: {}", proposalHash);
			ArgInfo("documentHash: {}", documentHash);
			ArgInfo("stage: {}", stage);
			ArgInfo("appropriation: {}", appropriation);
			ArgInfo("leaderPubKey: {}", leaderPubKey);
			ArgInfo("newLeaderPubKey: {}", newLeaderPubKey);

			ErrorChecker::CheckParam(type >= CRCProposalTracking::maxType, Error::InvalidArgument, "type is invalid");

			uint256 proposal(proposalHash);
			ErrorChecker::CheckParam(proposal.GetHex() != proposalHash, Error::InvalidArgument, "invalid proposalHash");

			uint256 document(documentHash);
			ErrorChecker::CheckParam(document.GetHex() != documentHash, Error::InvalidArgument, "invalid documentHash");

			BigInt appropriationValue;
			appropriationValue.setDec(appropriation);
			ErrorChecker::CheckParam(appropriationValue < 0, Error::InvalidArgument, "invalid appropriation");

			Key verifyPubKey;
			verifyPubKey.SetPubKey(leaderPubKey);

			if (type == CRCProposalTracking::CRCProposalTrackingType::proposalLeader) {
				verifyPubKey.SetPubKey(newLeaderPubKey);
			} else {
				ErrorChecker::CheckParam(!newLeaderPubKey.empty(), Error::InvalidArgument, "invalid newLeaderPubKey");
			}

			CRCProposalTracking tracking;
			tracking.SetType(CRCProposalTracking::CRCProposalTrackingType(type));
			tracking.SetProposalHash(proposal);
			tracking.SetDocumentHash(document);
			tracking.SetStage(stage);
			tracking.SetAppropriation(appropriationValue.getUint64());
			tracking.SetLeaderPubKey(leaderPubKey);
			tracking.SetNewLeaderPubKey(newLeaderPubKey);

			nlohmann::json result = tracking.ToJson(0);
			result.erase("NewLeaderSign");
			result.erase("SecretaryGeneralSign");

			ByteStream stream;
			tracking.SerializeUnsigned(stream, 0);
			uint256 digest(sha256(stream.GetBytes()));
			result["Digest"] = digest.GetHex();

			ArgInfo("r => {}", result.dump());
			return result;
		}

		nlohmann::json
		MainchainSubWallet::NewLeaderProposalTrackDigest(const nlohmann::json &leaderSignedProposalTracking) const {
			ArgInfo("{} {}", _walletManager->GetWallet()->GetWalletID(), GetFunName());
			ArgInfo("leaderSignedProposalTracking: {}", leaderSignedProposalTracking.dump());

			std::string signature = leaderSignedProposalTracking["LeaderSign"].get<std::string>();
			ErrorChecker::CheckParam(signature.empty(), Error::InvalidArgument, "no signed proposal tracking");

			CRCProposalTracking tracking;
			tracking.FromJson(leaderSignedProposalTracking, 0);

			ErrorChecker::CheckParam(tracking.GetType() != CRCProposalTracking::CRCProposalTrackingType::proposalLeader,
			                         Error::InvalidArgument, "invalid tracking type");

			ByteStream stream;
			tracking.SerializeUnsigned(stream, 0);
			stream.WriteVarBytes(tracking.GetLeaderSign());

			nlohmann::json result = tracking.ToJson(0);
			result.erase("SecretaryGeneralSign");

			uint256 digest(sha256(stream.GetBytes()));
			result["Digest"] = digest.GetHex();

			ArgInfo("r => {}", result.dump());
			return result;
		}

		nlohmann::json MainchainSubWallet::SecretaryGeneralProposalTrackDigest(
				const nlohmann::json &leaderSignedProposalTracking) const {
			ArgInfo("{} {}", _walletManager->GetWallet()->GetWalletID(), GetFunName());
			ArgInfo("leaderSignedProposalTracking: {}", leaderSignedProposalTracking.dump());

			ErrorChecker::CheckParam(
					leaderSignedProposalTracking.find("LeaderSign") == leaderSignedProposalTracking.end(),
					Error::InvalidArgument, "no signed proposal tracking");
			std::string signature = leaderSignedProposalTracking["LeaderSign"].get<std::string>();
			ErrorChecker::CheckParam(signature.empty(), Error::InvalidArgument, "no signed proposal tracking");

			CRCProposalTracking tracking;
			tracking.FromJson(leaderSignedProposalTracking, 0);

			ByteStream stream;
			tracking.SerializeUnsigned(stream, 0);
			stream.WriteVarBytes(tracking.GetLeaderSign());

			if (tracking.GetType() == CRCProposalTracking::CRCProposalTrackingType::proposalLeader) {
				bytes_t newLeaderSign = tracking.GetNewLeaderSign();
				ErrorChecker::CheckParam(newLeaderSign.empty(), Error::InvalidArgument,
				                         "no signed proposal tracking by new leader");

				ErrorChecker::CheckParam(tracking.GetNewLeaderPubKey().empty(), Error::InvalidArgument,
				                         "new leader public key is empty");

				stream.WriteVarBytes(newLeaderSign);
			}

			nlohmann::json result = tracking.ToJson(0);
			uint256 digest(sha256(stream.GetBytes()));
			result["Digest"] = digest.GetHex();

			ArgInfo("r => {}", result.dump());
			return result;
		}

		nlohmann::json
		MainchainSubWallet::CreateProposalTrackingTransaction(const nlohmann::json &SecretaryGeneralSignedPayload,
		                                  const std::string &memo) {
			ArgInfo("{} {}", _walletManager->GetWallet()->GetWalletID(), GetFunName());
			ArgInfo("SecretaryGeneralSignedPayload: {}", SecretaryGeneralSignedPayload.dump());
			ArgInfo("memo: {}", memo);

			ErrorChecker::CheckParam(
					SecretaryGeneralSignedPayload.find("LeaderSign") == SecretaryGeneralSignedPayload.end(),
					Error::InvalidArgument, "no signed proposal tracking");
			std::string signature = SecretaryGeneralSignedPayload["LeaderSign"].get<std::string>();
			ErrorChecker::CheckParam(signature.empty(), Error::InvalidArgument, "no signed proposal tracking");

			PayloadPtr payload = PayloadPtr(new CRCProposalTracking());
			try {
				payload->FromJson(SecretaryGeneralSignedPayload, 0);
			} catch (const nlohmann::detail::exception &e) {
				ErrorChecker::ThrowParamException(Error::JsonFormatError,
				                                  "Payload format err: " + std::string(e.what()));
			}

			CRCProposalTracking *tracking = static_cast<CRCProposalTracking *>(payload.get());

			if (tracking->GetType() == CRCProposalTracking::CRCProposalTrackingType::proposalLeader) {

				ErrorChecker::CheckParam(tracking->GetNewLeaderSign().empty(), Error::InvalidArgument,
				                         "new leader no signed proposal tracking");

				ErrorChecker::CheckParam(tracking->GetNewLeaderPubKey().empty(), Error::InvalidArgument,
				                         "new leader public key is empty");
			}

			std::vector<OutputPtr> outputs;
			AddressPtr receiveAddr = _walletManager->GetWallet()->GetReceiveAddress();
			outputs.push_back(OutputPtr(new TransactionOutput(0, *receiveAddr)));
			AddressPtr fromAddr(new Address(""));

			TransactionPtr tx = CreateTx(Transaction::crcProposalTracking, payload, fromAddr, outputs, memo);

			if (tx->GetOutputs().size() > 1) {
				tx->RemoveOutput(tx->GetOutputs().front());
				tx->FixIndex();
			}

			nlohmann::json result;
			EncodeTx(result, tx);
			ArgInfo("r => {}", result.dump());
			return result;
		}

		void MainchainSubWallet::onTxAdded(const TransactionPtr &tx) {
			uint8_t type = tx->GetTransactionType();
			Lock();
			if (type == Transaction::registerCR ||
			    type == Transaction::unregisterCR ||
			    type == Transaction::updateCR ||
			    type == Transaction::returnCRDepositCoin) {
				_crList.push_back(tx);
			} else if (type == Transaction::registerProducer ||
			           type == Transaction::cancelProducer ||
			           type == Transaction::updateProducer ||
			           type == Transaction::returnDepositCoin ||
			           type == Transaction::activateProducer) {
				_producerList.push_back(tx);
			}
			Unlock();
			SubWallet::onTxAdded(tx);
		}

		void MainchainSubWallet::onTxDeleted(const uint256 &hash, bool notifyUser, bool recommendRescan) {
			bool found = false;
			Lock();
			for (std::vector<TransactionPtr>::iterator it = _crList.begin(); it != _crList.end();) {
				if ((*it)->GetHash().GetHex() == hash.GetHex()) {
					it = _crList.erase(it);
					found = true;
					break;
				} else {
					++it;
				}
			}

			for (std::vector<TransactionPtr>::iterator it = _producerList.begin(); !found && it != _producerList.end();) {
				if ((*it)->GetHash().GetHex() == hash.GetHex()) {
					it = _producerList.erase(it);
					break;
				} else {
					++it;
				}
			}

			Unlock();

			SubWallet::onTxDeleted(hash, notifyUser, recommendRescan);
		}

	}
}
