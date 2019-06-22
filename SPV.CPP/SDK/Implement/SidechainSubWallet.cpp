// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "SidechainSubWallet.h"
#include "MasterWallet.h"

#include <SDK/Common/ErrorChecker.h>
#include <SDK/Plugin/Transaction/Payload/PayloadTransferCrossChainAsset.h>
#include <SDK/SpvService/Config.h>
#include <SDK/WalletCore/KeyStore/CoinInfo.h>
#include <Core/BRAddress.h>

#include <vector>
#include <map>
#include <boost/scoped_ptr.hpp>

namespace Elastos {
	namespace ElaWallet {

		SidechainSubWallet::SidechainSubWallet(const CoinInfoPtr &info,
											   const ChainConfigPtr &config,
											   MasterWallet *parent) :
				SubWallet(info, config, parent) {

		}

		SidechainSubWallet::~SidechainSubWallet() {

		}

		nlohmann::json SidechainSubWallet::CreateWithdrawTransaction(
			const std::string &fromAddress,
			uint64_t amount,
			const std::string &mainChainAddress,
			const std::string &memo) {

			Log::preinfo("{}:{} {} | {} | {} | {} | {}", _parent->GetWalletID(), _info->GetChainID(), GetFun(),
			             fromAddress, amount, mainChainAddress, memo);

			PayloadPtr payload = nullptr;
			try {
				std::vector<std::string> accounts = {mainChainAddress};
				std::vector<uint64_t> indexs = {0};
				std::vector<uint64_t> amounts = {amount};

				payload = PayloadPtr(new PayloadTransferCrossChainAsset(accounts, indexs, amounts));
			} catch (const nlohmann::detail::exception &e) {
				ErrorChecker::ThrowParamException(Error::JsonFormatError,
												  "main chain message error: " + std::string(e.what()));
			}

			std::vector<TransactionOutput> outputs;
			outputs.emplace_back(BigInt(amount + _config->MinFee()), Address(ELA_SIDECHAIN_DESTROY_ADDR), Asset::GetELAAssetID());

			TransactionPtr tx = CreateTx(fromAddress, outputs, memo);
			ErrorChecker::CheckLogic(tx == nullptr, Error::CreateTransaction, "Create withdraw tx");

			tx->SetTransactionType(Transaction::TransferCrossChainAsset, payload);

			nlohmann::json txJson = tx->ToJson();
			Log::retinfo("{}:{} {} | {}", _parent->GetWalletID(), _info->GetChainID(), GetFun(), txJson.dump());
			return txJson;
		}

		std::string SidechainSubWallet::GetGenesisAddress() const {
			Log::preinfo("{}:{} {}", _parent->GetWalletID(), _info->GetChainID(), GetFun());

			std::string address = _config->GenesisAddress();

			Log::retinfo("{}:{} {} | {}", _parent->GetWalletID(), _info->GetChainID(), GetFun(), address);
			return address;
		}

	}
}