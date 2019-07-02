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

			fs::path subWalletDBPath = _parent->_dataPath;
			subWalletDBPath /= parent->GetID();
			subWalletDBPath /= _info->GetChainID() + DB_FILE_EXTENSION;

			_subAccount = SubAccountPtr(new SubAccount(_parent->_account, _config->Index()));
			_walletManager = WalletManagerPtr(
					new SpvService(_subAccount, subWalletDBPath,
								   _info->GetEarliestPeerTime(), _config->DisconnectionTime(),
								   _config->PluginType(), config->ChainParameters()));

			_walletManager->RegisterWalletListener(this);
			_walletManager->RegisterPeerManagerListener(this);

			WalletPtr wallet = _walletManager->getWallet();
			wallet->SetWalletID(_parent->GetID() + ":" + GetChainID());

			if (_info->GetFeePerKB() < _config->MinFee())
				_info->SetFeePerKB(_config->MinFee());

			wallet->SetFeePerKb(_info->GetFeePerKB());
		}

		SubWallet::~SubWallet() {

		}

		std::string SubWallet::GetChainID() const {
			ArgInfo("{} {}", _walletManager->getWallet()->GetWalletID(), GetFunName());

			std::string chainID = _info->GetChainID();
			return chainID;
		}

		const std::string &SubWallet::GetInfoChainID() const {
			return _info->GetChainID();
		}

		const SubWallet::WalletManagerPtr &SubWallet::GetWalletManager() const {
			return _walletManager;
		}

		nlohmann::json SubWallet::GetBalanceInfo() const {
			ArgInfo("{} {}", _walletManager->getWallet()->GetWalletID(), GetFunName());

			nlohmann::json info = _walletManager->getWallet()->GetBalanceInfo();

			ArgInfo("r => {}", info.dump());
			return info;
		}

		uint64_t SubWallet::GetBalance(BalanceType type) const {
			ArgInfo("{} {}", _walletManager->getWallet()->GetWalletID(), GetFunName());
			ArgInfo("type: {}", GetBalanceTypeString(type));

			uint64_t balance = _walletManager->getWallet()->GetBalance(Asset::GetELAAssetID(), GroupedAsset::BalanceType(type)).getWord();

			ArgInfo("r => {}", balance);

			return balance;
		}

		std::string SubWallet::CreateAddress() {
			ArgInfo("{} {}", _walletManager->getWallet()->GetWalletID(), GetFunName());

			std::string address = _walletManager->getWallet()->GetReceiveAddress().String();

			ArgInfo("r => {}", address);

			return address;
		}

		nlohmann::json SubWallet::GetAllAddress(uint32_t start, uint32_t count) const {
			ArgInfo("{} {}", _walletManager->getWallet()->GetWalletID(), GetFunName());
			ArgInfo("start: {}", start);
			ArgInfo("count: {}", count);

			nlohmann::json j;
			std::vector<Address> addresses;
			size_t maxCount = _walletManager->getWallet()->GetAllAddresses(addresses, start, count, false);

			std::vector<std::string> addrString;
			for (size_t i = 0; i < addresses.size(); ++i) {
				addrString.push_back(addresses[i].String());
			}

			j["Addresses"] = addrString;
			j["MaxCount"] = maxCount;

			ArgInfo("r => {}", j.dump());
			return j;
		}

		uint64_t SubWallet::GetBalanceWithAddress(const std::string &address, BalanceType type) const {
			ArgInfo("{} {}", _walletManager->getWallet()->GetWalletID(), GetFunName());
			ArgInfo("addr: {}", address);
			ArgInfo("type: {}", GetBalanceTypeString(type));

			uint64_t balance = _walletManager->getWallet()->GetBalanceWithAddress(Asset::GetELAAssetID(), address,
					GroupedAsset::BalanceType(type)).getWord();

			ArgInfo("r => {}", balance);
			return balance;
		}

		void SubWallet::AddCallback(ISubWalletCallback *subCallback) {
			ArgInfo("{} {}", _walletManager->getWallet()->GetWalletID(), GetFunName());
			ArgInfo("callback: 0x{:x}", (long)subCallback);

			boost::mutex::scoped_lock scoped_lock(lock);

			if (std::find(_callbacks.begin(), _callbacks.end(), subCallback) != _callbacks.end())
				return;
			_callbacks.push_back(subCallback);
		}

		void SubWallet::RemoveCallback(ISubWalletCallback *subCallback) {
			ArgInfo("{} {}", _walletManager->getWallet()->GetWalletID(), GetFunName());
			ArgInfo("callback: 0x{:x}", (long)subCallback);
			boost::mutex::scoped_lock scoped_lock(lock);

			_callbacks.erase(std::remove(_callbacks.begin(), _callbacks.end(), subCallback), _callbacks.end());
		}

		TransactionPtr SubWallet::CreateTx(const std::string &fromAddress, const std::vector<TransactionOutput> &outputs,
		                                   const std::string &memo, bool useVotedUTXO) const {

			const WalletPtr &wallet = _walletManager->getWallet();

			for (const TransactionOutput &output : outputs) {
				ErrorChecker::CheckParam(!output.GetAddress().Valid(), Error::CreateTransaction,
				                         "invalid receiver address " + output.GetAddress().String());
			}

			std::string memoFormated = "type:text,msg:" + memo;

			TransactionPtr tx = wallet->CreateTransaction(fromAddress, outputs, memo, useVotedUTXO, false);

			if (_info->GetChainID() == "ELA") {
				tx->SetVersion(Transaction::TxVersion::V09);
			}

			return tx;
		}

		nlohmann::json SubWallet::CreateTransaction(const std::string &fromAddress, const std::string &toAddress,
		                                            uint64_t amount, const std::string &memo, bool useVotedUTXO) {

			ArgInfo("{} {}", _walletManager->getWallet()->GetWalletID(), GetFunName());
			ArgInfo("fromAddr: {}", fromAddress);
			ArgInfo("toAddr: {}", toAddress);
			ArgInfo("amount: {}", amount);
			ArgInfo("memo: {}", memo);
			ArgInfo("useVotedUTXO: {}", useVotedUTXO);

			BigInt bnAmount;
			bnAmount.setWord(amount);

			std::vector<TransactionOutput> outputs;
			Address receiveAddr(toAddress);
			outputs.emplace_back(bnAmount, receiveAddr);

			TransactionPtr tx = CreateTx(fromAddress, outputs, memo, useVotedUTXO);

			nlohmann::json txJson = tx->ToJson();

			ArgInfo("r => {}", txJson.dump());
			return txJson;
		}

		nlohmann::json SubWallet::GetAllUTXOs(uint32_t start, uint32_t count, const std::string &address) const {
			ArgInfo("{} {}", _walletManager->getWallet()->GetWalletID(), GetFunName());
			ArgInfo("start: {}", start);
			ArgInfo("count: {}", count);
			ArgInfo("addr: {}", address);
			size_t maxCount = 0, pageCount = 0;

			const WalletPtr &wallet = _walletManager->getWallet();

			std::vector<UTXO> UTXOs = wallet->GetAllUTXO(address);
			std::vector<CoinBaseUTXOPtr> coinbaseUTXOs = wallet->GetAllCoinBaseUTXO(address);

			maxCount = UTXOs.size() + coinbaseUTXOs.size();
			nlohmann::json j, jutxos;

			for (size_t i = start; i < UTXOs.size() && pageCount < count; ++i) {
				nlohmann::json item;
				item["Hash"] = UTXOs[i].Hash().GetHex();
				item["Index"] = UTXOs[i].Index();
				item["Amount"] = UTXOs[i].Amount().getDec();
				jutxos.push_back(item);
				pageCount++;
			}

			for (size_t i = start + pageCount; pageCount < count && i < UTXOs.size() + coinbaseUTXOs.size(); ++i) {
				nlohmann::json item;
				CoinBaseUTXOPtr cbp = coinbaseUTXOs[i - UTXOs.size()];
				item["Hash"] = cbp->Hash().GetHex();
				item["Index"] = cbp->Index();
				item["Amount"] = cbp->Amount().getDec();
				jutxos.push_back(item);
				pageCount++;
			}

			j["MaxCount"] = maxCount;
			j["UTXOs"] = jutxos;

			ArgInfo("r => {}", j.dump());
			return j;
		}

		nlohmann::json SubWallet::CreateCombineUTXOTransaction(const std::string &memo, bool useVotedUTXO) {
			ArgInfo("{} {}", _walletManager->getWallet()->GetWalletID(), GetFunName());
			ArgInfo("memo: {}", memo);
			ArgInfo("useVotedUTXO: {}", useVotedUTXO);

			TransactionPtr tx = _walletManager->getWallet()->CombineUTXO(memo, Asset::GetELAAssetID(), useVotedUTXO);

			if (_info->GetChainID() == "ELA")
				tx->SetVersion(Transaction::TxVersion::V09);

			nlohmann::json txJson = tx->ToJson();

			ArgInfo("r => {}", txJson.dump());
			return txJson;
		}

		nlohmann::json SubWallet::GetAllTransaction(uint32_t start, uint32_t count, const std::string &addressOrTxid) const {
			ArgInfo("{} {}", _walletManager->getWallet()->GetWalletID(), GetFunName());
			ArgInfo("start: {}", start);
			ArgInfo("count: {}", count);
			ArgInfo("addrOrTxID: {}", addressOrTxid);

			const WalletPtr &wallet = _walletManager->getWallet();

			std::vector<TransactionPtr> allTxs = wallet->GetAllTransactions();
			size_t fullTxCount = allTxs.size();
			size_t pageCount = count;
			nlohmann::json j;

			if (start >= fullTxCount) {
				j["Transactions"] = {};
				j["MaxCount"] = fullTxCount;

				ArgInfo("r => {}", j.dump());
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

			ArgInfo("r => {}", j.dump());
			return j;
		}

		nlohmann::json SubWallet::GetAllCoinBaseTransaction(uint32_t start, uint32_t count,
															const std::string &txID) const {
			ArgInfo("{} {}", _walletManager->getWallet()->GetWalletID(), GetFunName());
			ArgInfo("start: {}", start);
			ArgInfo("count: {}", count);
			ArgInfo("txID: {}", txID);

			nlohmann::json j;
			const WalletPtr wallet = _walletManager->getWallet();
			std::vector<CoinBaseUTXOPtr> cbs = wallet->GetAllCoinBaseUTXO("");
			size_t maxCount = cbs.size();
			size_t pageCount = count, realCount = 0;

			if (start >= maxCount) {
				j["Transactions"] = {};
				j["MaxCount"] = maxCount;
				ArgInfo("r => {}", j.dump());
				return j;
			}

			if (maxCount < start + count)
				pageCount = maxCount - start;

			if (!txID.empty())
				pageCount = 1;

			std::vector<nlohmann::json> jcbs;
			jcbs.reserve(pageCount);
			for (size_t i = maxCount - start; i > 0 && realCount < pageCount; --i) {
				const CoinBaseUTXOPtr &cbptr = cbs[i - 1];
				nlohmann::json cb;

				if (!txID.empty()) {
					if (cbptr->Hash().GetHex() == txID) {
						cb["TxHash"] = txID;
						uint32_t confirms = cbptr->GetConfirms(_walletManager->getPeerManager()->GetLastBlockHeight());
						cb["Timestamp"] = cbptr->Timestamp();
						cb["Amount"] = cbptr->Amount().getDec();
						cb["Status"] = confirms <= 100 ? "Pending" : "Confirmed";
						cb["Direction"] = "Received";

						cb["ConfirmStatus"] = confirms <= 100 ? std::to_string(confirms) : "100+";
						cb["Height"] = cbptr->BlockHeight();
						cb["Spent"] = cbptr->Spent();
						cb["Address"] = Address(cbptr->ProgramHash()).String();
						cb["Type"] = Transaction::CoinBase;
						jcbs.push_back(cb);
						break;
					}
				} else {
					nlohmann::json cb;

					cb["TxHash"] = cbptr->Hash().GetHex();
					uint32_t confirms = cbptr->GetConfirms(_walletManager->getPeerManager()->GetLastBlockHeight());
					cb["Timestamp"] = cbptr->Timestamp();
					cb["Amount"] = cbptr->Amount().getDec();
					cb["Status"] = confirms <= 100 ? "Pending" : "Confirmed";
					cb["Direction"] = "Received";

					jcbs.push_back(cb);
				}
			}
			j["Transactions"] = jcbs;
			j["MaxCount"] = maxCount;

			ArgInfo("r => {}", j.dump());
			return j;
		}

		void SubWallet::publishTransaction(const TransactionPtr &tx) {
			if (!_walletManager->getWallet()->ContainsTransaction(tx)) {
				ErrorChecker::ThrowLogicException(Error::WalletNotContainTx, "tx do not belong to the current wallet");
			}

			_walletManager->PublishTransaction(tx);
		}

		std::string SubWallet::Sign(const std::string &message, const std::string &payPassword) {
			ArgInfo("{} {}", _walletManager->getWallet()->GetWalletID(), GetFunName());
			ArgInfo("message: {}", message);
			ArgInfo("payPasswd: {}", "*");

			std::string content = _parent->Sign(message, payPassword);

			ArgInfo("r => {}", content);
			return content;
		}

		bool SubWallet::CheckSign(const std::string &publicKey, const std::string &message, const std::string &signature) {
			ArgInfo("{} {}", _walletManager->getWallet()->GetWalletID(), GetFunName());
			ArgInfo("pubkey: {}", publicKey);
			ArgInfo("msg: {}", message);
			ArgInfo("sign: {}", signature);

			bool result = _parent->CheckSign(publicKey, message, signature);

			ArgInfo("r => {}", result);
			return result;
		}

		void SubWallet::balanceChanged(const uint256 &assetID, const BigInt &balance) {
			ArgInfo("{} balanceChanged AssetID: {}, Balance: {}", _walletManager->getWallet()->GetWalletID(),
					assetID.GetHex(), balance.getDec());
			boost::mutex::scoped_lock scoped_lock(lock);

			std::for_each(_callbacks.begin(), _callbacks.end(),
						  [&assetID, &balance](ISubWalletCallback *callback) {
							  callback->OnBalanceChanged(assetID.GetHex(), balance.getDec());
						  });
		}

		void SubWallet::onCoinBaseTxAdded(const CoinBaseUTXOPtr &cb) {
			ArgInfo("{} onCoinBaseTxAdded Hash:{}", _walletManager->getWallet()->GetWalletID(), cb->Hash().GetHex());
		}

		void SubWallet::onCoinBaseTxUpdated(const std::vector<uint256> &hashes, uint32_t blockHeight, time_t timestamp) {
			ArgInfo("{} onCoinBaseTxUpdated size: {}, height: {}, timestamp: {}: [{},{} {}]",
					   _walletManager->getWallet()->GetWalletID(),
					   hashes.size(), blockHeight, timestamp, hashes.front().GetHex(),
					   (hashes.size() > 2 ? " ...," : ""),
					   (hashes.size() > 1 ? hashes.back().GetHex() : ""));
		}

		void SubWallet::onCoinBaseSpent(const std::vector<uint256> &spentHashes) {
			ArgInfo("{} onCoinBaseSpent size: {}: [{},{} {}]",
					   _walletManager->getWallet()->GetWalletID(),
					   spentHashes.size(), spentHashes.front().GetHex(),
					   (spentHashes.size() > 2 ? " ...," : ""),
					   (spentHashes.size() > 1 ? spentHashes.back().GetHex() : ""));
		}

		void SubWallet::onCoinBaseTxDeleted(const uint256 &hash, bool notifyUser, bool recommendRescan) {
			ArgInfo("{} onCoinBaseTxDeleted Hash: {}, notify: {}, rescan: {}",
					   _walletManager->getWallet()->GetWalletID(), hash.GetHex(), notifyUser, recommendRescan);
		}

		void SubWallet::onTxAdded(const TransactionPtr &tx) {
			const uint256 &txHash = tx->GetHash();
			ArgInfo("{} onTxAdded Hash: {}", _walletManager->getWallet()->GetWalletID(), txHash.GetHex());

			fireTransactionStatusChanged(txHash, "Added", tx->ToJson(), 0);
		}

		void SubWallet::onTxUpdated(const std::vector<uint256> &hashes, uint32_t blockHeight, time_t timestamp) {
			ArgInfo("{} onTxUpdated size: {}, height: {}, timestamp: {}: [{},{} {}]",
					_walletManager->getWallet()->GetWalletID(),
					hashes.size(), blockHeight, timestamp, hashes.front().GetHex(),
					(hashes.size() > 2 ? " ...," : ""),
					(hashes.size() > 1 ? hashes.back().GetHex() : ""));

			if (_walletManager->GetAllTransactionsCount() == 1) {
				_info->SetEaliestPeerTime(timestamp);
				_parent->Save();
			}

			for (size_t i = 0; i < hashes.size(); ++i) {
				TransactionPtr tx = _walletManager->getWallet()->TransactionForHash(hashes[i]);
				uint32_t confirm = tx->GetConfirms(blockHeight);

				fireTransactionStatusChanged(hashes[i], "Updated", nlohmann::json(), confirm);
			}
		}

		void SubWallet::onTxDeleted(const uint256 &hash, bool notifyUser, bool recommendRescan) {
			ArgInfo("{} onTxDeleted hash: {}, notify: {}, rescan: {}", _walletManager->getWallet()->GetWalletID(),
					hash.GetHex(), notifyUser, recommendRescan);

			fireTransactionStatusChanged(hash, "Deleted", nlohmann::json(), 0);
		}

		void SubWallet::onAssetRegistered(const AssetPtr &asset, uint64_t amount, const uint168 &controller) {
			ArgInfo("{} onAssetRegistered asset: {}, amount: {}, controller: {}",
					_walletManager->getWallet()->GetWalletID(), asset->GetHash().GetHex(), amount, controller.GetHex());

			std::for_each(_callbacks.begin(), _callbacks.end(),
						  [&asset, &amount, &controller](ISubWalletCallback *callback) {
							  callback->OnAssetRegistered(asset->GetHash().GetHex(), asset->ToJson());
			});
		}

		nlohmann::json SubWallet::SignTransaction(const nlohmann::json &txJson,
												  const std::string &payPassword) {

			ArgInfo("{} {}", _walletManager->getWallet()->GetWalletID(), GetFunName());
			ArgInfo("tx: {}", txJson.dump());
			ArgInfo("passwd: {}", "*");

			TransactionPtr tx(new Transaction());
			tx->FromJson(txJson);
			_walletManager->getWallet()->SignTransaction(tx, payPassword);
			nlohmann::json txSigned = tx->ToJson();

			ArgInfo("r => {}", txSigned.dump());
			return txSigned;
		}

		nlohmann::json SubWallet::PublishTransaction(const nlohmann::json &rawTransaction) {
			ArgInfo("{} {}", _walletManager->getWallet()->GetWalletID(), GetFunName());
			ArgInfo("tx: {}", rawTransaction.dump());

			TransactionPtr transaction(new Transaction());
			transaction->FromJson(rawTransaction);

			publishTransaction(transaction);

			nlohmann::json j;
			j["TxHash"] = transaction->GetHash().GetHex();
			j["Fee"] = transaction->GetFee();

			ArgInfo("r => {}", j.dump());
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

			ArgInfo("{} {}", _walletManager->getWallet()->GetWalletID(), GetFunName());

			boost::mutex::scoped_lock scoped_lock(lock);

			std::for_each(_callbacks.begin(), _callbacks.end(),
						  [](ISubWalletCallback *callback) {
							  callback->OnBlockSyncStarted();
						  });
		}

		void SubWallet::syncProgress(uint32_t currentHeight, uint32_t estimatedHeight, time_t lastBlockTime) {
			struct tm tm;

			localtime_r(&lastBlockTime, &tm);
			ArgInfo("{} {} [ {} / {} ] {}", _walletManager->getWallet()->GetWalletID(), GetFunName(),
					currentHeight, estimatedHeight, asctime(&tm));

			boost::mutex::scoped_lock scoped_lock(lock);

			std::for_each(_callbacks.begin(), _callbacks.end(),
						  [&currentHeight, &estimatedHeight, &lastBlockTime](ISubWalletCallback *callback) {
							  callback->OnBlockSyncProgress(currentHeight, estimatedHeight, lastBlockTime);
						  });
		}

		void SubWallet::syncStopped(const std::string &error) {
			_syncStartHeight = 0;

			ArgInfo("{} {}", _walletManager->getWallet()->GetWalletID(), GetFunName());
			boost::mutex::scoped_lock scoped_lock(lock);

			std::for_each(_callbacks.begin(), _callbacks.end(),
						  [](ISubWalletCallback *callback) {
							  callback->OnBlockSyncStopped();
						  });
		}

		void SubWallet::saveBlocks(bool replace, const std::vector<MerkleBlockPtr> &blocks) {
		}

		void SubWallet::txPublished(const std::string &hash, const nlohmann::json &result) {
			ArgInfo("{} {} hash: {} result: {}", _walletManager->getWallet()->GetWalletID(), GetFunName(), hash, result.dump());

			boost::mutex::scoped_lock scoped_lock(lock);

			std::for_each(_callbacks.begin(), _callbacks.end(), [&hash, &result](ISubWalletCallback *callback) {
				callback->OnTxPublished(hash, result);
			});
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

		std::string SubWallet::GetBalanceTypeString(BalanceType type) const {
			return type == Default ? "Default" : (type == Voted ? "Voted" : "Total");
		}

		void SubWallet::StartP2P() {
			_walletManager->Start();
		}

		void SubWallet::StopP2P() {
			_walletManager->Stop();
		}

		std::string SubWallet::GetPublicKey() const {
			ArgInfo("{} {}", _walletManager->getWallet()->GetWalletID(), GetFunName());
			std::string pubKey = _parent->GetPublicKey();

			ArgInfo("r => {}", pubKey);
			return pubKey;
		}

		nlohmann::json SubWallet::EncodeTransaction(const nlohmann::json &tx) const {
			ArgInfo("{} {}", _walletManager->getWallet()->GetWalletID(), GetFunName());
			ArgInfo("tx: {}", tx.dump());

			Transaction txn;

			txn.FromJson(tx);

			ByteStream stream;
			txn.Serialize(stream);
			bytes_t hex = stream.GetBytes();

			nlohmann::json result;

			result["Algorithm"] = "base64";
			result["Data"] = hex.getBase64();
			result["ChainID"] = GetChainID();

			ArgInfo("r => {}", result.dump());
			return result;
		}

		nlohmann::json SubWallet::DecodeTransaction(const nlohmann::json &encodedTx) const {
			ArgInfo("{} {}", _walletManager->getWallet()->GetWalletID(), GetFunName());
			ArgInfo("encodedTx: {}", encodedTx.dump());
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

			nlohmann::json txJson = txn.ToJson();

			ArgInfo("r => {}", txJson.dump());

			return txJson;
		}

		nlohmann::json SubWallet::GetBasicInfo() const {
			ArgInfo("{} {}", _walletManager->getWallet()->GetWalletID(), GetFunName());

			nlohmann::json j;
			j["Info"] = _subAccount->GetBasicInfo();
			j["ChainID"] = _info->GetChainID();

			ArgInfo("r => {}", j.dump());
			return j;
		}

		nlohmann::json SubWallet::GetTransactionSignedSigners(const nlohmann::json &rawTransaction) const {
			ArgInfo("{} {}", _walletManager->getWallet()->GetWalletID(), GetFunName());
			ArgInfo("tx: {}", rawTransaction.dump());

			TransactionPtr tx(new Transaction);
			tx->FromJson(rawTransaction);

			nlohmann::json info = tx->GetSignedInfo();

			ArgInfo("r => {}", info.dump());

			return info;
		}

		nlohmann::json SubWallet::GetAssetInfo(const std::string &assetID) const {
			ArgInfo("{} {}", _walletManager->getWallet()->GetWalletID(), GetFunName());
			ArgInfo("assetID: {}", assetID);

			nlohmann::json info;

			AssetPtr asset = _walletManager->getWallet()->GetAsset(uint256(assetID));
			info["Registered"] = (asset != nullptr);
			if (asset != nullptr)
				info["Info"] = asset->ToJson();
			else
				info["Info"] = {};

			ArgInfo("r => {}", info.dump());
			return info;
		}

	}
}
