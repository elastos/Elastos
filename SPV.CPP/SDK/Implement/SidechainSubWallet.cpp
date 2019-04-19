// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "SidechainSubWallet.h"

#include <SDK/Common/ErrorChecker.h>
#include <SDK/Plugin/Transaction/Payload/PayloadTransferCrossChainAsset.h>

#include <Core/BRAddress.h>

#include <vector>
#include <map>
#include <boost/scoped_ptr.hpp>

namespace Elastos {
	namespace ElaWallet {

		SidechainSubWallet::SidechainSubWallet(const CoinInfo &info,
											   const ChainParams &chainParams, const PluginType &pluginTypes,
											   MasterWallet *parent) :
				SubWallet(info, chainParams, pluginTypes, parent) {

		}

		SidechainSubWallet::~SidechainSubWallet() {

		}

		nlohmann::json SidechainSubWallet::CreateWithdrawTransaction(
			const std::string &fromAddress,
			uint64_t amount,
			const std::string &mainChainAddress,
			const std::string &memo,
			const std::string &remark) {

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

			TransactionPtr tx = CreateTx(fromAddress, ELA_SIDECHAIN_DESTROY_ADDR, amount + _info.GetMinFee(),
													Asset::GetELAAssetID(), memo, remark);
			ErrorChecker::CheckLogic(tx == nullptr, Error::CreateTransaction, "Create withdraw tx");

			tx->SetTransactionType(Transaction::TransferCrossChainAsset, payload);

			return tx->ToJson();
		}

		std::string SidechainSubWallet::GetGenesisAddress() const {
			return _info.GetGenesisAddress();
		}

	}
}