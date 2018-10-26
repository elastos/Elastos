// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <boost/scoped_ptr.hpp>
#include <algorithm>
#include <Core/BRTransaction.h>

#include "BRTransaction.h"
#include "BRKey.h"
#include "BRArray.h"

#include "SubWallet.h"
#include "MasterWallet.h"
#include "SubWalletCallback.h"
#include "Utils.h"
#include "Log.h"
#include "ParamChecker.h"
#include "Plugin/Transaction/TransactionOutput.h"
#include "Plugin/Transaction/Checker/TransactionChecker.h"
#include "Plugin/Transaction/Completer/TransactionCompleter.h"
#include "Account/MultiSignSubAccount.h"
#include "Account/SubAccountGenerator.h"

namespace fs = boost::filesystem;

#define DB_FILE_EXTENSION ".db"

namespace Elastos {
	namespace ElaWallet {

		SubWallet::SubWallet(const CoinInfo &info,
							 const MasterPubKeyPtr &masterPubKey,
							 const ChainParams &chainParams,
							 const PluginType &pluginTypes,
							 MasterWallet *parent) :
				PeerManager::Listener(pluginTypes),
				_parent(parent),
				_info(info),
				_syncStartHeight(0) {

			fs::path subWalletDbPath = _parent->_rootPath;
			subWalletDbPath /= parent->GetId();
			subWalletDbPath /= info.getChainId() + DB_FILE_EXTENSION;

			{
				SubAccountGenerator generator;
				generator.SetCoinInfo(_info);
				generator.SetParentAccount(_parent->_localStore.Account());
				generator.SetMasterPubKey(masterPubKey);
				_subAccount = SubAccountPtr(generator.Generate());

				_info.setChainCode(Utils::UInt256ToString(generator.GetResultChainCode()));
				if (generator.GetResultPublicKey().GetSize() > 0)
					_info.setPublicKey(Utils::encodeHex(generator.GetResultPublicKey()));
			}

			_walletManager = WalletManagerPtr(
					new SpvService(_subAccount, subWalletDbPath,
									  _info.getEarliestPeerTime(), _info.getReconnectSeconds(),
									  _info.getForkId(), pluginTypes,
									  chainParams));

			_walletManager->registerWalletListener(this);
			_walletManager->registerPeerManagerListener(this);

			if (info.getFeePerKb() > 0) {
				_walletManager->getWallet()->setFeePerKb(Asset::GetELAAssetID(), info.getFeePerKb());
			}
		}

		SubWallet::~SubWallet() {

		}

		std::string SubWallet::GetChainId() const {
			return _info.getChainId();
		}

		const SubWallet::WalletManagerPtr &SubWallet::GetWalletManager() const {
			return _walletManager;
		}

		nlohmann::json SubWallet::GetBalanceInfo() {
			return _walletManager->getWallet()->GetBalanceInfo(Asset::GetELAAssetID());
		}

		uint64_t SubWallet::GetBalance() {
			return _walletManager->getWallet()->getBalance(Asset::GetELAAssetID());
		}

		std::string SubWallet::CreateAddress() {
			return _walletManager->getWallet()->getReceiveAddress();
		}

		nlohmann::json SubWallet::GetAllAddress(uint32_t start,
												uint32_t count) {
			nlohmann::json j;
			std::vector<std::string> addresses = _walletManager->getWallet()->getAllAddresses();
			if (start >= addresses.size()) {
				j["Addresses"] = {};
				j["MaxCount"] = addresses.size();
			} else {
				uint32_t end = std::min(start + count, (uint32_t) addresses.size());
				std::vector<std::string> results(addresses.begin() + start, addresses.begin() + end);
				j["Addresses"] = results;
				j["MaxCount"] = addresses.size();
			}
			return j;
		}

		uint64_t SubWallet::GetBalanceWithAddress(const std::string &address) {
			return _walletManager->getWallet()->GetBalanceWithAddress(Asset::GetELAAssetID(), address);
		}

		void SubWallet::AddCallback(ISubWalletCallback *subCallback) {
			if (std::find(_callbacks.begin(), _callbacks.end(), subCallback) != _callbacks.end())
				return;
			_callbacks.push_back(subCallback);
		}

		void SubWallet::RemoveCallback(ISubWalletCallback *subCallback) {
			_callbacks.erase(std::remove(_callbacks.begin(), _callbacks.end(), subCallback), _callbacks.end());
		}

