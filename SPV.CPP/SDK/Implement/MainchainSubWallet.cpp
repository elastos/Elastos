// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <vector>
#include <map>
#include <boost/scoped_ptr.hpp>

#include "MainchainSubWallet.h"
#include "KeyStore/CoinInfo.h"
#include "Payload/PayloadTransferCrossChainAsset.h"
#include "ELACoreExt/ELATxOutput.h"
#include "Utils.h"
#include "ParamChecker.h"
#include "Transaction/MainchainTransactionChecker.h"
#include "Transaction/MainchainTransactionCompleter.h"

namespace Elastos {
	namespace ElaWallet {

		MainchainSubWallet::MainchainSubWallet(const CoinInfo &info, const MasterPubKeyPtr &masterPubKey,
											   const ChainParams &chainParams, const PluginTypes &pluginTypes,
											   MasterWallet *parent) :
				SubWallet(info, masterPubKey, chainParams, pluginTypes, parent) {

		}

		MainchainSubWallet::~MainchainSubWallet() {

		}

		nlohmann::json MainchainSubWallet::CreateDepositTransaction(const std::string &fromAddress,
																	const std::string &toAddress,
																	const uint64_t amount,
																	const nlohmann::json &sidechainAccounts,
																	const nlohmann::json &sidechainAmounts,
																	const nlohmann::json &sidechainIndices,
																	const std::string &memo,
																	const std::string &remark) {
			boost::scoped_ptr<TxParam> txParam(TxParamFactory::createTxParam(Mainchain, fromAddress, toAddress, amount,
																			 _info.getMinFee(), memo, remark));
			txParam->setAssetId(Key::getSystemAssetId());

			ParamChecker::checkJsonArray(sidechainAccounts, 1, "Side chain accounts");
			ParamChecker::checkJsonArray(sidechainAmounts, 1, "Side chain amounts");
			ParamChecker::checkJsonArray(sidechainIndices, 1, "Side chain indices");

			std::vector<std::string> accounts = sidechainAccounts.get<std::vector<std::string >>();
			std::vector<uint64_t> amounts = sidechainAmounts.get<std::vector<uint64_t >>();
			std::vector<uint64_t> indexs = sidechainIndices.get<std::vector<uint64_t >>();

			ParamChecker::checkCondition(accounts.size() != amounts.size() || accounts.size() != indexs.size(),
										 Error::DepositParam, "Invalid deposit parameters of side chain");

			DepositTxParam *withdrawTxParam = static_cast<DepositTxParam *>(txParam.get());
			withdrawTxParam->setSidechainDatas(accounts, indexs, amounts);

			//todo read main chain address from config
			std::string mainchainAddress;
			withdrawTxParam->setSidechainAddress(mainchainAddress);

			TransactionPtr transaction = createTransaction(txParam.get());
			ParamChecker::checkCondition(transaction == nullptr, Error::CreateTransaction, "Create tx error");

			return transaction->toJson();
		}

		boost::shared_ptr<Transaction>
		MainchainSubWallet::createTransaction(TxParam *param) const {
			TransactionPtr ptr = nullptr;
			DepositTxParam *depositTxParam = dynamic_cast<DepositTxParam *>(param);
			RegisterProducerTxParam *registerProducerTxParam = dynamic_cast<RegisterProducerTxParam *>(param);
			CancelProducerTxParam *cancelProducerTxParam = dynamic_cast<CancelProducerTxParam *>(param);
			VoteProducerTxParam *voteProducerTxParam = dynamic_cast<VoteProducerTxParam *>(param);

			if (depositTxParam == nullptr && registerProducerTxParam == nullptr && cancelProducerTxParam == nullptr &&
				voteProducerTxParam == nullptr) {
				ptr = SubWallet::createTransaction(param);
			} else {
				ptr = _walletManager->getWallet()->
						createTransaction(param->getFromAddress(), param->getFee(), param->getAmount(),
										  param->getToAddress(), param->getRemark(), param->getMemo());

				if (!ptr) return nullptr;

				if (depositTxParam != nullptr) {
					ptr->setTransactionType(ELATransaction::TransferCrossChainAsset);
					const std::vector<TransactionOutput *> &outList = ptr->getOutputs();
					std::for_each(outList.begin(), outList.end(),
								  [&param](TransactionOutput *output) {
									  ((ELATxOutput *) output->getRaw())->assetId = param->getAssetId();
								  });

					PayloadTransferCrossChainAsset *payloadTransferCrossChainAsset =
							static_cast<PayloadTransferCrossChainAsset *>(ptr->getPayload());
					payloadTransferCrossChainAsset->setCrossChainData(depositTxParam->getCrossChainAddress(),
																	  depositTxParam->getCrossChainOutputIndexs(),
																	  depositTxParam->getCrosschainAmouts());
				} else if (registerProducerTxParam != nullptr) {
					ptr->setTransactionType(ELATransaction::RegisterProducer);
					PayloadRegisterProducer *payloadRegisterProducer = static_cast<PayloadRegisterProducer *>(ptr->getPayload());
					*payloadRegisterProducer = registerProducerTxParam->GetPayload();
				} else if (cancelProducerTxParam != nullptr) {
					ptr->setTransactionType(ELATransaction::CancelProducer);
					PayloadCancelProducer *cancelProducer = static_cast<PayloadCancelProducer *>(ptr->getPayload());
					*cancelProducer = cancelProducerTxParam->GetPayload();
				} else if (voteProducerTxParam != nullptr) {
					PayloadVoteProducer *voteProducer = static_cast<PayloadVoteProducer *>(ptr->getPayload());
					*voteProducer = voteProducerTxParam->GetPayload();
				}
			}
			return ptr;
		}

