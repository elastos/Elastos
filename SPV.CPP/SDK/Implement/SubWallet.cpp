// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "MasterWallet.h"
#include "SubWallet.h"

#include <SDK/Common/Utils.h>
#include <SDK/Common/Log.h>
#include <SDK/Common/ErrorChecker.h>
#include <SDK/Plugin/Transaction/TransactionOutput.h>
#include <SDK/Account/SubAccount.h>
#include <SDK/WalletCore/BIPs/Base58.h>
#include <SDK/WalletCore/KeyStore/CoinInfo.h>
#include <SDK/SpvService/Config.h>

#include <algorithm>
#include <boost/scoped_ptr.hpp>

namespace fs = boost::filesystem;

#define DB_FILE_EXTENSION ".db"

namespace Elastos {
	namespace ElaWallet {

		SubWallet::SubWallet(const CoinInfoPtr &info,
							 const ChainConfigPtr &config,
							 MasterWallet *parent) :
				PeerManager::Listener(),
				_parent(parent),
				_info(info),
				_config(config),
				_syncStartHeight(0) {

			fs::path subWalletDbPath = _parent->_rootPath;
			subWalletDbPath /= parent->GetId();
			subWalletDbPath /= _info->GetChainID() + DB_FILE_EXTENSION;

			_subAccount = SubAccountPtr(new SubAccount(_parent->_account, _config->Index()));
			_walletManager = WalletManagerPtr(
					new SpvService(_subAccount, subWalletDbPath,
								   _info->GetEarliestPeerTime(), _config->DisconnectionTime(),
								   _config->PluginType(), config->ChainParameters()));

			_walletManager->RegisterWalletListener(this);
			_walletManager->RegisterPeerManagerListener(this);

			WalletPtr wallet = _walletManager->getWallet();
			wallet->SetWalletID(_parent->GetId() + ":" + GetChainID());

			if (_info->GetFeePerKB() < _config->MinFee())
				_info->SetFeePerKB(_config->MinFee());

			wallet->SetFeePerKb(_info->GetFeePerKB());
		}

		SubWallet::~SubWallet() {

		}

		std::string SubWallet::GetChainID() const {
			return _info->GetChainID();
		}

		const SubWallet::WalletManagerPtr &SubWallet::GetWalletManager() const {
			return _walletManager;
		}

		nlohmann::json SubWallet::GetBalanceInfo() const {
			return _walletManager->getWallet()->GetBalanceInfo();
		}

		uint64_t SubWallet::GetBalance(BalanceType type) const {
			return _walletManager->getWallet()->GetBalance(Asset::GetELAAssetID(), GroupedAsset::BalanceType(type)).getWord();
		}

		std::string SubWallet::CreateAddress() {
			return _walletManager->getWallet()->GetReceiveAddress().String();
		}

		nlohmann::json SubWallet::GetAllAddress(uint32_t start,
												uint32_t count) const {
			nlohmann::json j;
			std::vector<Address> addresses;
			size_t maxCount = _walletManager->getWallet()->GetAllAddresses(addresses, start, count, false);

			std::vector<std::string> addrString;
			for (size_t i = 0; i < addresses.size(); ++i) {
				addrString.push_back(addresses[i].String());
			}

			j["Addresses"] = addrString;
			j["MaxCount"] = maxCount;
			return j;
		}

		uint64_t SubWallet::GetBalanceWithAddress(const std::string &address, BalanceType type) const {
			return _walletManager->getWallet()->GetBalanceWithAddress(Asset::GetELAAssetID(), address,
																	  GroupedAsset::BalanceType(type)).getWord();
		}

		void SubWallet::AddCallback(ISubWalletCallback *subCallback) {
			boost::mutex::scoped_lock scoped_lock(lock);

			if (std::find(_callbacks.begin(), _callbacks.end(), subCallback) != _callbacks.end())
				return;
			_callbacks.push_back(subCallback);
		}

		void SubWallet::RemoveCallback(ISubWalletCallback *subCallback) {
			boost::mutex::scoped_lock scoped_lock(lock);

			_callbacks.erase(std::remove(_callbacks.begin(), _callbacks.end(), subCallback), _callbacks.end());
		}