		nlohmann::json SubWallet::CreateTransaction(const std::string &fromAddress, const std::string &toAddress,
													uint64_t amount, const std::string &memo,
													const std::string &remark) {
			boost::scoped_ptr<TxParam> txParam(
					TxParamFactory::createTxParam(Normal, fromAddress, toAddress, amount, _info.getMinFee(), memo,
												  remark, Asset::GetELAAssetID()));
			TransactionPtr transaction = createTransaction(txParam.get());
			ParamChecker::checkCondition(!transaction, Error::CreateTransaction,
										 "create transaction error.");
			return transaction->toJson();
		}

		nlohmann::json SubWallet::GetAllTransaction(uint32_t start, uint32_t count, const std::string &addressOrTxid) {
			const WalletPtr &wallet = _walletManager->getWallet();

			std::vector<TransactionPtr> allTxs = wallet->getAllTransactions();
			size_t fullTxCount = allTxs.size();
			size_t pageCount = count;

			if (fullTxCount < start + count)
				pageCount = fullTxCount - start;

			std::vector<TransactionPtr> transactions(pageCount);
			uint32_t realCount = 0;
			for (int i = fullTxCount - 1 - start; i >= 0 && realCount < pageCount; --i) {
				if (!filterByAddressOrTxId(allTxs[i], addressOrTxid))
					continue;
				transactions[realCount++] = allTxs[i];
			}

			std::vector<nlohmann::json> jsonList(realCount);
			for (size_t i = 0; i < realCount; ++i) {
				nlohmann::json txJson = transactions[i]->toJson();
				transactions[i]->generateExtraTransactionInfo(txJson, _walletManager->getWallet(),
															  _walletManager->getPeerManager()->GetLastBlockHeight());
				jsonList[i] = txJson;
			}
			nlohmann::json j;
			j["Transactions"] = jsonList;
			return j;
		}

		boost::shared_ptr<Transaction>
		SubWallet::createTransaction(TxParam *param) const {
			TransactionPtr ptr = _walletManager->getWallet()->
					createTransaction(param->getFromAddress(), std::max(param->getFee(), _info.getMinFee()),
									  param->getAmount(), param->getToAddress(), param->getAssetId(),
									  param->getRemark(), param->getMemo());
			if (!ptr) return nullptr;

			ptr->setTransactionType(Transaction::TransferAsset);
			std::vector<TransactionOutput> &outList = ptr->getOutputs();
			std::for_each(outList.begin(), outList.end(),
						  [&param](TransactionOutput &output) {
							  output.setAssetId(param->getAssetId());
						  });

			return ptr;
		}

		void SubWallet::publishTransaction(const TransactionPtr &transaction) {
			_walletManager->publishTransaction(transaction);
		}

		std::string SubWallet::Sign(const std::string &message, const std::string &payPassword) {

			Key key = _subAccount->DeriveMainAccountKey(payPassword);
			return key.compactSign(message);
		}

		nlohmann::json
		SubWallet::CheckSign(const std::string &publicKey, const std::string &message, const std::string &signature) {
			return _parent->CheckSign(publicKey, message, signature);
		}

		uint64_t SubWallet::CalculateTransactionFee(const nlohmann::json &rawTransaction, uint64_t feePerKb) {
			TransactionPtr transaction(new Transaction());
			transaction->fromJson(rawTransaction);
			_walletManager->getWallet()->setFeePerKb(Asset::GetELAAssetID(), feePerKb);
			return std::max(transaction->calculateFee(feePerKb), _info.getMinFee());
		}

		void SubWallet::balanceChanged() {
			std::for_each(_callbacks.begin(), _callbacks.end(),
						  [](ISubWalletCallback *callback) {
							  callback->OnBalanceChanged();
						  });
		}

		void SubWallet::onTxAdded(const TransactionPtr &transaction) {
			if (transaction == nullptr)
				return;

			std::string txHash = Utils::UInt256ToString(transaction->getHash());
			Log::debug("onTxAdded hash={}", txHash);
			_confirmingTxs[txHash] = transaction;

			fireTransactionStatusChanged(txHash, SubWalletCallback::convertToString(SubWalletCallback::Added),
										 transaction->toJson(), 0);
		}

