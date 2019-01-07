// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "SidechainSubWallet.h"

#include <SDK/Common/Utils.h>
#include <SDK/Common/ParamChecker.h>
#include <SDK/Plugin/Transaction/Payload/PayloadTransferCrossChainAsset.h>
#include <SDK/Plugin/Transaction/Checker/SidechainTransactionChecker.h>

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

		nlohmann::json SidechainSubWallet::CreateWithdrawTransaction(const std::string &fromAddress,
																	 const uint64_t amount,
																	 const nlohmann::json &mainchainAccounts,
																	 const nlohmann::json &mainchainAmounts,
																	 const nlohmann::json &mainchainIndexs,
																	 const std::string &memo,
																	 const std::string &remark) {
			ParamChecker::checkJsonArray(mainchainAccounts, 1, "Main chain accounts");
			ParamChecker::checkJsonArray(mainchainAmounts, 1, "Main chain amounts");
			ParamChecker::checkJsonArray(mainchainIndexs, 1, "Main chain indexs");

			PayloadPtr payload = nullptr;
			try {
				std::vector<std::string> accounts = mainchainAccounts.get<std::vector<std::string>>();
				std::vector<uint64_t> indexs = mainchainIndexs.get<std::vector<uint64_t>>();
				std::vector<uint64_t> amounts = mainchainAmounts.get<std::vector<uint64_t>>();

				ParamChecker::checkParam(accounts.size() != amounts.size() || accounts.size() != indexs.size(),
										 Error::WithdrawParam, "Invalid withdraw parameters of main chain");

				payload = PayloadPtr(new PayloadTransferCrossChainAsset(accounts, indexs, amounts));
			} catch (const nlohmann::detail::exception &e) {
				ParamChecker::throwParamException(Error::JsonFormatError, "main chain message error: " + std::string(e.what()));
			}

			TransactionPtr tx = SubWallet::CreateTx(fromAddress, ELA_SIDECHAIN_DESTROY_ADDR, amount,
													Asset::GetELAAssetID(), remark, memo);
			ParamChecker::checkLogic(tx == nullptr, Error::CreateTransaction, "Create withdraw tx");

			tx->setTransactionType(Transaction::TransferCrossChainAsset, payload);

			return tx->toJson();
		}

		std::string SidechainSubWallet::GetGenesisAddress() const {
			return _info.getGenesisAddress();
		}

		void SidechainSubWallet::verifyRawTransaction(const TransactionPtr &transaction) {
			if (transaction->getTransactionType() == Transaction::TransferCrossChainAsset) {
				SidechainTransactionChecker checker(transaction, _walletManager->getWallet());
				checker.Check();
			} else
				SubWallet::verifyRawTransaction(transaction);
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
			TransactionPtr tx = SubWallet::CreateTx(fromAddress, toAddress, amount, asset, memo, remark);
			return tx->toJson();
		}

		nlohmann::json SidechainSubWallet::GetBalanceInfo(const std::string &assetID) {
			return _walletManager->getWallet()->GetBalanceInfo(Utils::UInt256FromString(assetID, true));
		}

		uint64_t SidechainSubWallet::GetBalance(const std::string &assetID) {
			return _walletManager->getWallet()->getBalance(Utils::UInt256FromString(assetID, true), AssetTransactions::Total);
		}

		uint64_t SidechainSubWallet::GetBalanceWithAddress(const std::string &assetID, const std::string &address) {
			return _walletManager->getWallet()->GetBalanceWithAddress(Utils::UInt256FromString(assetID, true), address);
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