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
#include "MasterWallet.h"
#include "SubWallet.h"

#include <Common/Utils.h>
#include <Common/Log.h>
#include <Common/ErrorChecker.h>
#include <Plugin/Transaction/TransactionOutput.h>
#include <Plugin/Transaction/TransactionInput.h>
#include <Plugin/Transaction/IDTransaction.h>
#include <Plugin/Transaction/Payload/TransferAsset.h>
#include <Account/SubAccount.h>
#include <WalletCore/CoinInfo.h>
#include <SpvService/Config.h>

#include <algorithm>

namespace fs = boost::filesystem;

#define DB_FILE_EXTENSION ".db"

namespace Elastos {
	namespace ElaWallet {

		SubWallet::SubWallet(const CoinInfoPtr &info,
							 const ChainConfigPtr &config,
							 MasterWallet *parent,
							 const std::string &netType) :
			PeerManager::Listener(),
			_parent(parent),
			_info(info),
			_config(config),
			_callback(nullptr) {

			fs::path subWalletDBPath = _parent->GetDataPath();
			subWalletDBPath /= _info->GetChainID() + DB_FILE_EXTENSION;

			SubAccountPtr subAccount = SubAccountPtr(new SubAccount(_parent->_account, _config->Index()));
			_walletManager = WalletManagerPtr(
				new SpvService(_parent->GetID(), _info->GetChainID(), subAccount, subWalletDBPath,
							   _info->GetEarliestPeerTime(), _config, netType));

			_walletManager->RegisterWalletListener(this);
			_walletManager->RegisterPeerManagerListener(this);

			WalletPtr wallet = _walletManager->GetWallet();

			wallet->SetFeePerKb(_config->FeePerKB());
		}

		SubWallet::SubWallet(const std::string &netType,
							 MasterWallet *parent,
							 const ChainConfigPtr &config,
							 const CoinInfoPtr &info) :
			PeerManager::Listener(),
			_parent(parent),
			_info(info),
			_config(config),
			_callback(nullptr) {

		}

		SubWallet::~SubWallet() {

		}

		std::string SubWallet::GetChainID() const {
			return _info->GetChainID();
		}

		const std::string &SubWallet::GetInfoChainID() const {
			return _info->GetChainID();
		}

		const SubWallet::WalletManagerPtr &SubWallet::GetWalletManager() const {
			return _walletManager;
		}

		nlohmann::json SubWallet::GetBalanceInfo() const {
			ArgInfo("{} {}", _walletManager->GetWallet()->GetWalletID(), GetFunName());

			nlohmann::json info = _walletManager->GetWallet()->GetBalanceInfo();

			ArgInfo("r => {}", info.dump());
			return info;
		}

		std::string SubWallet::GetBalance() const {
			ArgInfo("{} {}", _walletManager->GetWallet()->GetWalletID(), GetFunName());

			std::string balance = _walletManager->GetWallet()->GetBalance(Asset::GetELAAssetID()).getDec();

			ArgInfo("r => {}", balance);

			return balance;
		}

		std::string SubWallet::CreateAddress() {
			ArgInfo("{} {}", _walletManager->GetWallet()->GetWalletID(), GetFunName());

			std::string address = _walletManager->GetWallet()->GetReceiveAddress()->String();

			ArgInfo("r => {}", address);

			return address;
		}

		nlohmann::json SubWallet::GetAllAddress(uint32_t start, uint32_t count, bool internal) const {
			ArgInfo("{} {}", _walletManager->GetWallet()->GetWalletID(), GetFunName());
			ArgInfo("start: {}", start);
			ArgInfo("count: {}", count);

			nlohmann::json j;
			AddressArray addresses;
			size_t maxCount = _walletManager->GetWallet()->GetAllAddresses(addresses, start, count, internal);

			std::vector<std::string> addrString;
			for (size_t i = 0; i < addresses.size(); ++i) {
				addrString.push_back(addresses[i]->String());
			}

			j["Addresses"] = addrString;
			j["MaxCount"] = maxCount;

			ArgInfo("r => {}", j.dump());
			return j;
		}

