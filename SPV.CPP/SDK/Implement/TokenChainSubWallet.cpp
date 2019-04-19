// Copyright (c) 2012-2019 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "TokenchainSubWallet.h"
#include <SDK/Common/ErrorChecker.h>
#include <SDK/Plugin/Transaction/Payload/PayloadRegisterAsset.h>

#include <vector>
#include <map>
#include <boost/scoped_ptr.hpp>

namespace Elastos {
	namespace ElaWallet {

		TokenchainSubWallet::TokenchainSubWallet(const CoinInfo &info,
											   const ChainParams &chainParams, const PluginType &pluginTypes,
											   MasterWallet *parent) :
			SidechainSubWallet(info, chainParams, pluginTypes, parent) {

		}

		TokenchainSubWallet::~TokenchainSubWallet() {

		}

		nlohmann::json TokenchainSubWallet::GetBasicInfo() const {
			nlohmann::json j;
			j["Type"] = "Tokenchain";
			j["Account"] = _subAccount->GetBasicInfo();
			return j;
		}

		nlohmann::json TokenchainSubWallet::CreateRegisterAssetTransaction(
				const std::string &name,
				const std::string &description,
				const std::string &registerToAddress,
				uint64_t registerAmount,
				uint8_t precision,
				const std::string &memo,
				const std::string &remark) {

			BigInt bnAmount;
			bnAmount.setWord(1000000000);
			TransactionPtr tx = CreateTx("", CreateAddress(), bnAmount, Asset::GetELAAssetID(), memo, remark);

			Asset asset(name, description, precision);
			Address address(registerToAddress);
			PayloadPtr payload = PayloadPtr(new PayloadRegisterAsset(asset, registerAmount, address.ProgramHash()));
			tx->SetTransactionType(Transaction::RegisterAsset, payload);

			BigInt assetAmount(registerAmount);
			assetAmount *= BigInt(TOKEN_ASSET_PRECISION, 10);
			tx->AddOutput(TransactionOutput(assetAmount, address.ProgramHash(), asset.GetHash()));

			return tx->ToJson();
		}

		nlohmann::json
		TokenchainSubWallet::CreateTransaction(const std::string &fromAddress, const std::string &toAddress,
											  const std::string &amount, const std::string &assetID,
											   const std::string &memo, const std::string &remark) {
			uint256 asset = uint256(assetID);
			BigInt bnAmount;
			bnAmount.setDec(amount);
			TransactionPtr tx = CreateTx(fromAddress, toAddress, bnAmount, asset, memo, remark);
			return tx->ToJson();
		}

		nlohmann::json TokenchainSubWallet::GetBalanceInfo(const std::string &assetID) const {
			return _walletManager->getWallet()->GetBalanceInfo();
		}

		std::string TokenchainSubWallet::GetBalance(const std::string &assetID) const {
			return _walletManager->getWallet()->GetBalance(uint256(assetID), AssetTransactions::Total).getDec();
		}

		std::string TokenchainSubWallet::GetBalanceWithAddress(const std::string &assetID, const std::string &address) const {
			return _walletManager->getWallet()->GetBalanceWithAddress(uint256(assetID), address, AssetTransactions::Total).getDec();
		}

		nlohmann::json TokenchainSubWallet::GetAllSupportedAssets() const {
			return _walletManager->getWallet()->GetAllSupportedAssets();
		}

		nlohmann::json TokenchainSubWallet::GetAllVisibleAssets() const {
			return _info.VisibleAssetsToJson();
		}

		void TokenchainSubWallet::SetVisibleAssets(const nlohmann::json &assets) {
			ErrorChecker::CheckJsonArray(assets, 1, "assets");
			for (nlohmann::json::const_iterator it = assets.cbegin(); it != assets.cend(); ++it) {
				ErrorChecker::CheckParam(!(*it).is_string(), Error::InvalidAsset, "invalid asset array");
				std::string assetID = (*it).get<std::string>();
				uint256 asset(assetID);
				if (!_walletManager->getWallet()->ContainsAsset(asset)) {
					ErrorChecker::ThrowParamException(Error::InvalidAsset, "asset not found: " + assetID);
				}
			}

			_info.VisibleAssetsFromJson(assets);
		}

		void TokenchainSubWallet::SetVisibleAsset(const std::string &assetID) {
			uint256 asset(assetID);
			if (!_walletManager->getWallet()->ContainsAsset(asset)) {
				ErrorChecker::ThrowParamException(Error::InvalidAsset, "asset not found: " + assetID);
			}

			_info.SetVisibleAsset(asset);
		}

	}
}