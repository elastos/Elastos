// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <set>
#include <boost/scoped_ptr.hpp>
#include <SDK/Common/ParamChecker.h>

#include "Utils.h"
#include "MasterWallet.h"
#include "IdChainSubWallet.h"
#include "Utils.h"
#include "SubWalletCallback.h"
#include "SDK/Common/Log.h"
#include "SDK/Plugin/Transaction/Payload/PayloadRegisterIdentification.h"
#include "SDK/Plugin/Transaction/Completer/IdchainTransactionCompleter.h"
#include "SDK/Plugin/Transaction/Checker/IdchainTransactionChecker.h"

#define ID_REGISTER_BUFFER_COUNT 100

namespace Elastos {
	namespace ElaWallet {

		IdChainSubWallet::IdChainSubWallet(const CoinInfo &info, const MasterPubKeyPtr &masterPubKey,
										   const ChainParams &chainParams, const PluginType &pluginTypes,
										   MasterWallet *parent) :
				SidechainSubWallet(info, masterPubKey, chainParams, pluginTypes, parent) {

			std::vector<std::string> registeredIds = _parent->GetAllIds();

			uint32_t purpose = (uint32_t) info.getIndex();
			std::set<std::string> bufferIds(registeredIds.begin(), registeredIds.end());

			if (_subAccount->GetParent()->GetType() == "Standard") { //We only derive ids when accout type is "Standard"
				for (int i = 0; i < registeredIds.size() + ID_REGISTER_BUFFER_COUNT; ++i) {
					bufferIds.insert(_parent->DeriveIdAndKeyForPurpose(purpose, i));
				}
			}

			std::vector<std::string> addrs(bufferIds.begin(), bufferIds.end());
			_walletManager->getWallet()->initListeningAddresses(addrs);
		}

		IdChainSubWallet::~IdChainSubWallet() {

		}

		nlohmann::json
		IdChainSubWallet::CreateIdTransaction(const std::string &fromAddress, const nlohmann::json &payloadJson,
											  const nlohmann::json &programJson, const std::string &memo,
											  const std::string &remark) {
			std::string toAddress = payloadJson["Id"].get<std::string>();
			boost::scoped_ptr<TxParam> txParam(
					TxParamFactory::createTxParam(Idchain, fromAddress, toAddress, 0, _info.getMinFee(), memo, remark,
												  Asset::GetELAAssetID()));

			TransactionPtr transaction = createTransaction(txParam.get());
			ParamChecker::checkCondition(transaction == nullptr, Error::CreateTransaction, "Create ID tx");

			PayloadRegisterIdentification *payloadIdChain = static_cast<PayloadRegisterIdentification *>(transaction->getPayload());
			payloadIdChain->fromJson(payloadJson);

			Program newProgram;
			newProgram.fromJson(programJson);
			transaction->addProgram(newProgram);

			return transaction->toJson();
		}

		boost::shared_ptr<Transaction>
		IdChainSubWallet::createTransaction(TxParam *param) const {
			IdTxParam *idTxParam = dynamic_cast<IdTxParam *>(param);

			if (idTxParam != nullptr) {
				//todo create transaction without to address

				TransactionPtr ptr = _walletManager->getWallet()->createTransaction(param->getFromAddress(),
																					param->getFee(),
																					param->getAmount(),
																					param->getToAddress(),
																					param->getAssetId(),
																					param->getRemark(),
																					param->getMemo());
				if (!ptr) return nullptr;
				ptr->setTransactionType(Transaction::RegisterIdentification);

				const std::vector<TransactionOutput> &outList = ptr->getOutputs();
				for (size_t i = 0; i < outList.size(); ++i) {
					const_cast<TransactionOutput &>(outList[i]).setAssetId(param->getAssetId());
				}

				return ptr;
			} else {
				return SidechainSubWallet::createTransaction(param);
			}
		}

		void IdChainSubWallet::verifyRawTransaction(const TransactionPtr &transaction) {
			if (transaction->getTransactionType() == Transaction::RegisterIdentification) {
				IdchainTransactionChecker checker(transaction, _walletManager->getWallet());
				checker.Check();
			} else
				SidechainSubWallet::verifyRawTransaction(transaction);
		}

