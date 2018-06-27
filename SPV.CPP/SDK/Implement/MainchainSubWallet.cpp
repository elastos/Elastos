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

		MainchainSubWallet::MainchainSubWallet(const CoinInfo &info, const ChainParams &chainParams,
											   const std::string &payPassword, const PluginTypes &pluginTypes,
											   MasterWallet *parent) :
				SubWallet(info, chainParams, payPassword, pluginTypes, parent) {

		}

		MainchainSubWallet::~MainchainSubWallet() {

		}

		nlohmann::json MainchainSubWallet::CreateDepositTransaction(const std::string &fromAddress,
																	const std::string &toAddress,
																	const uint64_t amount,
																	const nlohmann::json &sidechainAccounts,
																	const nlohmann::json &sidechainAmounts,
																	const nlohmann::json &sidechainIndexs,
																	uint64_t fee,
																	const std::string &memo) {
			boost::scoped_ptr<TxParam> txParam(
					TxParamFactory::createTxParam(Mainchain, fromAddress, toAddress, amount, fee, memo));
			txParam->setAssetId(Key::getSystemAssetId());

			ParamChecker::checkJsonArrayNotEmpty(sidechainAccounts);
			ParamChecker::checkJsonArrayNotEmpty(sidechainAmounts);
			ParamChecker::checkJsonArrayNotEmpty(sidechainIndexs);

			std::vector<std::string> accounts = sidechainAccounts.get<std::vector<std::string>>();
			std::vector<uint64_t> amounts = sidechainAmounts.get<std::vector<uint64_t>>();
			std::vector<uint64_t> indexs = sidechainIndexs.get<std::vector<uint64_t >>();
			if (accounts.size() != amounts.size() || accounts.size() != indexs.size())
				throw std::invalid_argument("Length of sidechain accounts amounts and indices should same.");

			DepositTxParam *withdrawTxParam = static_cast<DepositTxParam *>(txParam.get());
			withdrawTxParam->setSidechainDatas(accounts, indexs, amounts);

			//todo read main chain address from config
			std::string mainchainAddress;
			withdrawTxParam->setSidechainAddress(mainchainAddress);

			TransactionPtr transaction = createTransaction(txParam.get());
			if (transaction == nullptr) {
				throw std::logic_error("Create transaction error.");
			}
			return transaction->toJson();
		}

		boost::shared_ptr<Transaction>
		MainchainSubWallet::createTransaction(TxParam *param) const {
			TransactionPtr ptr = nullptr;
			DepositTxParam *depositTxParam = dynamic_cast<DepositTxParam *>(param);
			if (depositTxParam == nullptr) {
				ptr = SubWallet::createTransaction(param);
			} else {
				ptr = _walletManager->getWallet()->createTransaction(param->getFromAddress(), param->getFee(),
																	 param->getAmount(), param->getToAddress());

				if (!ptr) return nullptr;

				ptr->setTransactionType(ELATransaction::TransferCrossChainAsset);
				SharedWrapperList<TransactionOutput, BRTxOutput *> outList = ptr->getOutputs();
				std::for_each(outList.begin(), outList.end(),
							  [&param](const SharedWrapperList<TransactionOutput, BRTxOutput *>::TPtr &output) {
								  ((ELATxOutput *) output->getRaw())->assetId = param->getAssetId();
							  });

				PayloadTransferCrossChainAsset *payloadTransferCrossChainAsset =
						static_cast<PayloadTransferCrossChainAsset *>(ptr->getPayload().get());
				payloadTransferCrossChainAsset->setCrossChainData(depositTxParam->getCrossChainAddress(),
																  depositTxParam->getCrossChainOutputIndexs(),
																  depositTxParam->getCrosschainAmouts());
			}
			return ptr;
		}

		void MainchainSubWallet::verifyRawTransaction(const TransactionPtr &transaction) {
			MainchainTransactionChecker checker(transaction, _walletManager->getWallet());
			checker.Check();
		}

		TransactionPtr MainchainSubWallet::completeTransaction(const TransactionPtr &transaction, uint64_t actualFee) {
			MainchainTransactionCompleter completer(transaction, _walletManager->getWallet());
			return completer.Complete(actualFee);
		}
	}
}