		nlohmann::json SubWallet::GetAllPublicKeys(uint32_t start, uint32_t count) const {
			ArgInfo("{} {}", _walletManager->GetWallet()->GetWalletID(), GetFunName());
			ArgInfo("start: {}", start);
			ArgInfo("count: {}", count);

			nlohmann::json j;
			std::vector<bytes_t> publicKeys;
			size_t maxCount = _walletManager->GetWallet()->GetAllPublickeys(publicKeys, start, count, false);

			std::vector<std::string> pubKeyString;
			for (size_t i = 0; i < publicKeys.size(); ++i) {
				pubKeyString.push_back(publicKeys[i].getHex());
			}

			j["PublicKeys"] = pubKeyString;
			j["MaxCount"] = maxCount;

			ArgInfo("r => {}", j.dump());
			return j;
		}

		std::string SubWallet::GetBalanceWithAddress(const std::string &address) const {
			ArgInfo("{} {}", _walletManager->GetWallet()->GetWalletID(), GetFunName());
			ArgInfo("addr: {}", address);

			std::string balance = _walletManager->GetWallet()->GetBalanceWithAddress(Asset::GetELAAssetID(),
																					 address).getDec();

			ArgInfo("r => {}", balance);
			return balance;
		}

		void SubWallet::AddCallback(ISubWalletCallback *subCallback) {
			ArgInfo("{} {}", _walletManager->GetWallet()->GetWalletID(), GetFunName());
			ArgInfo("callback: *");

			boost::mutex::scoped_lock scoped_lock(lock);

			if (_callback != nullptr) {
				Log::warn("{} callback registered, ignore", _walletManager->GetWallet()->GetWalletID());
			} else {
				_callback = subCallback;

				const PeerManagerPtr &peerManager = _walletManager->GetPeerManager();

				uint32_t currentHeight = peerManager->GetLastBlockHeight();
				uint32_t lastBlockTime = peerManager->GetLastBlockTimestamp();
				uint32_t progress = (uint32_t) (peerManager->GetSyncProgress(0) * 100);

				nlohmann::json j;
				j["Progress"] = progress;
				j["LastBlockTime"] = lastBlockTime;
				j["BytesPerSecond"] = 0;
				j["DownloadPeer"] = "";

				_callback->OnBlockSyncProgress(j);
				ArgInfo("add callback done");
			}
		}

		void SubWallet::RemoveCallback() {
			ArgInfo("{} {}", _walletManager->GetWallet()->GetWalletID(), GetFunName());
			boost::mutex::scoped_lock scoped_lock(lock);

			_callback = nullptr;

			ArgInfo("remove callback done");
		}

		TransactionPtr SubWallet::CreateConsolidateTx(const std::string &memo, const uint256 &asset) const {
			std::string m;

			if (!memo.empty())
				m = "type:text,msg:" + memo;

			TransactionPtr tx = _walletManager->GetWallet()->Consolidate(m, asset);

			if (_info->GetChainID() == "ELA")
				tx->SetVersion(Transaction::TxVersion::V09);

			tx->FixIndex();

			return tx;
		}

		void SubWallet::EncodeTx(nlohmann::json &result, const TransactionPtr &tx) const {
			ByteStream stream;
			tx->Serialize(stream, true);
			const bytes_t &hex = stream.GetBytes();

			result["Algorithm"] = "base64";
			result["ID"] = tx->GetHash().GetHex().substr(0, 8);
			result["Data"] = hex.getBase64();
			result["ChainID"] = GetChainID();
			result["Fee"] = tx->GetFee();
		}

		TransactionPtr SubWallet::DecodeTx(const nlohmann::json &encodedTx) const {
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

			TransactionPtr tx;
			if (GetChainID() == CHAINID_MAINCHAIN) {
				tx = TransactionPtr(new Transaction());
			} else if (GetChainID() == CHAINID_IDCHAIN || GetChainID() == CHAINID_TOKENCHAIN) {
				tx = TransactionPtr(new IDTransaction());
			}

			bytes_t rawHex;
			if (algorithm == "base64") {
				rawHex.setBase64(data);
			} else {
				ErrorChecker::CheckCondition(true, Error::InvalidArgument, "Decode tx with unknown algorithm");
			}

			ByteStream stream(rawHex);
			ErrorChecker::CheckParam(!tx->Deserialize(stream, true), Error::InvalidArgument,
									 "Invalid input: deserialize fail");

			SPVLOG_DEBUG("decoded tx: {}", tx->ToJson().dump(4));
			return tx;
		}

