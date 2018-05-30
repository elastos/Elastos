// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <boost/scoped_ptr.hpp>

#include "ELACoreExt/ELABRTxOutput.h"
#include "ELACoreExt/Payload/PayloadRegisterIdentification.h"
#include "ELACoreExt/ELABRTransaction.h"

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

		std::string
		IdChainSubWallet::SendIdTransaction(const std::string &fromAddress,
											const nlohmann::json &payloadJson, const nlohmann::json &programJson,
											double fee, const std::string &payPassword, const std::string &memo) {
			boost::scoped_ptr<TxParam> txParam(
					TxParamFactory::createTxParam(Idchain, fromAddress, "", 0, fee, memo));

			TransactionPtr transaction = createTransaction(txParam.get());
			PayloadRegisterIdentification *payloadIdChain = static_cast<PayloadRegisterIdentification *>(transaction->getPayload().get());
			payloadIdChain->fromJson(payloadJson);

			ProgramPtr newProgram(new Program());
			newProgram->fromJson(programJson);
			transaction->addProgram(newProgram);

			TransactionOutput transactionOutput;
			transactionOutput.setAddress(payloadIdChain->getId());
			transactionOutput.setAmount(0);
			transactionOutput.setAssetId(txParam->getAssetId());
			transactionOutput.setProgramHash(Utils::UInt168FromString(payloadIdChain->getId()));
			transaction->addOutput(transactionOutput);

			return sendTransactionInternal(transaction, payPassword);
		}

		boost::shared_ptr<Transaction> IdChainSubWallet::createTransaction(TxParam *param) const {
			IdTxParam *idTxParam = dynamic_cast<IdTxParam *>(param);
			assert(idTxParam != nullptr);

			//todo create transaction without to address
			BRTransaction *tmp;
			if (!tmp) return nullptr;

			TransactionPtr ptr(new Transaction(tmp));
			ptr->setTransactionType(Transaction::RegisterIdentification);
			SharedWrapperList<TransactionOutput, BRTxOutput *> outList = ptr->getOutputs();
			std::for_each(outList.begin(), outList.end(),
						  [&param](const SharedWrapperList<TransactionOutput, BRTxOutput *>::TPtr &output) {
							  ((ELABRTxOutput *) output->getRaw())->assetId = param->getAssetId();
						  });

			return ptr;
		}

		bool IdChainSubWallet::verifyRawTransaction(const TransactionPtr &transaction) {
			//todo different verify from base class
			return SubWallet::verifyRawTransaction(transaction);
		}

		bool IdChainSubWallet::completeTransaction(const TransactionPtr &transaction) {
			//todo different complete from base class
			return SubWallet::completeTransaction(transaction);
		}

		void IdChainSubWallet::onTxAdded(const TransactionPtr &transaction) {
			std::for_each(_callbacks.begin(), _callbacks.end(),
						  [transaction](ISubWalletCallback *callback) {

							  if (transaction->getTransactionType() != Transaction::RegisterIdentification)
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
							  BRTransaction *transaction = BRWalletTransactionForHash(
									  _walletManager->getWallet()->getRaw(), Utils::UInt256FromString(hash));
							  if (transaction == nullptr ||
								  ((ELABRTransaction *) transaction)->type != Transaction::RegisterIdentification)
								  return;

							  Transaction wrapperTx(transaction);
							  callback->OnTransactionStatusChanged(hash, SubWalletCallback::convertToString(
									  SubWalletCallback::Updated), wrapperTx.toJson(), blockHeight);
						  });
		}

		void IdChainSubWallet::onTxDeleted(const std::string &hash, bool notifyUser, bool recommendRescan) {
			std::for_each(_callbacks.begin(), _callbacks.end(),
						  [&hash, notifyUser, recommendRescan, this](ISubWalletCallback *callback) {
							  BRTransaction *transaction = BRWalletTransactionForHash(
									  _walletManager->getWallet()->getRaw(), Utils::UInt256FromString(hash));
							  if (transaction == nullptr ||
								  ((ELABRTransaction *) transaction)->type != Transaction::RegisterIdentification)
								  return;

							  Transaction wrapperTx(transaction);
							  callback->OnTransactionStatusChanged(hash, SubWalletCallback::convertToString(
									  SubWalletCallback::Deleted), wrapperTx.toJson(), 0);
						  });
		}
	}
}