// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <boost/scoped_ptr.hpp>
#include <algorithm>
#include <SDK/ELACoreExt/ELATxOutput.h>
#include <Core/BRTransaction.h>

#include "BRTransaction.h"
#include "BRWallet.h"
#include "BRKey.h"
#include "BRArray.h"

#include "SubWallet.h"
#include "MasterWallet.h"
#include "SubWalletCallback.h"
#include "Utils.h"
#include "Log.h"
#include "ParamChecker.h"
#include "Transaction/TransactionOutput.h"
#include "Transaction/TransactionChecker.h"
#include "Transaction/TransactionCompleter.h"
#include "Account/MultiSignSubAccount.h"
#include "Account/SubAccountGenerator.h"

namespace fs = boost::filesystem;

#define DB_FILE_EXTENSION ".db"

namespace Elastos {
	namespace ElaWallet {

		SubWallet::SubWallet(const CoinInfo &info,
							 const MasterPubKeyPtr &masterPubKey,
							 const ChainParams &chainParams,
							 const PluginTypes &pluginTypes,
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
					new WalletManager(_subAccount, subWalletDbPath,
									  _info.getEarliestPeerTime(), _info.getReconnectSeconds(),
									  _info.getForkId(), pluginTypes,
									  chainParams));

			_walletManager->registerWalletListener(this);
			_walletManager->registerPeerManagerListener(this);

			if (info.getFeePerKb() > 0) {
				_walletManager->getWallet()->setFeePerKb(info.getFeePerKb());
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
			return _walletManager->getWallet()->GetBalanceInfo();
		}

		uint64_t SubWallet::GetBalance() {
			return _walletManager->getWallet()->getBalance();
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
			return _walletManager->getWallet()->GetBalanceWithAddress(address);
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
			boost::scoped_ptr<TxParam> txParam(TxParamFactory::createTxParam(Normal, fromAddress, toAddress, amount,
																			 _info.getMinFee(), memo, remark));
			TransactionPtr transaction = createTransaction(txParam.get());
			ParamChecker::checkCondition(!transaction, Error::CreateTransaction,
										 "create transaction error.");
			return transaction->toJson();
		}

		nlohmann::json SubWallet::GetAllTransaction(uint32_t start, uint32_t count, const std::string &addressOrTxid) {
			BRWallet *wallet = _walletManager->getWallet()->getRaw();
			assert(wallet != nullptr);
			nlohmann::json j;

			size_t fullTxCount = array_count(wallet->transactions);

			if (start >= fullTxCount) {
				j["Transactions"] = {};
				j["MaxCount"] = fullTxCount;
				return j;
			}

			size_t pageCount = count;
			pthread_mutex_lock(&wallet->lock);
			if (fullTxCount < start + count)
				pageCount = fullTxCount - start;

			BRTransaction *transactions[pageCount];
			uint32_t realCount = 0;
			for (int i = fullTxCount - 1 - start; i >= 0 && realCount < pageCount; --i) {
				if (!filterByAddressOrTxId(wallet->transactions[i], addressOrTxid))
					continue;
				transactions[realCount++] = wallet->transactions[i];
			}
			pthread_mutex_unlock(&wallet->lock);

			std::vector<nlohmann::json> jsonList(realCount);
			for (size_t i = 0; i < realCount; ++i) {
				TransactionPtr transactionPtr(new Transaction((ELATransaction *) transactions[i], false));
				uint32_t confirms = 0;

				uint32_t txBlockHeight = transactionPtr->getBlockHeight();
				uint32_t lastBlockHeight = _walletManager->getPeerManager()->getLastBlockHeight();
				if (txBlockHeight != TX_UNCONFIRMED) {
					confirms = lastBlockHeight >= txBlockHeight ? lastBlockHeight - txBlockHeight + 1 : 0;
				}

				jsonList[i] = transactionPtr->GetSummary(_walletManager->getWallet(), confirms, !addressOrTxid.empty());
			}
			j["Transactions"] = jsonList;
			j["MaxCount"] = fullTxCount;
			return j;
		}