		nlohmann::json SubWallet::CreateTransaction(const std::string &fromAddress, const std::string &targetAddress,
													const std::string &amount, const std::string &memo) {

			WalletPtr wallet = _walletManager->GetWallet();
			ArgInfo("{} {}", wallet->GetWalletID(), GetFunName());
			ArgInfo("fromAddr: {}", fromAddress);
			ArgInfo("targetAddr: {}", targetAddress);
			ArgInfo("amount: {}", amount);
			ArgInfo("memo: {}", memo);

			ErrorChecker::CheckBigIntAmount(amount);
			bool max = false;
			BigInt bnAmount;
			if (amount == "-1") {
				max = true;
				bnAmount = 0;
			} else {
				bnAmount.setDec(amount);
			}

			OutputArray outputs;
			Address receiveAddr(targetAddress);
			outputs.push_back(OutputPtr(new TransactionOutput(bnAmount, receiveAddr)));
			AddressPtr fromAddr(new Address(fromAddress));

			PayloadPtr payload = PayloadPtr(new TransferAsset());
			TransactionPtr tx = wallet->CreateTransaction(Transaction::transferAsset,
														  payload, fromAddr, outputs, memo, max);

			nlohmann::json result;
			EncodeTx(result, tx);

			ArgInfo("r => {}", result.dump());
			return result;
		}

		nlohmann::json SubWallet::SignTransaction(const nlohmann::json &tx,
												  const std::string &payPassword) const {

			ArgInfo("{} {}", _walletManager->GetWallet()->GetWalletID(), GetFunName());
			ArgInfo("tx: {}", tx.dump());
			ArgInfo("passwd: *");

			TransactionPtr txn = DecodeTx(tx);

			_walletManager->GetWallet()->SignTransaction(txn, payPassword);

			nlohmann::json result;
			EncodeTx(result, txn);

			ArgInfo("r => {}", result.dump());
			return result;
		}

		nlohmann::json SubWallet::PublishTransaction(const nlohmann::json &tx) {
			ArgInfo("{} {}", _walletManager->GetWallet()->GetWalletID(), GetFunName());
			ArgInfo("tx: {}", tx.dump());

			TransactionPtr txn = DecodeTx(tx);

			publishTransaction(txn);

			nlohmann::json result;
			result["TxHash"] = txn->GetHash().GetHex();
			result["Fee"] = txn->GetFee();

			ArgInfo("r => {}", result.dump());
			return result;
		}

		std::string SubWallet::ConvertToRawTransaction(const nlohmann::json &tx) {
			ArgInfo("{} {}", _walletManager->GetWallet()->GetWalletID(), GetFunName());
			ArgInfo("tx: {}", tx.dump());

			TransactionPtr txn = DecodeTx(tx);
			ByteStream stream;
			txn->Serialize(stream, false);
			std::string rawtx = stream.GetBytes().getHex();

			ArgInfo("r => {}", rawtx);

			return rawtx;
		}

		nlohmann::json SubWallet::GetAllUTXOs(uint32_t start, uint32_t count, const std::string &address) const {
			ArgInfo("{} {}", _walletManager->GetWallet()->GetWalletID(), GetFunName());
			ArgInfo("start: {}", start);
			ArgInfo("count: {}", count);
			ArgInfo("addr: {}", address);
			size_t maxCount = 0, pageCount = 0;

			const WalletPtr &wallet = _walletManager->GetWallet();

			std::vector<UTXOPtr> UTXOs = wallet->GetAllUTXO(address);

			maxCount = UTXOs.size();
			nlohmann::json j, jutxos;

			for (size_t i = start; i < UTXOs.size() && pageCount < count; ++i) {
				nlohmann::json item;
				item["Hash"] = UTXOs[i]->Hash().GetHex();
				item["Index"] = UTXOs[i]->Index();
				item["Amount"] = UTXOs[i]->Output()->Amount().getDec();
				jutxos.push_back(item);
				pageCount++;
			}

			j["MaxCount"] = maxCount;
			j["UTXOs"] = jutxos;

			ArgInfo("r => {}", j.dump());
			return j;
		}

