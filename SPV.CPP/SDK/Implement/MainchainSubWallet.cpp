// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "MainchainSubWallet.h"

#include <SDK/Common/Utils.h>
#include <SDK/Common/ParamChecker.h>
#include <SDK/KeyStore/CoinInfo.h>
#include <SDK/Plugin/Transaction/Asset.h>
#include <SDK/Plugin/Transaction/Checker/MainchainTransactionChecker.h>
#include <SDK/Plugin/Transaction/Completer/MainchainTransactionCompleter.h>
#include <SDK/Plugin/Transaction/Payload/PayloadTransferCrossChainAsset.h>

#include <vector>
#include <map>
#include <boost/scoped_ptr.hpp>

namespace Elastos {
	namespace ElaWallet {

		MainchainSubWallet::MainchainSubWallet(const CoinInfo &info,
											   const ChainParams &chainParams, const PluginType &pluginTypes,
											   MasterWallet *parent) :
				SubWallet(info, chainParams, pluginTypes, parent) {

		}

		MainchainSubWallet::~MainchainSubWallet() {

		}

		nlohmann::json MainchainSubWallet::CreateDepositTransaction(const std::string &fromAddress,
																	const std::string &toAddress,
																	const uint64_t amount,
																	const nlohmann::json &sidechainAccounts,
																	const nlohmann::json &sidechainAmounts,
																	const nlohmann::json &sidechainIndices,
																	const std::string &memo,
																	const std::string &remark) {
			ParamChecker::checkJsonArray(sidechainAccounts, 1, "Side chain accounts");
			ParamChecker::checkJsonArray(sidechainAmounts, 1, "Side chain amounts");
			ParamChecker::checkJsonArray(sidechainIndices, 1, "Side chain indices");

			PayloadPtr payload = nullptr;
			try {
				std::vector<std::string> accounts = sidechainAccounts.get<std::vector<std::string >>();
				std::vector<uint64_t> indexs = sidechainIndices.get<std::vector<uint64_t >>();
				std::vector<uint64_t> amounts = sidechainAmounts.get<std::vector<uint64_t >>();

				ParamChecker::checkCondition(accounts.size() != amounts.size() || accounts.size() != indexs.size(),
											 Error::DepositParam, "Invalid deposit parameters of side chain");

				payload = PayloadPtr(new PayloadTransferCrossChainAsset(accounts, indexs, amounts));
			} catch (const nlohmann::detail::exception &e) {
				ParamChecker::throwParamException(Error::JsonFormatError, "side chain message error: " + std::string(e.what()));
			}

			TransactionPtr tx = _walletManager->getWallet()->createTransaction(fromAddress, amount, toAddress,
																 Asset::GetELAAssetID(), remark, memo);

			ParamChecker::checkCondition(tx == nullptr, Error::CreateTransaction, "Create tx error");

			tx->setTransactionType(Transaction::TransferCrossChainAsset, payload);

			return tx->toJson();
		}

		nlohmann::json
		MainchainSubWallet::CreateVoteProducerTransaction(uint64_t stake, const nlohmann::json &publicKeys) {
			VoteProducerTxParam txParam;

			ParamChecker::checkJsonArray(publicKeys, 1, "Candidates public keys");
			ParamChecker::checkParam(stake == 0, Error::Code::VoteStakeError, "Vote stake should not be zero");

			std::vector<CMBlock> candidates;
			for (nlohmann::json::const_iterator it = publicKeys.cbegin(); it != publicKeys.cend(); ++it) {
				if (!(*it).is_string()) {
					ParamChecker::throwParamException(Error::Code::JsonFormatError, "Vote produce public keys is not string");
				}

				candidates.push_back(Utils::decodeHex((*it).get<std::string>()));
			}
			PayloadPtr payload = PayloadPtr(new PayloadVote(PayloadVote::Type::Delegate, candidates));

			TransactionPtr tx = _walletManager->getWallet()->createTransaction("", stake, CreateAddress(),
																			   Asset::GetELAAssetID(), "", "");

			ParamChecker::checkCondition(tx == nullptr, Error::CreateTransaction, "Create tx error");

			tx->setVersion(Transaction::TxVersion::V09);
			tx->setTransactionType(Transaction::TransferAsset);
			std::vector<TransactionOutput> &outputs = tx->getOutputs();
			outputs[0].SetType(TransactionOutput::Type::VoteOutput);
			outputs[0].SetPayload(payload);

			return tx->toJson();
		}

		nlohmann::json MainchainSubWallet::GetVotedProducerList() const {
			return nlohmann::json();
		}

		nlohmann::json MainchainSubWallet::ExportProducerKeystore(const std::string &backupPasswd,
																  const std::string &payPasswd) const {
			return nlohmann::json();
		}

		nlohmann::json MainchainSubWallet::GetRegisteredProducerInfo() const {
			return nlohmann::json();
		}



		void MainchainSubWallet::verifyRawTransaction(const TransactionPtr &transaction) {
			if (transaction->getTransactionType() == Transaction::TransferCrossChainAsset) {
				MainchainTransactionChecker checker(transaction, _walletManager->getWallet());
				checker.Check();
			} else
				SubWallet::verifyRawTransaction(transaction);
		}

		TransactionPtr MainchainSubWallet::completeTransaction(const TransactionPtr &transaction, uint64_t actualFee) {
			if (transaction->getTransactionType() == Transaction::TransferCrossChainAsset) {
				MainchainTransactionCompleter completer(transaction, _walletManager->getWallet());
				return completer.Complete(actualFee);
			} else
				return SubWallet::completeTransaction(transaction, actualFee);
		}

		nlohmann::json MainchainSubWallet::GetBasicInfo() const {
			nlohmann::json j;
			j["Type"] = "Mainchain";
			j["Account"] = _subAccount->GetBasicInfo();
			return j;
		}

	}
}