		void MainchainSubWallet::verifyRawTransaction(const TransactionPtr &transaction) {
			if (transaction->getTransactionType() == ELATransaction::TransferCrossChainAsset) {
				MainchainTransactionChecker checker(transaction, _walletManager->getWallet());
				checker.Check();
			} else
				SubWallet::verifyRawTransaction(transaction);
		}

		TransactionPtr
		MainchainSubWallet::completeTransaction(const TransactionPtr &transaction, uint64_t actualFee) {
			if (transaction->getTransactionType() == ELATransaction::TransferCrossChainAsset) {
				MainchainTransactionCompleter completer(transaction, _walletManager->getWallet());
				return completer.Complete(actualFee);
			} else
				return SubWallet::completeTransaction(transaction, actualFee);
		}

		nlohmann::json MainchainSubWallet::GetBasicInfo() const {
			nlohmann::json j;
			j["Type"] = "Mainchain";
			j["Account"] = _subAccount->GetBasicInfo();
			return j;
		}

		nlohmann::json MainchainSubWallet::CreateRegisterProducerTransaction(const std::string &fromAddress,
																			 const std::string &toAddress,
																			 const std::string &publicKey,
																			 const std::string &nickName,
																			 const std::string &url,
																			 const std::string &location) {
			RegisterProducerTxParam txParam;
			txParam.setFromAddress(fromAddress);
			txParam.setToAddress(toAddress);

			PayloadRegisterProducer payload;
			payload.SetPublicKey(publicKey);
			payload.SetNickName(nickName);
			payload.SetUrl(url);
			payload.SetLocation(location);
			txParam.SetPayload(payload);

			TransactionPtr transaction = createTransaction(&txParam);
			ParamChecker::checkCondition(transaction == nullptr, Error::CreateTransaction, "Create tx error");

			return transaction->toJson();
		}

		nlohmann::json MainchainSubWallet::CreateCancelProducerTransaction(const std::string &fromAddress,
																		   const std::string &toAddress,
																		   const std::string &publicKey) {
			CancelProducerTxParam txParam;
			txParam.setFromAddress(fromAddress);
			txParam.setToAddress(toAddress);

			PayloadCancelProducer payload;
			payload.SetPublicKey(publicKey);
			txParam.SetPayload(payload);

			TransactionPtr transaction = createTransaction(&txParam);
			ParamChecker::checkCondition(transaction == nullptr, Error::CreateTransaction, "Create tx error");

			return transaction->toJson();
		}

		nlohmann::json
		MainchainSubWallet::CreateVoteProducerTransaction(const std::string &fromAddress,
														  const std::string &toAddress,
														  uint64_t stake, const nlohmann::json &pubicKeys) {
			VoteProducerTxParam txParam;
			txParam.setFromAddress(fromAddress);
			txParam.setToAddress(toAddress);

			PayloadVoteProducer payload;
			payload.SetVoter(_subAccount->GetMainAccountPublicKey());
			payload.SetStake(stake);
			std::vector<std::string> keys;
			for (nlohmann::json::const_iterator it = pubicKeys.cbegin(); it != pubicKeys.cend(); ++it) {
				keys.push_back(it->get<std::string>());
			}
			payload.SetPublicKeys(keys);
			txParam.SetPayload(payload);

			TransactionPtr transaction = createTransaction(&txParam);
			ParamChecker::checkCondition(transaction == nullptr, Error::CreateTransaction, "Create tx error");

			return transaction->toJson();
		}
	}
}