		nlohmann::json SubWallet::CreateConsolidateTransaction(const std::string &memo) {
			ArgInfo("{} {}", _walletManager->GetWallet()->GetWalletID(), GetFunName());
			ArgInfo("memo: {}", memo);

			TransactionPtr tx = CreateConsolidateTx(memo, Asset::GetELAAssetID());

			nlohmann::json result;
			EncodeTx(result, tx);

			ArgInfo("r => {}", result.dump());
			return result;
		}

		nlohmann::json SubWallet::GetAllTransactionCommon(uint32_t start, uint32_t count, const std::string &txid,
														  TxnType type) const {

			nlohmann::json j;
			uint32_t confirms = 0;
			std::vector<nlohmann::json> jsonList;
			const WalletPtr &wallet = _walletManager->GetWallet();
			TransactionPtr txFound;

			std::vector<TransactionPtr> txnPending = wallet->LoadTxn(TXN_PENDING);
			for (std::vector<TransactionPtr>::iterator it = txnPending.begin(); it != txnPending.end();) {
				if (((type & TXN_NORMAL) == TXN_NORMAL && (*it)->IsCoinBase()) ||
					((type & TXN_COINBASE) == TXN_COINBASE && !(*it)->IsCoinBase())) {
					it = txnPending.erase(it);
				} else {
					if (txid == (*it)->GetHash().GetHex())
						txFound = *it;
					++it;
				}
			}
			size_t txnNormalCount = _walletManager->GetAllTransactionCount(type);

			j["MaxCount"] = txnNormalCount + txnPending.size();

			if (!txid.empty()) {
				j["Transactions"] = {};
				if (txFound == nullptr) {
					uint256 txHash(txid);
					txFound = wallet->LoadTxn(txHash);
				}
				if (txFound) {
					confirms = txFound->GetConfirms(wallet->LastBlockHeight());
					jsonList.push_back(txFound->GetSummary(wallet, confirms, true));
					j["Transactions"] = jsonList;
				}
			} else {
				size_t realCnt, cur;
				for (realCnt = 0, cur = start; cur < txnPending.size() && realCnt < count; ++realCnt, ++cur) {
					confirms = txnPending[realCnt]->GetConfirms(wallet->LastBlockHeight());
					jsonList.push_back(txnPending[realCnt]->GetSummary(wallet, confirms, false));
				}
				if (realCnt < count) {
					size_t offset = cur - txnPending.size();
					std::vector<TransactionPtr> txns = _walletManager->LoadTxnDesc(_info->GetChainID(), type,
																				   offset, count - realCnt);
					for (size_t i = 0; i < txns.size(); ++i) {
						confirms = txns[i]->GetConfirms(wallet->LastBlockHeight());
						jsonList.push_back(txns[i]->GetSummary(wallet, confirms, false));
					}
				}
				j["Transactions"] = jsonList;
			}

			return j;
		}

		nlohmann::json SubWallet::GetAllTransaction(uint32_t start, uint32_t count, const std::string &txid) const {
			ArgInfo("{} {}", _walletManager->GetWallet()->GetWalletID(), GetFunName());
			ArgInfo("start: {}", start);
			ArgInfo("count: {}", count);
			ArgInfo("txid: {}", txid);

			nlohmann::json j = GetAllTransactionCommon(start, count, txid, TXN_NORMAL);

			ArgInfo("r => {}", j.dump());
			return j;
		}

		nlohmann::json SubWallet::GetAllCoinBaseTransaction(uint32_t start, uint32_t count,
															const std::string &txID) const {
			ArgInfo("{} {}", _walletManager->GetWallet()->GetWalletID(), GetFunName());
			ArgInfo("start: {}", start);
			ArgInfo("count: {}", count);
			ArgInfo("txID: {}", txID);

			nlohmann::json j = GetAllTransactionCommon(start, count, txID, TXN_COINBASE);

			ArgInfo("r => {}", j.dump());
			return j;
		}

