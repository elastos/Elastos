// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <vector>
#include <map>
#include <boost/scoped_ptr.hpp>

#include "SidechainSubWallet.h"
#include "ELACoreExt/ELATxOutput.h"
#include "ELACoreExt/Payload/PayloadTransferCrossChainAsset.h"
#include "ParamChecker.h"
#include "Transaction/SidechainTransactionChecker.h"
#include "Transaction/SidechainTransactionCompleter.h"

namespace Elastos {
	namespace ElaWallet {

		SidechainSubWallet::SidechainSubWallet(const CoinInfo &info, const ChainParams &chainParams,
											   const std::string &payPassword, MasterWallet *parent) :
				SubWallet(info, chainParams, payPassword, parent) {

		}

		SidechainSubWallet::~SidechainSubWallet() {

		}

		nlohmann::json SidechainSubWallet::CreateWithdrawTransaction(const std::string &fromAddress,
																   const std::string &toAddress,
																   const uint64_t amount,
																   const nlohmann::json &mainchainAccounts,
																   const nlohmann::json &mainchainAmounts,
																   const nlohmann::json &mainchainIndexs,
																   uint64_t fee,
																   const std::string &memo) {
			boost::scoped_ptr<TxParam> txParam(
					TxParamFactory::createTxParam(Sidechain, fromAddress, toAddress, amount, fee, memo));

			ParamChecker::checkJsonArrayNotEmpty(mainchainAccounts);
			ParamChecker::checkJsonArrayNotEmpty(mainchainAmounts);
			ParamChecker::checkJsonArrayNotEmpty(mainchainIndexs);

			std::vector<std::string> accounts = mainchainAccounts.get<std::vector<std::string>>();
			std::vector<uint64_t> amounts = mainchainAmounts.get<std::vector<uint64_t>>();
			std::vector<uint64_t> indexs = mainchainIndexs.get<std::vector<uint64_t>>();
			assert(accounts.size() == amounts.size());
			assert(accounts.size() == indexs.size());

			WithdrawTxParam *withdrawTxParam = dynamic_cast<WithdrawTxParam *>(txParam.get());
			assert(withdrawTxParam != nullptr);
			withdrawTxParam->setMainchainDatas(accounts, indexs, amounts);

			//todo read main chain address from config
			std::string mainchainAddress;
			withdrawTxParam->setMainchainAddress(mainchainAddress);

			TransactionPtr transaction = createTransaction(txParam.get());
			if (transaction == nullptr) {
				throw std::logic_error("Create transaction error.");
			}
			return transaction->toJson();
		}

		boost::shared_ptr<Transaction>
		SidechainSubWallet::createTransaction(TxParam *param) const {
			WithdrawTxParam *withdrawTxParam = dynamic_cast<WithdrawTxParam *>(param);
			assert(withdrawTxParam != nullptr);

			TransactionPtr ptr = nullptr;
			if (param->getFee() > 0 || param->getFromAddress().empty() == true) {
				ptr = _walletManager->getWallet()->createTransaction(param->getFromAddress(), param->getFee(),
																	 param->getAmount(), param->getToAddress());
			} else {
				Address address(param->getToAddress());
				ptr = _walletManager->getWallet()->createTransaction(param->getAmount(), address);
			}
			if (!ptr) return nullptr;

			ptr->setTransactionType(ELATransaction::TransferCrossChainAsset);
			SharedWrapperList<TransactionOutput, BRTxOutput *> outList = ptr->getOutputs();
			std::for_each(outList.begin(), outList.end(),
						  [&param](const SharedWrapperList<TransactionOutput, BRTxOutput *>::TPtr &output) {
							  ((ELATxOutput *) output->getRaw())->assetId = param->getAssetId();
						  });

			PayloadTransferCrossChainAsset *payloadTransferCrossChainAsset =
					static_cast<PayloadTransferCrossChainAsset *>(ptr->getPayload().get());
			payloadTransferCrossChainAsset->setCrossChainData(withdrawTxParam->getCrossChainAddress(),
															  withdrawTxParam->getCrossChainOutputIndexs(),
															  withdrawTxParam->getCrosschainAmouts());

			return ptr;
		}

		void SidechainSubWallet::verifyRawTransaction(const TransactionPtr &transaction) {
			SidechainTransactionChecker checker(transaction);
			checker.Check();
		}

		void SidechainSubWallet::completeTransaction(const TransactionPtr &transaction, uint64_t actualFee) {
			SidechainTransactionCompleter completer(transaction, _walletManager->getWallet());
			completer.Complete(actualFee);
		}
	}
}