// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "SidechainSubWallet.h"

#include <SDK/Common/Utils.h>
#include <SDK/Common/ParamChecker.h>
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
				ParamChecker::throwParamException(Error::JsonFormatError, "main chain message error: " + std::string(e.what()));
			}

			TransactionPtr tx = CreateTx(fromAddress, ELA_SIDECHAIN_DESTROY_ADDR, amount + _info.getMinFee(),
													Asset::GetELAAssetID(), memo, remark);
			ParamChecker::checkLogic(tx == nullptr, Error::CreateTransaction, "Create withdraw tx");

			tx->setTransactionType(Transaction::TransferCrossChainAsset, payload);

			return tx->toJson();
		}

		std::string SidechainSubWallet::GetGenesisAddress() const {
			return _info.getGenesisAddress();
		}

		nlohmann::json SidechainSubWallet::GetBasicInfo() const {
			nlohmann::json j;
			j["Type"] = "Sidechain";
			j["Account"] = _subAccount->GetBasicInfo();
			return j;
		}

		nlohmann::json
		SidechainSubWallet::CreateTransaction(const std::string &fromAddress, const std::string &toAddress,
											  uint64_t amount, const std::string &assetID, const std::string &memo,
											  const std::string &remark) {
			UInt256 asset = Utils::UInt256FromString(assetID, true);
			TransactionPtr tx = CreateTx(fromAddress, toAddress, amount, asset, memo, remark);
			return tx->toJson();
		}

		nlohmann::json SidechainSubWallet::GetBalanceInfo(const std::string &assetID) const {
			return _walletManager->getWallet()->GetBalanceInfo(Utils::UInt256FromString(assetID, true));
		}

		uint64_t SidechainSubWallet::GetBalance(const std::string &assetID) const {
			return _walletManager->getWallet()->getBalance(Utils::UInt256FromString(assetID, true), AssetTransactions::Total);
		}

		uint64_t SidechainSubWallet::GetBalanceWithAddress(const std::string &assetID, const std::string &address) const {
			return _walletManager->getWallet()->GetBalanceWithAddress(Utils::UInt256FromString(assetID, true), address, AssetTransactions::Total);
		}

		nlohmann::json SidechainSubWallet::GetAllSupportedAssets() const {
			return _walletManager->getWallet()->GetAllSupportedAssets();
		}

		nlohmann::json SidechainSubWallet::GetAllVisibleAssets() const {
			return _info.VisibleAssetsToJson();
		}

		void SidechainSubWallet::SetVisibleAssets(const nlohmann::json &assets) {
			nlohmann::json existAssets;
			std::for_each(assets.begin(), assets.end(), [&existAssets, this](const nlohmann::json &asset){
				if (_walletManager->getWallet()->ContainsAsset(asset.get<std::string>()))
					existAssets.push_back(asset);
			});
			_info.VisibleAssetsFromJson(existAssets);
		}

	}
}