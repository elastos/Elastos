// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <vector>
#include <map>
#include <boost/scoped_ptr.hpp>

#include "MainchainSubWallet.h"
#include "KeyStore/CoinInfo.h"
#include "Payload/PayloadTransferCrossChainAsset.h"
#include "ELACoreExt/ELABRTxOutput.h"

namespace Elastos {
	namespace SDK {

		MainchainSubWallet::MainchainSubWallet(const CoinInfo &info, const ChainParams &chainParams,
											   const std::string &payPassword, MasterWallet *parent) :
				SubWallet(info, chainParams, payPassword, parent) {

		}

		MainchainSubWallet::~MainchainSubWallet() {

		}

		std::string MainchainSubWallet::SendDepositTransaction(const std::string &fromAddress,
															   const nlohmann::json &sidechainAccounts,
															   const nlohmann::json &sidechainAmounts,
															   const nlohmann::json &sidechainIndexs, uint64_t fee,
															   const std::string &payPassword,
															   const std::string &memo) {
			boost::scoped_ptr<TxParam> txParam(
					TxParamFactory::createTxParam(Mainchain, fromAddress, "", 0, fee, memo));

			std::vector<std::string> accounts = sidechainAccounts.get<std::vector<std::string>>();
			std::vector<uint64_t> amounts = sidechainAmounts.get<std::vector<uint64_t>>();
			std::vector<uint64_t> indexs = sidechainIndexs.get<std::vector<uint64_t >>();
			assert(accounts.size() == amounts.size() == indexs.size());

			DepositTxParam *withdrawTxParam = dynamic_cast<DepositTxParam *>(txParam.get());
			assert(withdrawTxParam != nullptr);
			withdrawTxParam->setSidechainDatas(accounts, indexs, amounts);

			//todo read main chain address from config
			std::string mainchainAddress;
			withdrawTxParam->setSidechainAddress(mainchainAddress);

			TransactionPtr transaction = createTransaction(txParam.get());
			return sendTransactionInternal(transaction, payPassword);
		}

		boost::shared_ptr<Transaction> MainchainSubWallet::createTransaction(TxParam *param) const {
			DepositTxParam *depositTxParam = dynamic_cast<DepositTxParam *>(param);
			assert(depositTxParam != nullptr);

			TransactionPtr ptr = nullptr;
			if (param->getFee() > 0 || param->getFromAddress().empty() == true) {
				ptr = _walletManager->getWallet()->createTransaction(param->getFromAddress(), param->getFee(),
				                                                     param->getAmount(), param->getToAddress());
			} else {
				Address address(param->getToAddress());
				ptr = _walletManager->getWallet()->createTransaction(param->getAmount(), address);
			}

			if (!ptr) return nullptr;

			ptr->setTransactionType(Transaction::TransferCrossChainAsset);
			SharedWrapperList<TransactionOutput, BRTxOutput *> outList = ptr->getOutputs();
			std::for_each(outList.begin(), outList.end(),
						  [&param](const SharedWrapperList<TransactionOutput, BRTxOutput *>::TPtr &output) {
							  ((ELABRTxOutput *) output->getRaw())->assetId = param->getAssetId();
						  });

			PayloadTransferCrossChainAsset *payloadTransferCrossChainAsset =
					static_cast<PayloadTransferCrossChainAsset *>(ptr->getPayload().get());
			payloadTransferCrossChainAsset->setCrossChainData(depositTxParam->getCrossChainAddress(),
			                                                  depositTxParam->getCrossChainOutputIndexs(),
			                                                  depositTxParam->getCrosschainAmouts());

			return ptr;
		}

		bool MainchainSubWallet::verifyRawTransaction(const TransactionPtr &transaction) {
			//todo different verify from base class
			if (transaction->getTransactionType() != Transaction::TransferCrossChainAsset) {
				return false;
			}

			return SubWallet::verifyRawTransaction(transaction);
		}

		bool MainchainSubWallet::checkTransactionPayload(const TransactionPtr &transaction) {
			const PayloadPtr payloadPtr = transaction->getPayload();

			bool isValid = SubWallet::checkTransactionPayload(transaction);

			if (isValid) {
				PayloadTransferCrossChainAsset *payloadTransferCrossChainAsset =
						static_cast<PayloadTransferCrossChainAsset *>(payloadPtr.get());

				std::vector<uint64_t> outputIndex = payloadTransferCrossChainAsset->getOutputIndex();
				const SharedWrapperList<TransactionOutput, BRTxOutput *> outputs = transaction->getOutputs();
				for (size_t i = 0; i < outputIndex.size(); ++i) {
					if (outputIndex[i] > outputs.size() - 1) {
						isValid = false;
						break;
					}
				}

			}
			return isValid;
		}

		bool MainchainSubWallet::completeTransaction(const TransactionPtr &transaction) {
			//todo different complete from base class
			return SubWallet::completeTransaction(transaction);
		}
	}
}