		void SubWallet::onTxUpdated(const std::string &hash, uint32_t blockHeight, uint32_t timeStamp) {
			if (_walletManager->getAllTransactionsCount() == 1) {
				_info.setEaliestPeerTime(timeStamp);
				_parent->Save();
			}

			if (_confirmingTxs.find(hash) == _confirmingTxs.end()) {
				_confirmingTxs[hash] = _walletManager->getWallet()->transactionForHash(Utils::UInt256FromString(hash));
			}

			uint32_t confirm = blockHeight >= _confirmingTxs[hash]->getBlockHeight() ? blockHeight -
				_confirmingTxs[hash]->getBlockHeight() + 1 : 0;
			if (_walletManager->getPeerManager()->getSyncProgress(_syncStartHeight) >= 1.0) {
				Log::debug("onTxUpdated hash={} confirm={}", hash, confirm);
				fireTransactionStatusChanged(hash, SubWalletCallback::convertToString(SubWalletCallback::Updated),
											 _confirmingTxs[hash]->toJson(), confirm);
			}
		}

		void SubWallet::onTxDeleted(const std::string &hash, const std::string &assetID, bool notifyUser,
									bool recommendRescan) {
			Log::debug("onTxDeleted hash={}", hash);
			fireTransactionStatusChanged(hash, SubWalletCallback::convertToString(SubWalletCallback::Deleted),
										 nlohmann::json(), 0);
		}

		void SubWallet::recover(int limitGap) {
			_walletManager->recover(limitGap);
		}

		nlohmann::json SubWallet::CreateMultiSignTransaction(const std::string &fromAddress,
															 const std::string &toAddress, uint64_t amount,
															 const std::string &memo) {
			return CreateTransaction(fromAddress, toAddress, amount, memo, "");
		}

		nlohmann::json SubWallet::SignTransaction(const nlohmann::json &rawTransaction,
												  const std::string &payPassword) {
			TransactionPtr transaction(new Transaction());
			transaction->fromJson(rawTransaction);
			_walletManager->getWallet()->signTransaction(transaction, payPassword);
			transaction->removeDuplicatePrograms();
			return transaction->toJson();
		}

		nlohmann::json SubWallet::PublishTransaction(const nlohmann::json &rawTransaction) {
			TransactionPtr transaction(new Transaction());
			transaction->fromJson(rawTransaction);

			verifyRawTransaction(transaction);
			publishTransaction(transaction);

			nlohmann::json j;
			j["TxHash"] = Utils::UInt256ToString(transaction->getHash(), true);
			j["Fee"] = transaction->getFee();
			return j;
		}

		nlohmann::json
		SubWallet::UpdateTransactionFee(const nlohmann::json &transactionJson, uint64_t fee) {
			TransactionPtr transaction(new Transaction());
			transaction->fromJson(transactionJson);

			completeTransaction(transaction, fee);
			return transaction->toJson();
		}

		void SubWallet::verifyRawTransaction(const TransactionPtr &transaction) {
			TransactionChecker checker(transaction, _walletManager->getWallet());
			checker.Check();
		}

		TransactionPtr SubWallet::completeTransaction(const TransactionPtr &transaction, uint64_t actualFee) {
			TransactionCompleter completer(transaction, _walletManager->getWallet());
			return completer.Complete(actualFee);
		}

		bool SubWallet::filterByAddressOrTxId(const TransactionPtr &transaction, const std::string &addressOrTxid) {

			if (addressOrTxid.empty()) {
				return true;
			}

			const WalletPtr &wallet = _walletManager->getWallet();
			for (size_t i = 0; i < transaction->getInputs().size(); ++i) {
				const TransactionPtr &tx = wallet->transactionForHash(transaction->getInputs()[i].getTransctionHash());
				if (tx->getOutputs()[transaction->getInputs()[i].getIndex()].getAddress() == addressOrTxid) {
					return true;
				}
			}
			for (size_t i = 0; i < transaction->getOutputs().size(); ++i) {
				if (transaction->getOutputs()[i].getAddress() == addressOrTxid) {
					return true;
				}
			}

			if (addressOrTxid.length() == sizeof(UInt256) * 2) {
				UInt256 Txid = Utils::UInt256FromString(addressOrTxid, true);
				if (UInt256Eq(&Txid, &transaction->getHash())) {
					return true;
				}
			}

			return false;
		}

