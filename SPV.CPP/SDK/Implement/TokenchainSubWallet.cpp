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

			ErrorChecker::CheckParam(_walletManager->getWallet()->AssetNameExist(name), Error::InvalidArgument,
									 "asset name already registered");
			Address address(registerToAddress);
			ErrorChecker::CheckParam(!address.Valid(), Error::InvalidArgument, "invalid address");
			ErrorChecker::CheckParam(precision > Asset::MaxPrecision, Error::InvalidArgument, "precision too large");

			AssetPtr asset(new Asset(name, description, precision));
			PayloadPtr payload = PayloadPtr(new PayloadRegisterAsset(asset, registerAmount, address.ProgramHash()));

			TransactionPtr tx = CreateTx("", CreateAddress(), BigInt(1000000000 - 10000), Asset::GetELAAssetID(), memo, remark);

			tx->SetTransactionType(Transaction::RegisterAsset, payload);

			BigInt assetAmount(registerAmount);
			assetAmount *= BigInt(TOKEN_ASSET_PRECISION, 10);
			tx->AddOutput(TransactionOutput(assetAmount, address.ProgramHash(), asset->GetHash()));

			if (tx->GetOutputs().size() > 0)
				tx->GetOutputs().erase(tx->GetOutputs().begin());

			return tx->ToJson();
		}

		nlohmann::json
		TokenchainSubWallet::CreateTransaction(const std::string &fromAddress, const std::string &toAddress,
											   const std::string &amount, const std::string &assetID,
											   const std::string &memo, const std::string &remark) {
			uint256 asset = uint256(assetID);

			AssetPtr assetInfo = _walletManager->getWallet()->GetAsset(asset);
			ErrorChecker::CheckParam(assetInfo == nullptr, Error::InvalidArgument, "asset not found: " + assetID);

			uint8_t invalidPrecision = Asset::MaxPrecision - assetInfo->GetPrecision();
			assert(invalidPrecision < Asset::MaxPrecision);
			BigInt bn(1);
			for (size_t i = 0; i < invalidPrecision; ++i)
				bn *= 10;

			BigInt bnAmount;
			bnAmount.setDec(amount);

			ErrorChecker::CheckParam((bnAmount % bn) != 0, Error::InvalidArgument, "amount exceed max presicion");
			TransactionPtr tx = CreateTx(fromAddress, toAddress, bnAmount, asset, memo, remark);

			return tx->ToJson();
		}

		uint64_t TokenchainSubWallet::CalculateTransactionFee(const nlohmann::json &txJson,
															  uint64_t feePerKb) {
			TransactionPtr tx(new Transaction());
			tx->FromJson(txJson);

			if (tx->GetTransactionType() == Transaction::RegisterAsset)
				return std::max(tx->CalculateFee(feePerKb), uint64_t(1000000000));

			return SubWallet::CalculateTxFee(tx, feePerKb);
		}

		nlohmann::json TokenchainSubWallet::GetBalanceInfo(const std::string &assetID) const {
			return _walletManager->getWallet()->GetBalanceInfo();
		}

		std::string TokenchainSubWallet::GetBalance(const std::string &assetID) const {
			return _walletManager->getWallet()->GetBalance(uint256(assetID), GroupedAsset::Total).getDec();
		}

		std::string TokenchainSubWallet::GetBalanceWithAddress(const std::string &assetID, const std::string &address) const {
			return _walletManager->getWallet()->GetBalanceWithAddress(uint256(assetID), address, GroupedAsset::Total).getDec();
		}

		nlohmann::json TokenchainSubWallet::GetAllAssets() const {
			return _walletManager->getWallet()->GetAllAssets();
		}

	}
}