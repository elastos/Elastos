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

namespace Elastos {
	namespace SDK {

		IdChainSubWallet::IdChainSubWallet(const CoinInfo &info, const ChainParams &chainParams,
										   const std::string &payPassword, MasterWallet *parent) :
				SubWallet(info, chainParams, payPassword, parent) {

		}

		IdChainSubWallet::~IdChainSubWallet() {

		}

		nlohmann::json
		IdChainSubWallet::SendIdTransaction(const std::string &fromAddress, const std::string &toAddress,
											const uint64_t amount, const nlohmann::json &payloadJson,
											const nlohmann::json &programJson, uint64_t fee,
											const std::string &payPassword, const std::string &memo) {
			boost::scoped_ptr<TxParam> txParam(
					TxParamFactory::createTxParam(Idchain, fromAddress, toAddress, amount, fee, memo));

			TransactionPtr transaction = createTransaction(txParam.get(), payPassword);
			if (transaction == nullptr) {
				throw std::logic_error("Create transaction error.");
			}
			PayloadRegisterIdentification *payloadIdChain = static_cast<PayloadRegisterIdentification *>(transaction->getPayload().get());
			payloadIdChain->fromJson(payloadJson);

			ProgramPtr newProgram(new Program());
			newProgram->fromJson(programJson);
			transaction->addProgram(newProgram);

			TransactionOutput *transactionOutput = new TransactionOutput();
			transactionOutput->setAddress(payloadIdChain->getId());
			transactionOutput->setAmount(0);
			transactionOutput->setAssetId(txParam->getAssetId());
			transactionOutput->setProgramHash(Utils::UInt168FromString(payloadIdChain->getId()));
			transaction->addOutput(transactionOutput);

			return sendTransactionInternal(transaction, payPassword);
		}

		boost::shared_ptr<Transaction>
		IdChainSubWallet::createTransaction(TxParam *param, const std::string &payPassword) const {
			IdTxParam *idTxParam = dynamic_cast<IdTxParam *>(param);
			assert(idTxParam != nullptr);

			//todo create transaction without to address

			TransactionPtr ptr = nullptr;
			if (param->getFee() > 0 || param->getFromAddress().empty() == true) {
				ptr = _walletManager->getWallet()->createTransaction(param->getFromAddress(), param->getFee(),
																	 param->getAmount(), param->getToAddress(),
																	 payPassword);
			} else {
				Address address(param->getToAddress());
				ptr = _walletManager->getWallet()->createTransaction(param->getAmount(), address, payPassword);
			}

			if (!ptr) return nullptr;
			ptr->setTransactionType(ELATransaction::RegisterIdentification);

			SharedWrapperList<TransactionOutput, BRTxOutput *> outList = ptr->getOutputs();
			std::for_each(outList.begin(), outList.end(),
						  [&param](const SharedWrapperList<TransactionOutput, BRTxOutput *>::TPtr &output) {
							  ((ELATxOutput *) output->getRaw())->assetId = param->getAssetId();
						  });

			return ptr;
		}

		void IdChainSubWallet::verifyRawTransaction(const TransactionPtr &transaction) {
			//todo different verify from base class
			SubWallet::verifyRawTransaction(transaction);
		}

		bool IdChainSubWallet::checkTransactionOutput(const TransactionPtr &transaction) {

			if (transaction->getTransactionType() != ELATransaction::RegisterIdentification) {
				return false;
			}

			SubWallet::checkTransactionOutput(transaction);

			const SharedWrapperList<TransactionOutput, BRTxOutput *> outputs = transaction->getOutputs();
			size_t size = outputs.size();
			if (size < 1) {
				return false;
			}
			for (size_t i = 0; i < size; ++i) {
				TransactionOutputPtr output = outputs[i];
				if (output->getAddress().empty()) {
					return false;
				}

				if (output->getAmount() != 0) {
					return false;
				}

			}
			return true;
		}

		void IdChainSubWallet::completeTransaction(const TransactionPtr &transaction) {
			//todo different complete from base class
			SubWallet::completeTransaction(transaction);
		}

		void IdChainSubWallet::onTxAdded(const TransactionPtr &transaction) {
			std::for_each(_callbacks.begin(), _callbacks.end(),
						  [transaction](ISubWalletCallback *callback) {

							  if (transaction->getTransactionType() != ELATransaction::RegisterIdentification)
								  return;

							  PayloadRegisterIdentification *payload = static_cast<PayloadRegisterIdentification *>(
									  transaction->getPayload().get());
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

							  PayloadRegisterIdentification *payload = static_cast<PayloadRegisterIdentification *>(
									  transaction->getPayload().get());
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

							  PayloadRegisterIdentification *payload = static_cast<PayloadRegisterIdentification *>(
									  transaction->getPayload().get());
							  callback->OnTransactionStatusChanged(hash, SubWalletCallback::convertToString(
									  SubWalletCallback::Deleted), payload->toJson(), 0);
						  });
		}

	}
}