		void SubWallet::syncStarted() {
			_syncStartHeight = _walletManager->getPeerManager()->getSyncStartHeight();
			if (_info.getEarliestPeerTime() == 0) {
				_info.setEaliestPeerTime(time(nullptr));
			}

			std::for_each(_callbacks.begin(), _callbacks.end(),
						  [](ISubWalletCallback *callback) {
							  callback->OnBlockSyncStarted();
						  });
		}

		void SubWallet::syncStopped(const std::string &error) {
			_syncStartHeight = 0;

			if (!error.empty()) {
				Log::error("syncStopped with error: {}", error);
			}

			std::for_each(_callbacks.begin(), _callbacks.end(),
						  [](ISubWalletCallback *callback) {
							  callback->OnBlockSyncStopped();
						  });
		}

		void SubWallet::saveBlocks(bool replace, const std::vector<MerkleBlockPtr> &blocks) {
			Log::debug("[{}] Save blocks count={}", _info.getChainId(), blocks.size());
		}

		void SubWallet::blockHeightIncreased(uint32_t blockHeight) {
			for (TransactionMap::iterator it = _confirmingTxs.begin(); it != _confirmingTxs.end(); ++it) {

				double process = _walletManager->getPeerManager()->getSyncProgress(_syncStartHeight);
				if (process < 1.0)
					continue;

				uint32_t confirms = blockHeight >= it->second->getBlockHeight() ?
									blockHeight - it->second->getBlockHeight() + 1 : 0;

				if (confirms > 1) {
					Log::debug("Tx hash={} confirms={}", it->first, confirms);
					fireTransactionStatusChanged(it->first, SubWalletCallback::convertToString(SubWalletCallback::Updated),
												 it->second->toJson(), confirms);
				}
			}

			for (TransactionMap::iterator it = _confirmingTxs.begin(); it != _confirmingTxs.end();) {
				uint32_t confirms =
					blockHeight >= it->second->getBlockHeight() ? blockHeight - it->second->getBlockHeight() + 1 : 0;

				if (confirms >= 6)
					_confirmingTxs.erase(it++);
				else
					++it;
			}

			std::for_each(_callbacks.begin(), _callbacks.end(),
						  [blockHeight, this](ISubWalletCallback *callback) {
							  callback->OnBlockHeightIncreased(
									  blockHeight,
									  (int) (_walletManager->getPeerManager()->getSyncProgress(_syncStartHeight) *
											 100));
						  });
		}

		void SubWallet::fireTransactionStatusChanged(const std::string &txid, const std::string &status,
													 const nlohmann::json &desc, uint32_t confirms) {
			std::string reversedId(txid.rbegin(), txid.rend());
			std::for_each(_callbacks.begin(), _callbacks.end(),
						  [&reversedId, &status, &desc, confirms](ISubWalletCallback *callback) {
							  callback->OnTransactionStatusChanged(reversedId, status, desc, confirms);
						  });
		}

		const CoinInfo &SubWallet::getCoinInfo() {
			_info.setFeePerKb(_walletManager->getWallet()->getFeePerKb(Asset::GetELAAssetID()));
			return _info;
		}

		void SubWallet::StartP2P() {
			if (_info.getEnableP2P())
				_walletManager->start();
		}

		void SubWallet::StopP2P() {
			if (_info.getEnableP2P())
				_walletManager->stop();
		}

		std::string SubWallet::GetPublicKey() const {
			return _subAccount->GetMainAccountPublicKey();
		}

		nlohmann::json SubWallet::GetBasicInfo() const {
			nlohmann::json j;
			j["Type"] = "Normal";
			j["Account"] = _subAccount->GetBasicInfo();
			return j;
		}

		nlohmann::json SubWallet::GetTransactionSignedSigners(const nlohmann::json &rawTransaction) {
			nlohmann::json result;
			MultiSignSubAccount *subAccount = dynamic_cast<MultiSignSubAccount *>(_subAccount.get());
			if (subAccount != nullptr) {
				TransactionPtr transaction(new Transaction);
				transaction->fromJson(rawTransaction);
				result["Signers"] = subAccount->GetTransactionSignedSigners(transaction);
			}
			return result;
		}

		nlohmann::json SubWallet::GetAssetDetails(const std::string &assetID) const {
			Asset asset = _walletManager->FindAsset(Utils::UInt256FromString(assetID));
			return asset.toJson();
		}

	}
}
