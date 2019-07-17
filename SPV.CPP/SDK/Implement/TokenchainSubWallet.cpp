// Copyright (c) 2012-2019 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "TokenchainSubWallet.h"
#include "MasterWallet.h"
#include <SDK/Common/ErrorChecker.h>
#include <SDK/WalletCore/KeyStore/CoinInfo.h>
#include <SDK/Plugin/Transaction/Payload/RegisterAsset.h>
#include <SDK/Plugin/Transaction/Transaction.h>
#include <SDK/Plugin/Transaction/TransactionOutput.h>

#include <vector>
#include <map>
#include <boost/scoped_ptr.hpp>

namespace Elastos {
	namespace ElaWallet {

		TokenchainSubWallet::TokenchainSubWallet(const CoinInfoPtr &info,
											   const ChainConfigPtr &config,
											   MasterWallet *parent) :
			SidechainSubWallet(info, config, parent) {

		}

		TokenchainSubWallet::~TokenchainSubWallet() {

		}

		nlohmann::json TokenchainSubWallet::CreateRegisterAssetTransaction(
				const std::string &name,
				const std::string &description,
				const std::string &registerToAddress,
				const std::string &registerAmount,
				uint8_t precision,
				const std::string &memo) {

			ArgInfo("{} {}", _walletManager->getWallet()->GetWalletID(), GetFunName());
			ArgInfo("name: {}", name);
			ArgInfo("desc: {}", description);
			ArgInfo("registerToAddr: {}", registerToAddress);
			ArgInfo("amount: {}", registerAmount);
			ArgInfo("precision: {}", precision);
			ArgInfo("memo: {}", memo);

			BigInt assetAmount;
			assetAmount.setDec(registerAmount);

			ErrorChecker::CheckParam(_walletManager->getWallet()->AssetNameExist(name), Error::InvalidArgument,
									 "asset name already registered");
			Address address(registerToAddress);
			ErrorChecker::CheckParam(!address.Valid(), Error::InvalidArgument, "invalid address");
			ErrorChecker::CheckParam(precision > Asset::MaxPrecision, Error::InvalidArgument, "precision too large");

			AssetPtr asset(new Asset(name, description, precision));
			PayloadPtr payload = PayloadPtr(new RegisterAsset(asset, assetAmount.getWord(), address.ProgramHash()));

			std::vector<OutputPtr> outputs;
			Address receiveAddr(CreateAddress());
			outputs.emplace_back(OutputPtr(new TransactionOutput(BigInt(1000000000), receiveAddr, Asset::GetELAAssetID())));

			TransactionPtr tx = CreateTx("", outputs, memo);

			tx->SetTransactionType(Transaction::registerAsset, payload);

			assetAmount *= BigInt(TOKEN_ASSET_PRECISION, 10);
			tx->AddOutput(OutputPtr(new TransactionOutput(assetAmount, address.ProgramHash(), asset->GetHash())));

			if (tx->GetOutputs().size() > 0)
				tx->RemoveOutput(tx->GetOutputs().front());

			nlohmann::json result;
			EncodeTx(result, tx);

			ArgInfo("r => {}", result.dump());
			return result;
		}

		nlohmann::json
		TokenchainSubWallet::CreateTransaction(const std::string &fromAddress, const std::string &toAddress,
											   const std::string &amount, const std::string &assetID,
											   const std::string &memo) {

			ArgInfo("{} {}", _walletManager->getWallet()->GetWalletID(), GetFunName());
			ArgInfo("fromAddr: {}", fromAddress);
			ArgInfo("toAddr: {}", toAddress);
			ArgInfo("amount: {}", amount);
			ArgInfo("assetID: {}", assetID);
			ArgInfo("memo: {}", memo);

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

			std::vector<OutputPtr> outputs;
			Address receiveAddr(toAddress);
			outputs.push_back(OutputPtr(new TransactionOutput(bnAmount, receiveAddr, asset)));

			TransactionPtr tx = CreateTx(fromAddress, outputs, memo);

			nlohmann::json result;
			EncodeTx(result, tx);

			ArgInfo("r => {}", result.dump());
			return result;
		}

		nlohmann::json TokenchainSubWallet::CreateCombineUTXOTransaction(const std::string &assetID,
																		 const std::string &memo) {
			ArgInfo("{} {}", _walletManager->getWallet()->GetWalletID(), GetFunName());
			ArgInfo("assetID: {}", assetID);
			ArgInfo("memo: {}", memo);

			std::string addr = CreateAddress();

			BigInt bnAmount = _walletManager->getWallet()->GetBalance(uint256(assetID), GroupedAsset::Total);

			uint256 asset = uint256(assetID);

			std::vector<OutputPtr> outputs;
			outputs.push_back(OutputPtr(new TransactionOutput(bnAmount, Address(addr), asset)));

			TransactionPtr tx = CreateTx("", outputs, memo);

			nlohmann::json result;
			EncodeTx(result, tx);

			ArgInfo("r => {}", result.dump());
			return result;
		}

		nlohmann::json TokenchainSubWallet::GetBalanceInfo(const std::string &assetID) const {
			ArgInfo("{} {}", _walletManager->getWallet()->GetWalletID(), GetFunName());
			ArgInfo("assetID: {}", assetID);

			nlohmann::json balanceInfo = _walletManager->getWallet()->GetBalanceInfo();

			ArgInfo("r => {}", balanceInfo.dump());
			return  balanceInfo;
		}

		std::string TokenchainSubWallet::GetBalance(const std::string &assetID) const {
			ArgInfo("{} {}", _walletManager->getWallet()->GetWalletID(), GetFunName());
			ArgInfo("assetID: {}", assetID);

			std::string balance = _walletManager->getWallet()->GetBalance(uint256(assetID), GroupedAsset::Total).getDec();

			ArgInfo("r => {}", balance);
			return balance;
		}

		std::string TokenchainSubWallet::GetBalanceWithAddress(const std::string &assetID, const std::string &address) const {
			ArgInfo("{} {}", _walletManager->getWallet()->GetWalletID(), GetFunName());
			ArgInfo("assetID: {}", assetID);
			ArgInfo("addr: {}", address);

			std::string balance = _walletManager->getWallet()->GetBalanceWithAddress(uint256(assetID), address,
					GroupedAsset::Total).getDec();

			ArgInfo("r => {}", balance);

			return balance;
		}

		nlohmann::json TokenchainSubWallet::GetAllAssets() const {
			ArgInfo("{} {}", _walletManager->getWallet()->GetWalletID(), GetFunName());

			nlohmann::json jsonData = _walletManager->getWallet()->GetAllAssets();

			ArgInfo("r => {}", jsonData.dump());

			return jsonData;
		}

	}
}