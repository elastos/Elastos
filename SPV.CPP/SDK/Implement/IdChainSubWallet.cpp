// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "IdChainSubWallet.h"
#include "MasterWallet.h"

#include <SDK/Common/ErrorChecker.h>
#include <SDK/Common/Utils.h>
#include <SDK/Common/Log.h>
#include <SDK/Plugin/Transaction/Payload/PayloadRegisterIdentification.h>

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

			uint32_t purpose = (uint32_t) info.GetIndex();
			std::set<std::string> bufferIds(registeredIds.begin(), registeredIds.end());

			//We only derive ids when accout type is "Standard"
			if (_subAccount->Parent()->GetSignType() != Account::MultiSign) {
				for (int i = 0; i < registeredIds.size() + ID_REGISTER_BUFFER_COUNT; ++i) {
					bufferIds.insert(_parent->DeriveIdAndKeyForPurpose(purpose, i));
				}
			}

			std::vector<std::string> addrs(bufferIds.begin(), bufferIds.end());
			_walletManager->getWallet()->InitListeningAddresses(addrs);
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
				program.FromJson(programJson);
				payload = PayloadPtr(new PayloadRegisterIdentification());
				payload->FromJson(payloadJson, 0);
			} catch (const nlohmann::detail::exception &e) {
				ErrorChecker::ThrowParamException(Error::JsonFormatError,
												  "Create id tx param error: " + std::string(e.what()));
			}

			TransactionPtr tx = CreateTx(fromAddress, toAddress, 0, Asset::GetELAAssetID(), memo, remark);

			tx->SetTransactionType(Transaction::RegisterIdentification, payload);

			tx->AddProgram(program);

			return tx->ToJson();
		}

		void IdChainSubWallet::onTxAdded(const TransactionPtr &transaction) {
			if (transaction != nullptr && transaction->GetTransactionType() == Transaction::RegisterIdentification) {
				std::string txHash = transaction->GetHash().GetHex();

				std::for_each(_callbacks.begin(), _callbacks.end(),
							  [&transaction, &txHash](ISubWalletCallback *callback) {
								  const PayloadRegisterIdentification *payload = static_cast<const PayloadRegisterIdentification *>(
									  transaction->GetPayload());
								  callback->OnTransactionStatusChanged(txHash, "Added", payload->ToJson(0), 0);
							  });
			} else {
				SubWallet::onTxAdded(transaction);
			}
		}

		void IdChainSubWallet::onTxUpdated(const std::string &hash, uint32_t blockHeight, uint32_t timeStamp) {
			TransactionPtr transaction = _walletManager->getWallet()->TransactionForHash(uint256(hash));
			if (transaction != nullptr && transaction->GetTransactionType() == Transaction::RegisterIdentification) {

				uint32_t confirm = blockHeight >= transaction->GetBlockHeight() ? blockHeight -
					transaction->GetBlockHeight() + 1 : 0;

				std::for_each(_callbacks.begin(), _callbacks.end(),
							  [&hash, &confirm, &transaction, this](ISubWalletCallback *callback) {

								  const PayloadRegisterIdentification *payload = static_cast<const PayloadRegisterIdentification *>(
									  transaction->GetPayload());
								  callback->OnTransactionStatusChanged(hash, "Updated", payload->ToJson(0), confirm);
							  });
			} else {
				SubWallet::onTxUpdated(hash, blockHeight, timeStamp);
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