		boost::shared_ptr<Transaction>
		SubWallet::createTransaction(TxParam *param) const {
			TransactionPtr ptr = _walletManager->getWallet()->
					createTransaction(param->getFromAddress(), std::max(param->getFee(), _info.getMinFee()),
									  param->getAmount(), param->getToAddress(), param->getRemark(),
									  param->getMemo());
			if (!ptr) return nullptr;

			ptr->setTransactionType(ELATransaction::TransferAsset);
			const std::vector<TransactionOutput *> &outList = ptr->getOutputs();
			std::for_each(outList.begin(), outList.end(),
						  [&param](TransactionOutput *output) {
							  ((ELATxOutput *) output->getRaw())->assetId = param->getAssetId();
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
			_walletManager->getWallet()->setFeePerKb(feePerKb);
			return std::max(transaction->calculateFee(feePerKb), _info.getMinFee());
		}

		void SubWallet::balanceChanged(uint64_t balance) {
			std::for_each(_callbacks.begin(), _callbacks.end(),
						  [&balance](ISubWalletCallback *callback) {
							  callback->OnBalanceChanged(balance);
						  });
		}

		void SubWallet::onTxAdded(const TransactionPtr &transaction) {
			if (transaction == nullptr)
				return;


			std::string txHash = Utils::UInt256ToString(transaction->getHash());
			Log::getLogger()->debug("onTxAdded: Tx hash={}", txHash);
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

			uint32_t confirm = blockHeight != TX_UNCONFIRMED && blockHeight >= _confirmingTxs[hash]->getBlockHeight() ?
							   blockHeight - _confirmingTxs[hash]->getBlockHeight() + 1 : 0;
			if (_walletManager->getPeerManager()->getRaw()->syncSucceeded) {
				Log::getLogger()->debug("onTxUpdated: hash = {}, confirm = {}", hash, confirm);
				fireTransactionStatusChanged(hash, SubWalletCallback::convertToString(SubWalletCallback::Updated),
											 _confirmingTxs[hash]->toJson(), confirm);
			}
		}

		void SubWallet::onTxDeleted(const std::string &hash, bool notifyUser, bool recommendRescan) {
			std::string reversedId(hash.rbegin(), hash.rend());
			std::for_each(_callbacks.begin(), _callbacks.end(),
						  [&reversedId, &notifyUser, recommendRescan](ISubWalletCallback *callback) {
							  callback->OnTxDeleted(reversedId, notifyUser, recommendRescan);
						  });
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
			_walletManager->getWallet()->signTransaction(transaction, _info.getForkId(), payPassword);
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
			j["Fee"] = transaction->getStandardFee();
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

		bool SubWallet::filterByAddressOrTxId(BRTransaction *transaction, const std::string &addressOrTxid) {
			ELATransaction *tx = (ELATransaction *) transaction;

			if (addressOrTxid == "") {
				return true;
			}

			for (size_t i = 0; i < tx->raw.inCount; ++i) {
				BRTxInput *input = &tx->raw.inputs[i];
				std::string addr(input->address);
				if (addr == addressOrTxid) {
					return true;
				}
			}
			for (size_t i = 0; i < tx->outputs.size(); ++i) {
				TransactionOutput *output = tx->outputs[i];
				if (output->getAddress() == addressOrTxid) {
					return true;
				}
			}

			if (addressOrTxid.length() == sizeof(UInt256) * 2) {
				Transaction txn(tx, false);
				UInt256 Txid = Utils::UInt256FromString(addressOrTxid, true);
				if (UInt256Eq(&Txid, &tx->raw.txHash)) {
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

		void SubWallet::syncProgress(uint32_t currentHeight, uint32_t estimatedHeight) {
			std::for_each(_callbacks.begin(), _callbacks.end(),
						  [&currentHeight, &estimatedHeight](ISubWalletCallback *callback) {
							  callback->OnBlockSyncProgress(currentHeight, estimatedHeight);
						  });
		}

		void SubWallet::syncStopped(const std::string &error) {
			_syncStartHeight = 0;

			if (!error.empty()) {
				Log::getLogger()->error("syncStopped with error: {}", error);
			}

			std::for_each(_callbacks.begin(), _callbacks.end(),
						  [](ISubWalletCallback *callback) {
							  callback->OnBlockSyncStopped();
						  });
		}

		void SubWallet::saveBlocks(bool replace, const SharedWrapperList<IMerkleBlock, BRMerkleBlock *> &blocks) {
			Log::getLogger()->debug("Saving blocks: block count = {}, chain id = {}", blocks.size(),
								   _info.getChainId());
		}

		void SubWallet::txPublished(const std::string &hash, const nlohmann::json &result) {
			std::for_each(_callbacks.begin(), _callbacks.end(), [&hash, &result](ISubWalletCallback *callback) {
				callback->OnTxPublished(hash, result);
			});
		}

		void SubWallet::blockHeightIncreased(uint32_t blockHeight) {
			if (_walletManager->getPeerManager()->getRaw()->syncSucceeded) {
				for (TransactionMap::iterator it = _confirmingTxs.begin(); it != _confirmingTxs.end(); ++it) {

					uint32_t confirms = blockHeight != TX_UNCONFIRMED && blockHeight >= it->second->getBlockHeight() ?
										blockHeight - it->second->getBlockHeight() + 1 : 0;

					if (confirms > 1) {
						Log::getLogger()->debug("Tx height increased: hash = {}, confirms = {}", it->first, confirms);
						fireTransactionStatusChanged(it->first, SubWalletCallback::convertToString(SubWalletCallback::Updated),
													 it->second->toJson(), confirms);
					}
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
			_info.setFeePerKb(_walletManager->getWallet()->getFeePerKb());
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

	}
}
