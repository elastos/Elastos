// Copyright (c) 2012-2019 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "TokenchainSubWallet.h"
#include "MasterWallet.h"

#include <Common/ErrorChecker.h>
#include <WalletCore/CoinInfo.h>
#include <Plugin/Transaction/Payload/RegisterAsset.h>
#include <Plugin/Transaction/Payload/TransferAsset.h>
#include <Plugin/Transaction/Transaction.h>
#include <Plugin/Transaction/TransactionOutput.h>

#include <map>
#include <vector>
#include <boost/scoped_ptr.hpp>

namespace Elastos {
	namespace ElaWallet {

		TokenchainSubWallet::TokenchainSubWallet(const CoinInfoPtr &info,
											   const ChainConfigPtr &config,
											   MasterWallet *parent,
											   const std::string &netType) :
			SidechainSubWallet(info, config, parent, netType) {

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

			ArgInfo("{} {}", _walletManager->GetWallet()->GetWalletID(), GetFunName());
			ArgInfo("name: {}", name);
			ArgInfo("desc: {}", description);
			ArgInfo("registerToAddr: {}", registerToAddress);
			ArgInfo("amount: {}", registerAmount);
			ArgInfo("precision: {}", precision);
			ArgInfo("memo: {}", memo);

			ErrorChecker::CheckBigIntAmount(registerAmount);
			BigInt assetAmount;
			assetAmount.setDec(registerAmount);

			ErrorChecker::CheckParam(_walletManager->GetWallet()->AssetNameExist(name), Error::InvalidArgument,
									 "asset name already registered");
			Address address(registerToAddress);
			ErrorChecker::CheckParam(!address.Valid(), Error::InvalidArgument, "invalid address");
			ErrorChecker::CheckParam(precision > Asset::MaxPrecision, Error::InvalidArgument, "precision too large");

			AssetPtr asset(new Asset(name, description, precision));
			PayloadPtr payload = PayloadPtr(new RegisterAsset(asset, assetAmount.getUint64(), address.ProgramHash()));

			std::vector<OutputPtr> outputs;
			AddressPtr receiveAddr = _walletManager->GetWallet()->GetReceiveAddress();
			outputs.emplace_back(OutputPtr(new TransactionOutput(BigInt(1000000000), *receiveAddr, Asset::GetELAAssetID())));
			AddressPtr fromAddr(new Address());

			TransactionPtr tx = CreateTx(Transaction::registerAsset, payload, fromAddr, outputs, memo);

			assetAmount *= BigInt(TOKEN_ASSET_PRECISION, 10);
			tx->AddOutput(OutputPtr(new TransactionOutput(assetAmount, address, asset->GetHash())));

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

			ArgInfo("{} {}", _walletManager->GetWallet()->GetWalletID(), GetFunName());
			ArgInfo("fromAddr: {}", fromAddress);
			ArgInfo("toAddr: {}", toAddress);
			ArgInfo("amount: {}", amount);
			ArgInfo("assetID: {}", assetID);
			ArgInfo("memo: {}", memo);

			ErrorChecker::CheckBigIntAmount(amount);
			uint256 asset = uint256(assetID);

			AssetPtr assetInfo = _walletManager->GetWallet()->GetAsset(asset);
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
			AddressPtr fromAddr(new Address(fromAddress));

			PayloadPtr payload = PayloadPtr(new TransferAsset());
			TransactionPtr tx = CreateTx(Transaction::transferAsset, payload, fromAddr, outputs, memo);

			nlohmann::json result;
			EncodeTx(result, tx);

			ArgInfo("r => {}", result.dump());
			return result;
		}

		nlohmann::json TokenchainSubWallet::CreateConsolidateTransaction(const std::string &assetID,
																		 const std::string &memo) {
			ArgInfo("{} {}", _walletManager->GetWallet()->GetWalletID(), GetFunName());
			ArgInfo("assetID: {}", assetID);
			ArgInfo("memo: {}", memo);

			TransactionPtr tx = CreateConsolidateTx(memo, uint256(assetID));

			nlohmann::json result;
			EncodeTx(result, tx);

			ArgInfo("r => {}", result.dump());
			return result;
		}

		nlohmann::json TokenchainSubWallet::GetBalanceInfo(const std::string &assetID) const {
			ArgInfo("{} {}", _walletManager->GetWallet()->GetWalletID(), GetFunName());
			ArgInfo("assetID: {}", assetID);

			nlohmann::json balanceInfo = _walletManager->GetWallet()->GetBalanceInfo();

			ArgInfo("r => {}", balanceInfo.dump());
			return  balanceInfo;
		}

		std::string TokenchainSubWallet::GetBalance(const std::string &assetID) const {
			ArgInfo("{} {}", _walletManager->GetWallet()->GetWalletID(), GetFunName());
			ArgInfo("assetID: {}", assetID);

			std::string balance = _walletManager->GetWallet()->GetBalance(uint256(assetID)).getDec();

			ArgInfo("r => {}", balance);
			return balance;
		}

		std::string TokenchainSubWallet::GetBalanceWithAddress(const std::string &assetID, const std::string &address) const {
			ArgInfo("{} {}", _walletManager->GetWallet()->GetWalletID(), GetFunName());
			ArgInfo("assetID: {}", assetID);
			ArgInfo("addr: {}", address);

			std::string balance = _walletManager->GetWallet()->GetBalanceWithAddress(uint256(assetID), address).getDec();

			ArgInfo("r => {}", balance);

			return balance;
		}

		nlohmann::json TokenchainSubWallet::GetAllAssets() const {
			ArgInfo("{} {}", _walletManager->GetWallet()->GetWalletID(), GetFunName());

			nlohmann::json jsonData = _walletManager->GetWallet()->GetAllAssets();

			ArgInfo("r => {}", jsonData.dump());

			return jsonData;
		}

	}
}