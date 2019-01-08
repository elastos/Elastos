// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "SubWalletCallback.h"
#include "IdChainSubWallet.h"
#include "MasterWallet.h"

#include <SDK/Common/ParamChecker.h>
#include <SDK/Common/Utils.h>
#include <SDK/Common/Log.h>
#include <SDK/Plugin/Transaction/Payload/PayloadRegisterIdentification.h>
#include <SDK/Plugin/Transaction/Checker/IdchainTransactionChecker.h>

#include <set>
#include <boost/scoped_ptr.hpp>

#define ID_REGISTER_BUFFER_COUNT 100

namespace Elastos {
	namespace ElaWallet {

		IdChainSubWallet::IdChainSubWallet(const CoinInfo &info,
										   const ChainParams &chainParams, const PluginType &pluginTypes,
										   MasterWallet *parent) :
				SidechainSubWallet(info, chainParams, pluginTypes, parent) {

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
			std::string toAddress;
			Program program;
			PayloadPtr payload = nullptr;
			try {
				toAddress = payloadJson["Id"].get<std::string>();
				program.fromJson(programJson);
				payload = PayloadPtr(new PayloadRegisterIdentification());
				payload->fromJson(payloadJson, 0);
			} catch (const nlohmann::detail::exception &e) {
				ParamChecker::throwParamException(Error::JsonFormatError,
												  "Create id tx param error: " + std::string(e.what()));
			}

			TransactionPtr tx = SubWallet::CreateTx(fromAddress, toAddress, 0, Asset::GetELAAssetID(), memo, remark);

			tx->setTransactionType(Transaction::RegisterIdentification, payload);

			tx->addProgram(program);

			return tx->toJson();
		}

		void IdChainSubWallet::verifyRawTransaction(const TransactionPtr &transaction) {
			if (transaction->getTransactionType() == Transaction::RegisterIdentification) {
				IdchainTransactionChecker checker(transaction, _walletManager->getWallet());
				checker.Check();
			} else
				SidechainSubWallet::verifyRawTransaction(transaction);
		}

		void IdChainSubWallet::onTxAdded(const TransactionPtr &transaction) {
			if (transaction != nullptr && transaction->getTransactionType() == Transaction::RegisterIdentification) {
				std::string txHash = Utils::UInt256ToString(transaction->getHash(), true);

				std::for_each(_callbacks.begin(), _callbacks.end(),
							  [&transaction, &txHash](ISubWalletCallback *callback) {
								  const PayloadRegisterIdentification *payload = static_cast<const PayloadRegisterIdentification *>(
										  transaction->getPayload());
								  callback->OnTransactionStatusChanged(txHash,
										  SubWalletCallback::convertToString(SubWalletCallback::Added),
										  payload->toJson(0), 0);
							  });
			} else {
				SubWallet::onTxAdded(transaction);
			}
		}

		void IdChainSubWallet::onTxUpdated(const std::string &hash, uint32_t blockHeight, uint32_t timeStamp) {
			TransactionPtr transaction = _walletManager->getWallet()->transactionForHash(
					Utils::UInt256FromString(hash, true));
			if (transaction != nullptr && transaction->getTransactionType() == Transaction::RegisterIdentification) {

				uint32_t confirm = blockHeight >= transaction->getBlockHeight() ? blockHeight -
					transaction->getBlockHeight() + 1 : 0;

				std::string reversedId(hash.rbegin(), hash.rend());
				std::for_each(_callbacks.begin(), _callbacks.end(),
							  [&reversedId, &confirm, &transaction, this](ISubWalletCallback *callback) {

								  const PayloadRegisterIdentification *payload = static_cast<const PayloadRegisterIdentification *>(
										  transaction->getPayload());
								  callback->OnTransactionStatusChanged(reversedId, SubWalletCallback::convertToString(
										  SubWalletCallback::Updated), payload->toJson(0), confirm);
							  });
			} else {
				SubWallet::onTxUpdated(hash, blockHeight, timeStamp);
			}
		}

		void IdChainSubWallet::onTxDeleted(const std::string &hash, const std::string &assetID, bool notifyUser,
										   bool recommendRescan) {
			TransactionPtr transaction = _walletManager->getWallet()->transactionForHash(
					Utils::UInt256FromString(hash, true));
			if (transaction != nullptr && transaction->getTransactionType() == Transaction::RegisterIdentification) {
				std::for_each(_callbacks.begin(), _callbacks.end(),
							  [&hash, &notifyUser, &recommendRescan, &transaction, this](
									  ISubWalletCallback *callback) {

								  callback->OnTxDeleted(hash, notifyUser, recommendRescan);
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