		TransactionPtr SubWallet::CreateTx(const std::string &fromAddress, const std::string &toAddress,
										   const BigInt &amount, const uint256 &assetID, const std::string &memo,
										   bool useVotedUTXO) const {
			Address receiveAddr(toAddress);

			ErrorChecker::CheckParam(!receiveAddr.Valid(), Error::CreateTransaction,
									 "invalid receiver address " + toAddress);

			const WalletPtr &wallet = _walletManager->getWallet();

			std::vector<TransactionOutput> outputs;
			outputs.emplace_back(amount, receiveAddr, assetID);

			TransactionPtr tx = wallet->CreateTransaction(fromAddress, outputs, useVotedUTXO, false);

			std::set<std::string> uniqueAddress;
			std::vector<TransactionInput> &inputs = tx->GetInputs();
			for (size_t i = 0; i < inputs.size(); ++i) {
				TransactionPtr txInput = wallet->TransactionForHash(inputs[i].GetTransctionHash());
				if (txInput) {
					TransactionOutput &o = txInput->GetOutputs()[inputs[i].GetIndex()];
					Address addr = o.GetAddress();
					if (o.GetOutputLock() > tx->GetLockTime()) {
						tx->SetLockTime(o.GetOutputLock());
					}

					if (uniqueAddress.find(addr.String()) == uniqueAddress.end()) {
						uniqueAddress.insert(addr.String());
						bytes_t code = _subAccount->GetRedeemScript(addr);
						tx->AddProgram(Program(code, bytes_t()));
					}
				}
			}

			if (_info->GetChainID() == "ELA") {
				tx->SetVersion(Transaction::TxVersion::V09);
			}

			tx->AddAttribute(Attribute(Attribute::Nonce, bytes_t(std::to_string((std::rand() & 0xFFFFFFFF)))));
			if (!memo.empty()) {
				tx->AddAttribute(Attribute(Attribute::Memo, bytes_t(memo.c_str(), memo.size())));
			}

			return tx;
		}

		nlohmann::json SubWallet::CreateTransaction(const std::string &fromAddress, const std::string &toAddress,
													uint64_t amount, const std::string &memo,
													bool useVotedUTXO) {
			BigInt bnAmount;
			bnAmount.setWord(amount);
			TransactionPtr tx = CreateTx(fromAddress, toAddress, bnAmount, Asset::GetELAAssetID(), memo, useVotedUTXO);
			return tx->ToJson();
		}

		nlohmann::json SubWallet::CreateCombineUTXOTransaction(const std::string &memo, bool useVotedUTXO) {
			std::string addr = CreateAddress();
			uint64_t balance = 0;

			if (useVotedUTXO)
				balance = GetBalance(BalanceType::Total);
			else
				balance = GetBalance(BalanceType::Default);

			TransactionPtr tx = CreateTx("", addr, balance, Asset::GetELAAssetID(), memo, useVotedUTXO);

			return tx->ToJson();
		}

		nlohmann::json SubWallet::GetAllTransaction(uint32_t start, uint32_t count, const std::string &addressOrTxid) const {
			const WalletPtr &wallet = _walletManager->getWallet();

			std::vector<TransactionPtr> allTxs = wallet->GetAllTransactions();
			size_t fullTxCount = allTxs.size();
			size_t pageCount = count;
			nlohmann::json j;

			if (start >= fullTxCount) {
				j["Transactions"] = {};
				j["MaxCount"] = fullTxCount;
				return j;
			}

			if (fullTxCount < start + count)
				pageCount = fullTxCount - start;

			std::vector<TransactionPtr> transactions(pageCount);
			uint32_t realCount = 0;
			for (long i = fullTxCount - 1 - start; i >= 0 && realCount < pageCount; --i) {
				if (!filterByAddressOrTxId(allTxs[i], addressOrTxid))
					continue;
				transactions[realCount++] = allTxs[i];
			}

			std::vector<nlohmann::json> jsonList(realCount);
			for (size_t i = 0; i < realCount; ++i) {
				uint32_t confirms = 0;
				uint32_t lastBlockHeight = _walletManager->getPeerManager()->GetLastBlockHeight();
				std::string hash = transactions[i]->GetHash().GetHex();

				confirms = transactions[i]->GetConfirms(lastBlockHeight);

				jsonList[i] = transactions[i]->GetSummary(_walletManager->getWallet(), confirms, !addressOrTxid.empty());
			}
			j["Transactions"] = jsonList;
			j["MaxCount"] = fullTxCount;
			return j;
		}

