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
																	const nlohmann::json &sidechainIndices,
																	const std::string &memo,
																	const std::string &remark) {
			boost::scoped_ptr<TxParam> txParam(TxParamFactory::createTxParam(Mainchain, fromAddress, toAddress, amount,
																			 _info.getMinFee(), memo, remark));
			txParam->setAssetId(Key::getSystemAssetId());

			ParamChecker::checkJsonArrayNotEmpty(sidechainAccounts);
			ParamChecker::checkJsonArrayNotEmpty(sidechainAmounts);
			ParamChecker::checkJsonArrayNotEmpty(sidechainIndices);

			std::vector<std::string> accounts = sidechainAccounts.get<std::vector<std::string>>();
			std::vector<uint64_t> amounts = sidechainAmounts.get<std::vector<uint64_t>>();
			std::vector<uint64_t> indexs = sidechainIndices.get<std::vector<uint64_t >>();
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
				ptr = _walletManager->getWallet()->
						createTransaction(param->getFromAddress(), param->getFee(), param->getAmount(),
										  param->getToAddress(), param->getRemark(), param->getMemo());

				if (!ptr) return nullptr;

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

		TransactionPtr MainchainSubWallet::completeTransaction(const TransactionPtr &transaction, uint64_t actualFee) {
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
	}
}