		void SubWallet::publishTransaction(const TransactionPtr &tx) {
			_walletManager->PublishTransaction(tx);
		}

		void SubWallet::onBalanceChanged(const uint256 &assetID, const BigInt &balance) {
			ArgInfo("{} {} Balance: {}", _walletManager->GetWallet()->GetWalletID(), GetFunName(), balance.getDec());
			boost::mutex::scoped_lock scoped_lock(lock);

			if (_callback) {
				_callback->OnBalanceChanged(assetID.GetHex(), balance.getDec());
			} else {
				Log::warn("{} callback not register", _walletManager->GetWallet()->GetWalletID());
			}
		}

		void SubWallet::onTxAdded(const TransactionPtr &tx) {
			const uint256 &txHash = tx->GetHash();
			ArgInfo("{} {} Hash: {}, h: {}", _walletManager->GetWallet()->GetWalletID(), GetFunName(), txHash.GetHex(),
					tx->GetBlockHeight());

			fireTransactionStatusChanged(txHash, "Added", nlohmann::json(), 0);
		}

		void SubWallet::onTxUpdated(const std::vector<TransactionPtr> &txns) {
			ArgInfo("{} {} size: {}, height: {}, timestamp: {}", _walletManager->GetWallet()->GetWalletID(),
					GetFunName(), txns.size(), txns.front()->GetBlockHeight(), txns.front()->GetTimestamp());

			uint32_t walletBlockHeight = _walletManager->GetWallet()->LastBlockHeight();
			for (const TransactionPtr tx : txns) {
				uint32_t confirm = tx->GetConfirms(walletBlockHeight);

				fireTransactionStatusChanged(tx->GetHash(), "Updated", nlohmann::json(), confirm);
			}
		}

		void SubWallet::onTxDeleted(const TransactionPtr &tx, bool notifyUser, bool recommendRescan) {
			ArgInfo("{} {} hash: {}, notify: {}, rescan: {}", _walletManager->GetWallet()->GetWalletID(), GetFunName(),
					tx->GetHash().GetHex(), notifyUser, recommendRescan);

			fireTransactionStatusChanged(tx->GetHash(), "Deleted", nlohmann::json(), 0);
		}

		void SubWallet::onAssetRegistered(const AssetPtr &asset, uint64_t amount, const uint168 &controller) {
			ArgInfo("{} {} asset: {}, amount: {}",
					_walletManager->GetWallet()->GetWalletID(), GetFunName(),
					asset->GetName(), amount, controller.GetHex());

			boost::mutex::scoped_lock scoped_lock(lock);

			if (_callback) {
				_callback->OnAssetRegistered(asset->GetHash().GetHex(), asset->ToJson());
			} else {
				Log::warn("{} callback not register", _walletManager->GetWallet()->GetWalletID());
			}
		}

		void SubWallet::syncStarted() {
		}

		void SubWallet::syncProgress(uint32_t progress, time_t lastBlockTime, uint32_t bytesPerSecond,
									 const std::string &downloadPeer) {
			struct tm tm;

			localtime_r(&lastBlockTime, &tm);
			char timeString[100] = {0};
			strftime(timeString, sizeof(timeString), "%F %T", &tm);
			ArgInfo("{} {} [{}] [{}] [{}%  {} Bytes / s]", _walletManager->GetWallet()->GetWalletID(), GetFunName(),
					downloadPeer, timeString, progress, bytesPerSecond);

			nlohmann::json j;
			j["Progress"] = progress;
			j["LastBlockTime"] = lastBlockTime;
			j["BytesPerSecond"] = bytesPerSecond;
			j["DownloadPeer"] = downloadPeer;

			boost::mutex::scoped_lock scoped_lock(lock);

			if (_callback) {
				_callback->OnBlockSyncProgress(j);
			} else {
				Log::warn("{} callback not register", _walletManager->GetWallet()->GetWalletID());
			}
		}

		void SubWallet::syncStopped(const std::string &error) {
		}

		void SubWallet::saveBlocks(bool replace, const std::vector<MerkleBlockPtr> &blocks) {
		}