		void SubWallet::publishTransaction(const TransactionPtr &transaction) {
			_walletManager->PublishTransaction(transaction);
		}

		std::string SubWallet::Sign(const std::string &message, const std::string &payPassword) {
			return _parent->Sign(message, payPassword);
		}

		bool SubWallet::CheckSign(const std::string &publicKey, const std::string &message, const std::string &signature) {
			return _parent->CheckSign(publicKey, message, signature);
		}

		void SubWallet::balanceChanged(const uint256 &assetID, const BigInt &balance) {
			boost::mutex::scoped_lock scoped_lock(lock);

			std::for_each(_callbacks.begin(), _callbacks.end(),
						  [&assetID, &balance](ISubWalletCallback *callback) {
							  callback->OnBalanceChanged(assetID.GetHex(), balance.getDec());
						  });
		}

		void SubWallet::onTxAdded(const TransactionPtr &transaction) {
			if (transaction == nullptr)
				return;

			const uint256 &txHash = transaction->GetHash();
			_confirmingTxs[txHash] = transaction;

			if (transaction->GetTransactionType() != Transaction::CoinBase) {
				fireTransactionStatusChanged(txHash, "Added", transaction->ToJson(), 0);
			} else {
				Log::debug("{} onTxAdded: {}", _walletManager->getWallet()->GetWalletID(), txHash.GetHex());
			}
		}

		void SubWallet::onTxUpdated(const uint256 &hash, uint32_t blockHeight, uint32_t timeStamp) {
			if (_walletManager->GetAllTransactionsCount() == 1) {
				_info->SetEaliestPeerTime(timeStamp);
				_parent->Save();
			}

			if (_confirmingTxs.find(hash) == _confirmingTxs.end()) {
				_confirmingTxs[hash] = _walletManager->getWallet()->TransactionForHash(uint256(hash));
			}

			uint32_t txBlockHeight = _confirmingTxs[hash]->GetBlockHeight();
			uint32_t confirm = txBlockHeight != TX_UNCONFIRMED &&
								blockHeight >= txBlockHeight ? blockHeight - txBlockHeight + 1 : 0;

			if (_confirmingTxs[hash]->GetTransactionType() != Transaction::CoinBase) {
				fireTransactionStatusChanged(hash, "Updated", _confirmingTxs[hash]->ToJson(), confirm);
			} else {
				Log::debug("{} onTxUpdated: {}, confirm: {}", _walletManager->getWallet()->GetWalletID(),
						   hash.GetHex(), confirm);
			}
		}

		void SubWallet::onTxDeleted(const uint256 &hash, bool notifyUser, bool recommendRescan) {
			if (_confirmingTxs.find(hash) != _confirmingTxs.end()) {
				_confirmingTxs.erase(hash);
			}

			fireTransactionStatusChanged(hash, "Deleted", nlohmann::json(), 0);
		}

		void SubWallet::onAssetRegistered(const AssetPtr &asset, uint64_t amount, const uint168 &controller) {
			std::for_each(_callbacks.begin(), _callbacks.end(),
						  [&asset, &amount, &controller](ISubWalletCallback *callback) {
							  callback->OnAssetRegistered(asset->GetHash().GetHex(), asset->ToJson());
			});
		}

		nlohmann::json SubWallet::SignTransaction(const nlohmann::json &rawTransaction,
												  const std::string &payPassword) {
			TransactionPtr tx(new Transaction());
			tx->FromJson(rawTransaction);
			_walletManager->getWallet()->SignTransaction(tx, payPassword);
			return tx->ToJson();
		}

