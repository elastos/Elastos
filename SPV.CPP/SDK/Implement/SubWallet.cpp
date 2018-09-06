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
#include "ErrorCode.h"
#include "ParamChecker.h"
#include "Transaction/TransactionOutput.h"
#include "Transaction/TransactionChecker.h"
#include "Transaction/TransactionCompleter.h"

namespace fs = boost::filesystem;

#define DB_FILE_EXTENSION ".db"

namespace Elastos {
	namespace ElaWallet {

		SubWallet::SubWallet(const CoinInfo &info,
							 const ChainParams &chainParams,
							 const std::string &payPassword,
							 const PluginTypes &pluginTypes,
							 MasterWallet *parent) :
				PeerManager::Listener(pluginTypes),
				_parent(parent),
				_info(info),
				_syncStartHeight(0) {

			fs::path subWalletDbPath = _parent->_rootPath;
			subWalletDbPath /= parent->GetId();
			subWalletDbPath /= info.getChainId() + DB_FILE_EXTENSION;

			CMBlock encryptedKey;
			UInt256 chainCode = UINT256_ZERO;
			MasterPubKey masterPubKey;
			if (!payPassword.empty()) {
				UInt512 seed = _parent->deriveSeed(payPassword);
				BRKey key;
				BRBIP32PrivKeyPath(&key, &chainCode, &seed, sizeof(seed), 3, 44 | BIP32_HARD,
								   _info.getIndex() | BIP32_HARD, 0 | BIP32_HARD);

				char rawKey[BRKeyPrivKey(&key, nullptr, 0)];
				BRKeyPrivKey(&key, rawKey, sizeof(rawKey));

				Key wrapperKey(key.secret, key.compressed);
				CMBlock pubKey = wrapperKey.getPubkey();

				masterPubKey = MasterPubKey(key, chainCode);

				_info.setChainCode(Utils::UInt256ToString(chainCode));
				_info.setPublicKey(Utils::encodeHex(pubKey));

			} else {
				ParamChecker::checkNotEmpty(_info.getPublicKey(), false);
				ParamChecker::checkNotEmpty(_info.getChainCode(), false);

				chainCode = Utils::UInt256FromString(_info.getChainCode());
				CMBlock pubKey = Utils::decodeHex(_info.getPublicKey());

				masterPubKey = MasterPubKey(pubKey, Utils::UInt256FromString(_info.getChainCode()));
			}

			_walletManager = WalletManagerPtr(
					new WalletManager(masterPubKey, subWalletDbPath,
									  _info.getEarliestPeerTime(), _info.getReconnectSeconds(),
									  _info.getSingleAddress(), _info.getIndex(), _info.getForkId(), pluginTypes,
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
			Log::getLogger()->info("chain = {}, balance = {}", _info.getChainId(),
								   _walletManager->getWallet()->getBalance());
			return _walletManager->getWallet()->getBalance();
		}

		std::string SubWallet::CreateAddress() {
			return _walletManager->getWallet()->getReceiveAddress();
		}

		nlohmann::json SubWallet::GetAllAddress(uint32_t start,
												uint32_t count) {
			std::vector<std::string> addresses = _walletManager->getWallet()->getAllAddresses();
			uint32_t end = std::min(start + count, (uint32_t) addresses.size());
			std::vector<std::string> results(addresses.begin() + start, addresses.begin() + end);
			nlohmann::json j;
			j["Addresses"] = results;
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
			if (!transaction)
				throw std::logic_error("create transaction error.");
			return transaction->toJson();
		}

		nlohmann::json SubWallet::GetAllTransaction(uint32_t start, uint32_t count, const std::string &addressOrTxid) {
			BRWallet *wallet = _walletManager->getWallet()->getRaw();
			assert(wallet != nullptr);

			Log::getLogger()->info("GetAllTransaction: start = {}, count = {}, addressOrTxid = {}", start, count,
								   addressOrTxid);

			size_t fullTxCount = array_count(wallet->transactions);
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
				nlohmann::json txJson = transactionPtr->toJson();
				transactionPtr->generateExtraTransactionInfo(txJson, _walletManager->getWallet(),
															 _walletManager->getPeerManager()->getLastBlockHeight());
				jsonList[i] = txJson;
			}
			nlohmann::json j;
			j["Transactions"] = jsonList;
			return j;
		}

		boost::shared_ptr<Transaction>
		SubWallet::createTransaction(TxParam *param) const {
			//todo consider the situation of from address and fee not null
			//todo initialize asset id if null
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

		nlohmann::json SubWallet::sendTransactionInternal(const boost::shared_ptr<Transaction> &transaction,
														  const std::string &payPassword) {
			signTransaction(transaction, _info.getForkId(), payPassword);
			transaction->removeDuplicatePrograms();
			publishTransaction(transaction);

			nlohmann::json j;
			j["TxHash"] = Utils::UInt256ToString(transaction->getHash(), true);
			j["Fee"] = transaction->getStandardFee();
			return j;
		}

		void SubWallet::publishTransaction(const TransactionPtr &transaction) {
			_walletManager->publishTransaction(transaction);
		}

		std::string SubWallet::Sign(const std::string &message, const std::string &payPassword) {

			Key key = deriveKey(payPassword);
			return key.compactSign(message);
		}

		nlohmann::json
		SubWallet::CheckSign(const std::string &publicKey, const std::string &message, const std::string &signature) {
			return _parent->CheckSign(publicKey, message, signature);
		}

		uint64_t SubWallet::CalculateTransactionFee(const nlohmann::json &rawTransaction, uint64_t feePerKb) {
			TransactionPtr transaction(new Transaction());
			transaction->fromJson(rawTransaction);
			return std::max(transaction->calculateFee(feePerKb), _info.getMinFee());
		}

		void SubWallet::balanceChanged(uint64_t balance) {
			Log::getLogger()->info("Tx callback (balanceChanged): balance={}", balance);
		}

		void SubWallet::onTxAdded(const TransactionPtr &transaction) {
			if (transaction == nullptr)
				return;


			std::string txHash = Utils::UInt256ToString(transaction->getHash());
			Log::getLogger()->info("Tx callback (onTxAdded): Tx hash={}", txHash);
			_confirmingTxs[txHash] = transaction;

			fireTransactionStatusChanged(txHash, SubWalletCallback::convertToString(SubWalletCallback::Added),
										 transaction->toJson(), 0);
			Log::getLogger()->info(
					"Tx callback (onTxAdded) finished. Details: txHash={}, tx height = {}, confirm count={}.",
					txHash, transaction->getBlockHeight(), 0);
		}

		void SubWallet::onTxUpdated(const std::string &hash, uint32_t blockHeight, uint32_t timeStamp) {
			Log::getLogger()->info("Tx callback (onTxUpdated)");
			if (_confirmingTxs.find(hash) == _confirmingTxs.end()) {
				_confirmingTxs[hash] = _walletManager->getWallet()->transactionForHash(Utils::UInt256FromString(hash));
			}

			Log::getLogger()->info("Tx callback (onTxUpdated): Tx hash={}", hash);
			uint32_t confirm = blockHeight >= _confirmingTxs[hash]->getBlockHeight() ? blockHeight -
																					   _confirmingTxs[hash]->getBlockHeight() +
																					   1 : 0;
			fireTransactionStatusChanged(hash, SubWalletCallback::convertToString(SubWalletCallback::Updated),
										 _confirmingTxs[hash]->toJson(), confirm);
			Log::getLogger()->info("Tx callback (onTxUpdated) finished. Details: txHash={}, confirm count={}.",
								   hash, confirm);
		}

		void SubWallet::onTxDeleted(const std::string &hash, bool notifyUser, bool recommendRescan) {
			Log::getLogger()->info("Tx callback (onTxDeleted) begin");
			fireTransactionStatusChanged(hash, SubWalletCallback::convertToString(SubWalletCallback::Deleted),
										 nlohmann::json(), 0);
			Log::getLogger()->info("Tx callback (onTxDeleted hash={}) finished.", hash);
		}

		void SubWallet::recover(int limitGap) {
			_walletManager->recover(limitGap);
		}

		Key SubWallet::deriveKey(const std::string &payPassword) {
			UInt512 seed = _parent->deriveSeed(payPassword);
			Key key;
			UInt256 chainCode;
			BRBIP32PrivKeyPath(key.getRaw(), &chainCode, &seed, sizeof(seed), 3, 44 | BIP32_HARD, _info.getIndex() | BIP32_HARD,
							   0 | BIP32_HARD);
			return key;
		}

		void SubWallet::signTransaction(const boost::shared_ptr<Transaction> &transaction, int forkId,
										const std::string &payPassword) {
			Log::getLogger()->info("SubWallet signTransaction method begin.");

			ParamChecker::checkNullPointer(transaction.get());
			BRWallet *wallet = _walletManager->getWallet()->getRaw();
			ParamChecker::checkNullPointer(wallet);
			Log::getLogger()->info("SubWallet signTransaction derive key down.");

			BRTransaction *tx = transaction->getRaw();
			uint32_t j, internalIdx[tx->inCount], externalIdx[tx->inCount];
			size_t i, internalCount = 0, externalCount = 0;


			Log::getLogger()->info("SubWallet signTransaction begin get indices.");
			pthread_mutex_lock(&wallet->lock);
			for (i = 0; i < tx->inCount; i++) {
				if (wallet->internalChain) {
					for (j = (uint32_t) array_count(wallet->internalChain); j > 0; j--) {
						if (BRAddressEq(tx->inputs[i].address, &wallet->internalChain[j - 1])) {
							internalIdx[internalCount++] = j - 1;
						}
					}
				}

				for (j = (uint32_t) array_count(wallet->externalChain); j > 0; j--) {
					if (BRAddressEq(tx->inputs[i].address, &wallet->externalChain[j - 1])) {
						externalIdx[externalCount++] = j - 1;
					}

				}
			}
			pthread_mutex_unlock(&wallet->lock);
			Log::getLogger()->info("SubWallet signTransaction end get indices.");

			UInt512 seed = _parent->deriveSeed(payPassword);

			BRKey keys[internalCount + externalCount];
			BRBIP44PrivKeyList(keys, internalCount, &seed, sizeof(seed), _info.getIndex(),
							   SEQUENCE_INTERNAL_CHAIN, internalIdx);
			BRBIP44PrivKeyList(&keys[internalCount], externalCount, &seed, sizeof(seed), _info.getIndex(),
							   SEQUENCE_EXTERNAL_CHAIN, externalIdx);
			memset(&seed, 0, sizeof(seed));

			Log::getLogger()->info("SubWallet signTransaction calculate private key list done.");

			if (tx) {
				Log::getLogger()->info("SubWallet signTransaction begin sign method.");
				WrapperList<Key, BRKey> keyList;
				for (i = 0; i < internalCount + externalCount; ++i) {
					Key key(keys[i].secret, keys[i].compressed);
					keyList.push_back(key);
				}
				if (!transaction->sign(keyList, forkId)) {
					throw std::logic_error("Transaction Sign error!");
				}
				Log::getLogger()->info("SubWallet signTransaction end sign method.");
			}

			for (i = 0; i < internalCount + externalCount; i++) BRKeyClean(&keys[i]);
		}

		std::string SubWallet::CreateMultiSignAddress(const nlohmann::json &multiPublicKeyJson, uint32_t totalSignNum,
													  uint32_t requiredSignNum) {
			//todo complete me
			return "";
		}

		nlohmann::json SubWallet::CreateMultiSignTransaction(const std::string &fromAddress,
															 const std::string &toAddress, uint64_t amount,
															 const std::string &memo) {
			//todo complete me
			return nlohmann::json();
		}

		nlohmann::json
		SubWallet::SendRawTransaction(const nlohmann::json &transactionJson, uint64_t fee,
									  const std::string &payPassword) {
			TransactionPtr transaction(new Transaction());
			transaction->fromJson(transactionJson);

			verifyRawTransaction(transaction);
			completeTransaction(transaction, fee);

			return sendTransactionInternal(transaction, payPassword);
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

		void SubWallet::syncStopped(const std::string &error) {
			_syncStartHeight = 0;

			if (!error.empty()) {
				Log::error(error);
			}

			std::for_each(_callbacks.begin(), _callbacks.end(),
						  [](ISubWalletCallback *callback) {
							  callback->OnBlockSyncStopped();
						  });
		}

		void SubWallet::saveBlocks(bool replace, const SharedWrapperList<IMerkleBlock, BRMerkleBlock *> &blocks) {
			Log::getLogger()->info("Saving blocks: block count = {}, chain id = {}", blocks.size(),
								   _info.getChainId());
		}

		void SubWallet::blockHeightIncreased(uint32_t blockHeight) {
			for (TransactionMap::iterator it = _confirmingTxs.begin(); it != _confirmingTxs.end(); ++it) {
				uint32_t confirms =
						blockHeight > it->second->getBlockHeight() ? blockHeight - it->second->getBlockHeight() + 1 : 0;
				Log::getLogger()->info(
						"Transaction height increased: txHash = {}, confirms = {}, tx height = {}, last block height = {}",
						it->first, confirms, it->second->getBlockHeight(),
						blockHeight);

				if (it->second->getBlockHeight() == TX_UNCONFIRMED)
					continue;

				fireTransactionStatusChanged(it->first, SubWalletCallback::convertToString(SubWalletCallback::Updated),
											 it->second->toJson(), confirms);
			}

			for (TransactionMap::iterator it = _confirmingTxs.begin(); it != _confirmingTxs.end();) {
				if (it->second->getBlockHeight() == TX_UNCONFIRMED)
					continue;

				if (blockHeight - it->second->getBlockHeight() + 1 >= 6)
					_confirmingTxs.erase(it++);
				else
					++it;
			}

			std::for_each(_callbacks.begin(), _callbacks.end(),
						  [blockHeight, this](ISubWalletCallback *callback) {
							  callback->OnBlockHeightIncreased(
									  blockHeight, _walletManager->getPeerManager()->getSyncProgress(_syncStartHeight));
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

		void SubWallet::StartP2P() {
			if (_info.getEnableP2P())
				_walletManager->start();
		}

		void SubWallet::StopP2P() {
			if (_info.getEnableP2P())
				_walletManager->stop();
		}

		void SubWallet::fireDestroyWallet() {
			std::for_each(_callbacks.begin(), _callbacks.end(),
						  [](ISubWalletCallback *callback) {
							  callback->OnDestroyWallet();
						  });
		}

		std::string SubWallet::GetPublicKey() const {
			return _info.getPublicKey();
		}

	}
}
