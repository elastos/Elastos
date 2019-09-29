// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "IDChainSubWallet.h"
#include "MasterWallet.h"

#include <SDK/Common/ErrorChecker.h>
#include <SDK/Common/Utils.h>
#include <SDK/Common/Log.h>
#include <SDK/WalletCore/KeyStore/CoinInfo.h>
#include <SDK/Plugin/Transaction/Payload/DIDInfo.h>
#include <SDK/Plugin/Transaction/Program.h>
#include <SDK/Plugin/Transaction/TransactionOutput.h>
#include <SDK/Plugin/Transaction/IDTransaction.h>

#include <set>
#include <boost/scoped_ptr.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/join.hpp>
#include <boost/algorithm/string/classification.hpp>

namespace Elastos {
	namespace ElaWallet {

		IDChainSubWallet::IDChainSubWallet(const CoinInfoPtr &info,
										   const ChainConfigPtr &config,
										   MasterWallet *parent) :
				SidechainSubWallet(info, config, parent) {

			std::vector<std::string> registeredIds = _parent->GetAllIDs();
			if (registeredIds.size() != 1) {
				if (_subAccount->Parent()->GetSignType() == Account::Standard) {
					registeredIds.clear();
					registeredIds.push_back(_parent->DeriveIDAndKeyForPurpose(0, 0));
				}
			}

			_walletManager->GetWallet()->InitListeningAddresses(registeredIds);
		}

		IDChainSubWallet::~IDChainSubWallet() {

		}

		nlohmann::json
		IDChainSubWallet::CreateIDTransaction(const nlohmann::json &payloadJson, const std::string &memo) {
			ArgInfo("{} {}", _walletManager->GetWallet()->GetWalletID(), GetFunName());
			ArgInfo("payload: {}", payloadJson.dump());
			ArgInfo("memo: {}", memo);

			Address receiveAddr;
			PayloadPtr payload = nullptr;
			try {
				payload = PayloadPtr(new DIDInfo());
				payload->FromJson(payloadJson, 0);
				std::string id = static_cast<DIDInfo *>(payload.get())->DIDPayload().ID();
				std::vector<std::string> idSplited;
				boost::algorithm::split(idSplited, id, boost::is_any_of(":"), boost::token_compress_on);
				ErrorChecker::CheckParam(idSplited.size() != 3, Error::InvalidArgument, "invalid id format in payload JSON");
				receiveAddr = Address(idSplited[2]);
				ErrorChecker::CheckParam(!receiveAddr.Valid(), Error::InvalidArgument, "invalid receive addr(id) in payload JSON");
			} catch (const nlohmann::detail::exception &e) {
				ErrorChecker::ThrowParamException(Error::JsonFormatError,
												  "Create id tx param error: " + std::string(e.what()));
			}

			std::vector<OutputPtr> outputs;
			outputs.push_back(OutputPtr(new TransactionOutput(0, receiveAddr, Asset::GetELAAssetID())));

			TransactionPtr tx = CreateTx(IDTransaction::didTransaction, payload, "", outputs, memo);

			nlohmann::json result;
			EncodeTx(result, tx);

			ArgInfo("r => {}", result.dump());

			return result;
		}

	}
}