		nlohmann::json SubWallet::PublishTransaction(const nlohmann::json &rawTransaction) {
			TransactionPtr transaction(new Transaction());
			transaction->FromJson(rawTransaction);

			publishTransaction(transaction);

			nlohmann::json j;
			j["TxHash"] = transaction->GetHash().GetHex();
			j["Fee"] = transaction->GetFee();
			return j;
		}

		bool SubWallet::filterByAddressOrTxId(const TransactionPtr &transaction, const std::string &addressOrTxid) const {

			if (addressOrTxid.empty()) {
				return true;
			}

			const WalletPtr &wallet = _walletManager->getWallet();
			for (size_t i = 0; i < transaction->GetInputs().size(); ++i) {
				const TransactionPtr &tx = wallet->TransactionForHash(transaction->GetInputs()[i].GetTransctionHash());
				if (tx && tx->GetOutputs()[transaction->GetInputs()[i].GetIndex()].GetAddress() == addressOrTxid) {
					return true;
				}
			}
			for (size_t i = 0; i < transaction->GetOutputs().size(); ++i) {
				if (transaction->GetOutputs()[i].GetAddress() == addressOrTxid) {
					return true;
				}
			}

			if (addressOrTxid.length() == 64) {
				if (uint256(addressOrTxid) == transaction->GetHash()) {
					return true;
				}
			}

			return false;
		}

		void SubWallet::syncStarted() {
			_syncStartHeight = _walletManager->getPeerManager()->GetSyncStartHeight();
			if (_info->GetEarliestPeerTime() == 0) {
				_info->SetEaliestPeerTime(time(nullptr));
			}

			boost::mutex::scoped_lock scoped_lock(lock);

			std::for_each(_callbacks.begin(), _callbacks.end(),
						  [](ISubWalletCallback *callback) {
							  callback->OnBlockSyncStarted();
						  });
		}

		void SubWallet::syncProgress(uint32_t currentHeight, uint32_t estimatedHeight, time_t lastBlockTime) {
			boost::mutex::scoped_lock scoped_lock(lock);

			std::for_each(_callbacks.begin(), _callbacks.end(),
						  [&currentHeight, &estimatedHeight, &lastBlockTime](ISubWalletCallback *callback) {
							  callback->OnBlockSyncProgress(currentHeight, estimatedHeight, lastBlockTime);
						  });
		}

		void SubWallet::syncStopped(const std::string &error) {
			_syncStartHeight = 0;

			boost::mutex::scoped_lock scoped_lock(lock);

			std::for_each(_callbacks.begin(), _callbacks.end(),
						  [](ISubWalletCallback *callback) {
							  callback->OnBlockSyncStopped();
						  });
		}

		void SubWallet::saveBlocks(bool replace, const std::vector<MerkleBlockPtr> &blocks) {
		}

		void SubWallet::txPublished(const std::string &hash, const nlohmann::json &result) {
			boost::mutex::scoped_lock scoped_lock(lock);

			std::for_each(_callbacks.begin(), _callbacks.end(), [&hash, &result](ISubWalletCallback *callback) {
				callback->OnTxPublished(hash, result);
			});
		}

		void SubWallet::blockHeightIncreased(uint32_t blockHeight) {
			if (_walletManager->getPeerManager()->SyncSucceeded()) {
				for (TransactionMap::iterator it = _confirmingTxs.begin(); it != _confirmingTxs.end(); ++it) {
					uint32_t txBlockHeight = it->second->GetBlockHeight();
					uint32_t confirms = txBlockHeight != TX_UNCONFIRMED &&
										blockHeight >= txBlockHeight ? blockHeight - txBlockHeight + 1 : 0;

					if (confirms > 1) {
						fireTransactionStatusChanged(it->first, "Updated", it->second->ToJson(), confirms);
					}
				}
			}

			for (TransactionMap::iterator it = _confirmingTxs.begin(); it != _confirmingTxs.end();) {
				uint32_t txBlockHeight = it->second->GetBlockHeight();
				uint32_t confirms = txBlockHeight != TX_UNCONFIRMED &&
									blockHeight >= txBlockHeight ? blockHeight - txBlockHeight + 1 : 0;

				if (confirms >= 6)
					_confirmingTxs.erase(it++);
				else
					++it;
			}
		}