		void SubWallet::txPublished(const std::string &hash, const nlohmann::json &result) {
			ArgInfo("{} {} hash: {} reason: {}", _walletManager->GetWallet()->GetWalletID(), GetFunName(), hash, result.dump());

			boost::mutex::scoped_lock scoped_lock(lock);

			if (_callback) {
				_callback->OnTxPublished(hash, result);
			} else {
				Log::warn("{} callback not register", _walletManager->GetWallet()->GetWalletID());
			}
		}

		void SubWallet::connectStatusChanged(const std::string &status) {
			ArgInfo("{} {} status: {}", _walletManager->GetWallet()->GetWalletID(), GetFunName(), status);

			boost::mutex::scoped_lock scopedLock(lock);

			if (_callback) {
				_callback->OnConnectStatusChanged(status);
			} else {
				Log::warn("{} callback not register", _walletManager->GetWallet()->GetWalletID());
			}
		}

		void SubWallet::fireTransactionStatusChanged(const uint256 &txid, const std::string &status,
													 const nlohmann::json &desc, uint32_t confirms) {
			boost::mutex::scoped_lock scoped_lock(lock);

			if (_callback) {
				_callback->OnTransactionStatusChanged(txid.GetHex(), status, desc, confirms);
			} else {
				Log::warn("{} callback not register", _walletManager->GetWallet()->GetWalletID());
			}
		}

		const CoinInfoPtr &SubWallet::GetCoinInfo() const {
			return _info;
		}

		void SubWallet::StartP2P() {
			_walletManager->SyncStart();
		}

		void SubWallet::StopP2P() {
			_walletManager->SyncStop();
			_walletManager->ExecutorStop();
		}

		void SubWallet::FlushData() {
			_walletManager->DatabaseFlush();
		}

		time_t SubWallet::GetFirstTxnTimestamp() const {
			return _walletManager->GetFirstTxnTimestamp();
		}

		bool SubWallet::SetFixedPeer(const std::string &address, uint16_t port) {
			ArgInfo("{} {}", _walletManager->GetWallet()->GetWalletID(), GetFunName());
			ArgInfo("address: {}", address);
			ArgInfo("port: {}", port);
			return _walletManager->GetPeerManager()->SetFixedPeer(address, port);
		}

		void SubWallet::SyncStart() {
			ArgInfo("{} {}", _walletManager->GetWallet()->GetWalletID(), GetFunName());
			_walletManager->SyncStart();
		}

		void SubWallet::SyncStop() {
			ArgInfo("{} {}", _walletManager->GetWallet()->GetWalletID(), GetFunName());
			_walletManager->SyncStop();
		}

		void SubWallet::Resync() {
			ArgInfo("{} {}", _walletManager->GetWallet()->GetWalletID(), GetFunName());
			_walletManager->SyncStop();
			_walletManager->GetWallet()->ClearData();
			_walletManager->GetPeerManager()->ClearData();
			_walletManager->SyncStart();
		}

		nlohmann::json SubWallet::GetBasicInfo() const {
			ArgInfo("{} {}", _walletManager->GetWallet()->GetWalletID(), GetFunName());

			nlohmann::json j;
			j["Info"] = _walletManager->GetWallet()->GetBasicInfo();
			j["ChainID"] = _info->GetChainID();

			ArgInfo("r => {}", j.dump());
			return j;
		}

		nlohmann::json SubWallet::GetTransactionSignedInfo(const nlohmann::json &encodedTx) const {
			ArgInfo("{} {}", _walletManager->GetWallet()->GetWalletID(), GetFunName());
			ArgInfo("tx: {}", encodedTx.dump());

			TransactionPtr tx = DecodeTx(encodedTx);

			nlohmann::json info = tx->GetSignedInfo();

			ArgInfo("r => {}", info.dump());

			return info;
		}

		nlohmann::json SubWallet::GetAssetInfo(const std::string &assetID) const {
			ArgInfo("{} {}", _walletManager->GetWallet()->GetWalletID(), GetFunName());
			ArgInfo("assetID: {}", assetID);

			nlohmann::json info;

			AssetPtr asset = _walletManager->GetWallet()->GetAsset(uint256(assetID));
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
