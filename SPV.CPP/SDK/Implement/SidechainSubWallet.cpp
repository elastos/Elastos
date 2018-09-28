// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <vector>
#include <map>
#include <boost/scoped_ptr.hpp>

#include "SidechainSubWallet.h"
#include "ELACoreExt/Payload/PayloadTransferCrossChainAsset.h"
#include "ParamChecker.h"
#include "Transaction/SidechainTransactionChecker.h"
#include "Transaction/SidechainTransactionCompleter.h"

namespace Elastos {
	namespace ElaWallet {

		SidechainSubWallet::SidechainSubWallet(const CoinInfo &info, const MasterPubKeyPtr &masterPubKey,
											   const ChainParams &chainParams, const PluginTypes &pluginTypes,
											   MasterWallet *parent) :
			SubWallet(info, masterPubKey, chainParams, pluginTypes, parent) {

		}

		SidechainSubWallet::~SidechainSubWallet() {

		}

		nlohmann::json SidechainSubWallet::CreateWithdrawTransaction(const std::string &fromAddress,
																	 const uint64_t amount,
																	 const nlohmann::json &mainchainAccounts,
																	 const nlohmann::json &mainchainAmounts,
																	 const nlohmann::json &mainchainIndexs,
																	 const std::string &memo,
																	 const std::string &remark) {
			boost::scoped_ptr<TxParam> txParam(TxParamFactory::createTxParam(Sidechain, fromAddress, ELA_SIDECHAIN_DESTROY_ADDR, amount,
																			 _info.getMinFee(), memo, remark));

			ParamChecker::checkJsonArray(mainchainAccounts, 1, "Main chain accounts");
			ParamChecker::checkJsonArray(mainchainAmounts, 1, "Main chain amounts");
			ParamChecker::checkJsonArray(mainchainIndexs, 1, "Main chain indexs");

			std::vector<std::string> accounts = mainchainAccounts.get<std::vector<std::string>>();
			std::vector<uint64_t> amounts = mainchainAmounts.get<std::vector<uint64_t>>();
			std::vector<uint64_t> indexs = mainchainIndexs.get<std::vector<uint64_t>>();

			ParamChecker::checkCondition(accounts.size() != amounts.size() || accounts.size() != indexs.size(),
										 Error::WithdrawParam, "Invalid withdraw parameters of main chain");

			WithdrawTxParam *withdrawTxParam = dynamic_cast<WithdrawTxParam *>(txParam.get());
			withdrawTxParam->setMainchainDatas(accounts, indexs, amounts);

			//todo read main chain address from config
			std::string mainchainAddress;
			withdrawTxParam->setMainchainAddress(mainchainAddress);

			TransactionPtr transaction = createTransaction(txParam.get());
			ParamChecker::checkCondition(transaction == nullptr, Error::CreateTransaction, "Create withdraw tx");
			return transaction->toJson();
		}

		std::string SidechainSubWallet::GetGenesisAddress() const {
			return _info.getGenesisAddress();
		}

		boost::shared_ptr<Transaction>
		SidechainSubWallet::createTransaction(TxParam *param) const {
			WithdrawTxParam *withdrawTxParam = dynamic_cast<WithdrawTxParam *>(param);
			if (withdrawTxParam != nullptr) {
				TransactionPtr ptr = _walletManager->getWallet()->
					createTransaction(param->getFromAddress(), param->getFee(), param->getAmount(),
									  param->getToAddress(), param->getRemark(), param->getMemo());
				if (!ptr) return nullptr;

				ptr->setTransactionType(Transaction::TransferCrossChainAsset);
				const std::vector<TransactionOutput> &outList = ptr->getOutputs();

				std::for_each(outList.begin(), outList.end(),
							  [&param](const TransactionOutput &output) {
								  const_cast<TransactionOutput &>(output).setAssetId(param->getAssetId());
							  });

				PayloadTransferCrossChainAsset *payloadTransferCrossChainAsset =
					static_cast<PayloadTransferCrossChainAsset *>(ptr->getPayload());
				payloadTransferCrossChainAsset->setCrossChainData(withdrawTxParam->getCrossChainAddress(),
																  withdrawTxParam->getCrossChainOutputIndexs(),
																  withdrawTxParam->getCrosschainAmouts());
				return ptr;
			} else
				return SubWallet::createTransaction(param);
		}

		void SidechainSubWallet::verifyRawTransaction(const TransactionPtr &transaction) {
			if (transaction->getTransactionType() == Transaction::TransferCrossChainAsset) {
				SidechainTransactionChecker checker(transaction, _walletManager->getWallet());
				checker.Check();
			} else
				SubWallet::verifyRawTransaction(transaction);
		}

		TransactionPtr SidechainSubWallet::completeTransaction(const TransactionPtr &transaction, uint64_t actualFee) {
			if (transaction->getTransactionType() == Transaction::TransferCrossChainAsset) {
				SidechainTransactionCompleter completer(transaction, _walletManager->getWallet());
				return completer.Complete(actualFee);
			} else
				return SubWallet::completeTransaction(transaction, actualFee);
		}

		nlohmann::json SidechainSubWallet::GetBasicInfo() const {
			nlohmann::json j;
			j["Type"] = "Sidechain";
			j["Account"] = _subAccount->GetBasicInfo();
			return j;
		}
	}
}