		void SubWallet::fireTransactionStatusChanged(const uint256 &txid, const std::string &status,
													 const nlohmann::json &desc, uint32_t confirms) {
			boost::mutex::scoped_lock scoped_lock(lock);

			std::for_each(_callbacks.begin(), _callbacks.end(),
						  [&txid, &status, &desc, confirms](ISubWalletCallback *callback) {
							  callback->OnTransactionStatusChanged(txid.GetHex(), status, desc, confirms);
						  });
		}

		const CoinInfoPtr &SubWallet::GetCoinInfo() const {
			return _info;
		}

		void SubWallet::StartP2P() {
			_walletManager->Start();
		}

		void SubWallet::StopP2P() {
			_walletManager->Stop();
		}

		std::string SubWallet::GetPublicKey() const {
			return _parent->GetPublicKey();
		}

		nlohmann::json SubWallet::EncodeTransaction(const nlohmann::json &tx) const {
			Transaction txn;

			txn.FromJson(tx);

			ByteStream stream;
			txn.Serialize(stream);
			bytes_t hex = stream.GetBytes();

			nlohmann::json result;

			result["Algorithm"] = "base64";
			result["Data"] = hex.getBase64();
			result["ChainID"] = GetChainID();

			return result;
		}

		nlohmann::json SubWallet::DecodeTransaction(const nlohmann::json &encodedTx) const {
			Transaction txn;

			if (encodedTx.find("Algorithm") == encodedTx.end() ||
				encodedTx.find("Data") == encodedTx.end() ||
				encodedTx.find("ChainID") == encodedTx.end()) {
				ErrorChecker::ThrowParamException(Error::InvalidArgument, "Invalid input");
			}

			std::string algorithm, data, chainID;

			try {
				algorithm = encodedTx["Algorithm"].get<std::string>();
				data = encodedTx["Data"].get<std::string>();
				chainID = encodedTx["ChainID"].get<std::string>();
			} catch (const std::exception &e) {
				ErrorChecker::ThrowParamException(Error::InvalidArgument, "Invalid input: " + std::string(e.what()));
			}

			if (chainID != GetChainID()) {
				ErrorChecker::ThrowParamException(Error::InvalidArgument,
												  "Invalid input: tx is not belongs to current subwallet");
			}

			bytes_t rawHex;
			if (algorithm == "base64") {
				rawHex.setBase64(data);
			} else if (algorithm == "base58") {
				if (!Base58::CheckDecode(data, rawHex)) {
					ErrorChecker::ThrowLogicException(Error::InvalidArgument, "Decode tx from base58 error");
				}
			} else {
				ErrorChecker::CheckCondition(true, Error::InvalidArgument, "Decode tx with unknown algorithm");
			}

			ByteStream stream(rawHex);
			ErrorChecker::CheckParam(!txn.Deserialize(stream), Error::InvalidArgument,
									 "Invalid input: deserialize fail");

			return txn.ToJson();
		}

		nlohmann::json SubWallet::GetBasicInfo() const {
			nlohmann::json j;
			j["SubAccount"] = _subAccount->GetBasicInfo();
			return j;
		}

		nlohmann::json SubWallet::GetTransactionSignedSigners(const nlohmann::json &rawTransaction) const {
			TransactionPtr tx(new Transaction);
			tx->FromJson(rawTransaction);

			return tx->GetSignedInfo();
		}

		nlohmann::json SubWallet::GetAssetInfo(const std::string &assetID) const {
			nlohmann::json info;

			AssetPtr asset = _walletManager->getWallet()->GetAsset(uint256(assetID));
			info["Registered"] = (asset != nullptr);
			if (asset != nullptr)
				info["Info"] = asset->ToJson();
			else
				info["Info"] = {};

			return info;
		}

	}
}
