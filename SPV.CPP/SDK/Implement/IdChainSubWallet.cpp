// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <boost/scoped_ptr.hpp>

#include "ELACoreExt/ELATxOutput.h"
#include "ELACoreExt/Payload/PayloadRegisterIdentification.h"
#include "ELACoreExt/ELATransaction.h"

#include "Utils.h"
#include "IdChainSubWallet.h"
#include "Utils.h"
#include "SubWalletCallback.h"
#include "Transaction/IdchainTransactionChecker.h"
#include "Transaction/IdchainTransactionCompleter.h"

namespace Elastos {
	namespace ElaWallet {

		IdChainSubWallet::IdChainSubWallet(const CoinInfo &info, const ChainParams &chainParams,
										   const std::string &payPassword, const PluginTypes &pluginTypes,
										   MasterWallet *parent) :
				SidechainSubWallet(info, chainParams, payPassword, pluginTypes, parent) {

		}

		IdChainSubWallet::~IdChainSubWallet() {

		}

		nlohmann::json
		IdChainSubWallet::CreateIdTransaction(const std::string &fromAddress, const std::string &toAddress,
											  const uint64_t amount, const nlohmann::json &payloadJson,
											  const nlohmann::json &programJson, uint64_t fee,
											  const std::string &memo, const std::string &remark) {
			boost::scoped_ptr<TxParam> txParam(
					TxParamFactory::createTxParam(Idchain, fromAddress, toAddress, amount, fee, memo, remark));

			TransactionPtr transaction = createTransaction(txParam.get());
			if (transaction == nullptr) {
				throw std::logic_error("Create transaction error.");
			}
			PayloadRegisterIdentification *payloadIdChain = static_cast<PayloadRegisterIdentification *>(transaction->getPayload());
			payloadIdChain->fromJson(payloadJson);

			Program *newProgram = new Program();
			newProgram->fromJson(programJson);
			transaction->addProgram(newProgram);

			TransactionOutput *transactionOutput = new TransactionOutput();
			transactionOutput->setAddress(payloadIdChain->getId());
			transactionOutput->setAmount(0);
			transactionOutput->setAssetId(txParam->getAssetId());
			UInt168 programHash = UINT168_ZERO;
			bool ret = Utils::UInt168FromAddress(programHash, payloadIdChain->getId());
			if (!ret) {
				throw std::logic_error("payloadIdChain ID is error!");
			}
			transactionOutput->setProgramHash(programHash);
			transaction->addOutput(transactionOutput);

			return transaction->toJson();
		}

		boost::shared_ptr<Transaction>
		IdChainSubWallet::createTransaction(TxParam *param) const {
			IdTxParam *idTxParam = dynamic_cast<IdTxParam *>(param);

			if (idTxParam != nullptr) {
				//todo create transaction without to address

				TransactionPtr ptr = _walletManager->getWallet()->
						createTransaction(param->getFromAddress(), param->getFee(), param->getAmount(),
										  param->getToAddress(), param->getRemark(), param->getMemo());
				if (!ptr) return nullptr;
				ptr->setTransactionType(ELATransaction::RegisterIdentification);

				const std::vector<TransactionOutput *> &outList = ptr->getOutputs();
				for (size_t i = 0; i < outList.size(); ++i) {
					((ELATxOutput *) outList[i]->getRaw())->assetId = param->getAssetId();
				}

				return ptr;
			} else {
				return SidechainSubWallet::createTransaction(param);
			}
		}

		void IdChainSubWallet::verifyRawTransaction(const TransactionPtr &transaction) {
			if (transaction->getTransactionType() == ELATransaction::RegisterIdentification) {
				IdchainTransactionChecker checker(transaction, _walletManager->getWallet());
				checker.Check();
			} else
				SidechainSubWallet::verifyRawTransaction(transaction);
		}

		TransactionPtr IdChainSubWallet::completeTransaction(const TransactionPtr &transaction, uint64_t actualFee) {
			if (transaction->getTransactionType() == ELATransaction::RegisterIdentification) {
				IdchainTransactionCompleter completer(transaction, _walletManager->getWallet());
				return completer.Complete(actualFee);
			}
			return SidechainSubWallet::completeTransaction(transaction, actualFee);
		}

		void IdChainSubWallet::onTxAdded(const TransactionPtr &transaction) {
			std::for_each(_callbacks.begin(), _callbacks.end(),
						  [transaction](ISubWalletCallback *callback) {

							  if (transaction->getTransactionType() != ELATransaction::RegisterIdentification)
								  return;

							  const PayloadRegisterIdentification *payload = static_cast<const PayloadRegisterIdentification *>(
									  transaction->getPayload());
							  callback->OnTransactionStatusChanged(std::string((char *) transaction->getHash().u8, 32),
																   SubWalletCallback::convertToString(
																		   SubWalletCallback::Added),
																   payload->toJson(), transaction->getBlockHeight());
						  });
		}

		void IdChainSubWallet::onTxUpdated(const std::string &hash, uint32_t blockHeight, uint32_t timeStamp) {
			std::for_each(_callbacks.begin(), _callbacks.end(),
						  [&hash, blockHeight, timeStamp, this](ISubWalletCallback *callback) {

							  TransactionPtr transaction = _walletManager->getWallet()->transactionForHash(
									  Utils::UInt256FromString(hash));
							  if (transaction == nullptr ||
								  transaction->getTransactionType() != ELATransaction::RegisterIdentification)
								  return;

							  const PayloadRegisterIdentification *payload = static_cast<const PayloadRegisterIdentification *>(
									  transaction->getPayload());
							  callback->OnTransactionStatusChanged(hash, SubWalletCallback::convertToString(
									  SubWalletCallback::Updated), payload->toJson(), blockHeight);
						  });
		}

		void IdChainSubWallet::onTxDeleted(const std::string &hash, bool notifyUser, bool recommendRescan) {
			std::for_each(_callbacks.begin(), _callbacks.end(),
						  [&hash, notifyUser, recommendRescan, this](ISubWalletCallback *callback) {
							  TransactionPtr transaction = _walletManager->getWallet()->transactionForHash(
									  Utils::UInt256FromString(hash));
							  if (transaction == nullptr ||
								  transaction->getTransactionType() != ELATransaction::RegisterIdentification)
								  return;

							  const PayloadRegisterIdentification *payload = static_cast<const PayloadRegisterIdentification *>(
									  transaction->getPayload());
							  callback->OnTransactionStatusChanged(hash, SubWalletCallback::convertToString(
									  SubWalletCallback::Deleted), payload->toJson(), 0);
						  });
		}

	}
}