		TransactionPtr IdChainSubWallet::completeTransaction(const TransactionPtr &transaction, uint64_t actualFee) {
			if (transaction->getTransactionType() == Transaction::RegisterIdentification) {
				IdchainTransactionCompleter completer(transaction, _walletManager->getWallet());
				return completer.Complete(actualFee);
			}
			return SidechainSubWallet::completeTransaction(transaction, actualFee);
		}

		void IdChainSubWallet::onTxAdded(const TransactionPtr &transaction) {
			if (transaction != nullptr && transaction->getTransactionType() == Transaction::RegisterIdentification) {
				std::string txHash = Utils::UInt256ToString(transaction->getHash());
				Log::debug("ID onTxAdded hash={}", txHash);

				std::for_each(_callbacks.begin(), _callbacks.end(),
							  [transaction](ISubWalletCallback *callback) {
								  const PayloadRegisterIdentification *payload = static_cast<const PayloadRegisterIdentification *>(
										  transaction->getPayload());
								  callback->OnTransactionStatusChanged(
										  Utils::UInt256ToString(transaction->getHash(), true),
										  SubWalletCallback::convertToString(
												  SubWalletCallback::Added),
										  payload->toJson(), 0);
							  });
			} else {
				SubWallet::onTxAdded(transaction);
			}
		}

		void IdChainSubWallet::onTxUpdated(const std::string &hash, uint32_t blockHeight, uint32_t timeStamp) {
			TransactionPtr transaction = _walletManager->getWallet()->transactionForHash(
					Utils::UInt256FromString(hash));
			if (transaction != nullptr && transaction->getTransactionType() == Transaction::RegisterIdentification) {

				uint32_t confirm = blockHeight >= transaction->getBlockHeight() ? blockHeight -
					transaction->getBlockHeight() + 1 : 0;

				Log::debug("ID onTxUpdated hash={}, confirm={}", hash, confirm);
				std::string reversedId(hash.rbegin(), hash.rend());
				std::for_each(_callbacks.begin(), _callbacks.end(),
							  [&reversedId, &confirm, &transaction, this](ISubWalletCallback *callback) {

								  const PayloadRegisterIdentification *payload = static_cast<const PayloadRegisterIdentification *>(
										  transaction->getPayload());
								  callback->OnTransactionStatusChanged(reversedId, SubWalletCallback::convertToString(
										  SubWalletCallback::Updated), payload->toJson(), confirm);
							  });
			} else {
				SubWallet::onTxUpdated(hash, blockHeight, timeStamp);
			}
		}

		void IdChainSubWallet::onTxDeleted(const std::string &hash, const std::string &assetID, bool notifyUser,
										   bool recommendRescan) {
			TransactionPtr transaction = _walletManager->getWallet()->transactionForHash(
					Utils::UInt256FromString(hash));
			if (transaction != nullptr && transaction->getTransactionType() == Transaction::RegisterIdentification) {
				Log::debug("ID onTxDeleted");
				std::string reversedId(hash.rbegin(), hash.rend());
				std::for_each(_callbacks.begin(), _callbacks.end(),
							  [&reversedId, notifyUser, recommendRescan, &transaction, this](
									  ISubWalletCallback *callback) {

								  const PayloadRegisterIdentification *payload = static_cast<const PayloadRegisterIdentification *>(
										  transaction->getPayload());
								  callback->OnTransactionStatusChanged(reversedId, SubWalletCallback::convertToString(
										  SubWalletCallback::Deleted), payload->toJson(), 0);
							  });
			} else {
				SubWallet::onTxDeleted(hash, assetID, notifyUser, recommendRescan);
			}
		}

		nlohmann::json IdChainSubWallet::GetBasicInfo() const {
			nlohmann::json j;
			j["Type"] = "Idchain";
			j["Account"] = _subAccount->GetBasicInfo();
			return j;
		}

	}
}
