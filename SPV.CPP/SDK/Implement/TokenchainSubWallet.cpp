/*
 * Copyright (c) 2019 Elastos Foundation
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */
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

			WalletPtr wallet = _walletManager->GetWallet();
			ArgInfo("{} {}", wallet->GetWalletID(), GetFunName());
			ArgInfo("name: {}", name);
			ArgInfo("desc: {}", description);
			ArgInfo("registerToAddr: {}", registerToAddress);
			ArgInfo("amount: {}", registerAmount);
			ArgInfo("precision: {}", precision);
			ArgInfo("memo: {}", memo);

			ErrorChecker::CheckBigIntAmount(registerAmount);
			BigInt assetAmount;
			assetAmount.setDec(registerAmount);

			ErrorChecker::CheckParam(wallet->AssetNameExist(name), Error::InvalidArgument,
									 "asset name already registered");
			Address address(registerToAddress);
			ErrorChecker::CheckParam(!address.Valid(), Error::InvalidArgument, "invalid address");
			ErrorChecker::CheckParam(precision > Asset::MaxPrecision, Error::InvalidArgument, "precision too large");

			AssetPtr asset(new Asset(name, description, precision));
			PayloadPtr payload = PayloadPtr(new RegisterAsset(asset, assetAmount.getUint64(), address.ProgramHash()));

			OutputArray outputs;
			AddressPtr receiveAddr = wallet->GetReceiveAddress();
			outputs.emplace_back(OutputPtr(new TransactionOutput(BigInt(1000000000), *receiveAddr, Asset::GetELAAssetID())));
			AddressPtr fromAddr(new Address());

			TransactionPtr tx = wallet->CreateTransaction(Transaction::registerAsset, payload, fromAddr, outputs, memo);

			assetAmount *= BigInt(TOKEN_ASSET_PRECISION, 10);
			tx->AddOutput(OutputPtr(new TransactionOutput(assetAmount, address, asset->GetHash())));

			if (tx->GetOutputs().size() > 0) {
				tx->RemoveOutput(tx->GetOutputs().front());
				tx->FixIndex();
			}

			nlohmann::json result;
			EncodeTx(result, tx);

			ArgInfo("r => {}", result.dump());
			return result;
		}

		nlohmann::json
		TokenchainSubWallet::CreateTransaction(const std::string &fromAddress, const std::string &toAddress,
											   const std::string &amount, const std::string &assetID,
											   const std::string &memo) {
			WalletPtr wallet = _walletManager->GetWallet();
			ArgInfo("{} {}", wallet->GetWalletID(), GetFunName());
			ArgInfo("fromAddr: {}", fromAddress);
			ArgInfo("toAddr: {}", toAddress);
			ArgInfo("amount: {}", amount);
			ArgInfo("assetID: {}", assetID);
			ArgInfo("memo: {}", memo);

			ErrorChecker::CheckBigIntAmount(amount);
			uint256 asset = uint256(assetID);

			AssetPtr assetInfo = wallet->GetAsset(asset);
			ErrorChecker::CheckParam(assetInfo == nullptr, Error::InvalidArgument, "asset not found: " + assetID);

			uint8_t invalidPrecision = Asset::MaxPrecision - assetInfo->GetPrecision();
			assert(invalidPrecision < Asset::MaxPrecision);
			BigInt bn(1);
			for (size_t i = 0; i < invalidPrecision; ++i)
				bn *= 10;

			BigInt bnAmount;
			bnAmount.setDec(amount);

			ErrorChecker::CheckParam((bnAmount % bn) != 0, Error::InvalidArgument, "amount exceed max presicion");

			OutputArray outputs;
			Address receiveAddr(toAddress);
			outputs.push_back(OutputPtr(new TransactionOutput(bnAmount, receiveAddr, asset)));
			AddressPtr fromAddr(new Address(fromAddress));

			PayloadPtr payload = PayloadPtr(new TransferAsset());
			TransactionPtr tx = wallet->CreateTransaction(Transaction::transferAsset, payload, fromAddr, outputs, memo);

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