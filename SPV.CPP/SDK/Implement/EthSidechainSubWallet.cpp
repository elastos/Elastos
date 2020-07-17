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
#include "EthSidechainSubWallet.h"
#include <Common/Log.h>
#include <Ethereum/EthereumClient.h>
#include <ethereum/ewm/BREthereumAccount.h>
#include <WalletCore/CoinInfo.h>
#include <Common/ErrorChecker.h>
#include <Common/hash.h>

namespace Elastos {
	namespace ElaWallet {

const std::string CALLBACK_IS_NULL_PROMPT = "callback is null";

		EthSidechainSubWallet::~EthSidechainSubWallet() {
		}

		EthSidechainSubWallet::EthSidechainSubWallet(const CoinInfoPtr &info,
													 const ChainConfigPtr &config,
													 MasterWallet *parent,
													 const std::string &netType) :
													 _info(info),
													 _config(config),
													 _callback(nullptr),
													 _parent(parent) {

			_walletID = _parent->GetID() + ":" + info->GetChainID();

			AccountPtr account = _parent->GetAccount();
			bytes_t pubkey = account->GetETHSCPubKey();
			if (pubkey.empty()) {
				if (!account->HasMnemonic() || account->Readonly()) {
					ErrorChecker::ThrowParamException(Error::UnsupportOperation, "unsupport operation: ethsc pubkey is empty");
				} else {
					if (account->HasPassphrase()) {
						ErrorChecker::ThrowParamException(Error::Other, "need to call IMasterWallet::VerifyPassPhrase() first");
					} else {
						ErrorChecker::ThrowParamException(Error::Other, "need to call IMasterWallet::VerifyPayPassword() first");
					}
				}
			}

			EthereumNetworkPtr network(new EthereumNetwork(netType));
			_client = ClientPtr(new EthereumClient(this, network, parent->GetDataPath(), pubkey));
			_client->_ewm->getWallet()->setDefaultGasPrice(5000000000);
		}

		std::string EthSidechainSubWallet::GetTransferID(const EthereumTransferPtr &tx) const {
			void *id = tx->getRaw();
			bytes_t tid = sha256(bytes_t(&id, sizeof(id)));
			tid.erase(tid.begin() + 4, tid.end());
			return tid.getHex();
		}

		EthereumTransferPtr EthSidechainSubWallet::LookupTransfer(const std::string &tid) const {
			std::vector<EthereumTransferPtr> transfers = _client->_ewm->getWallet()->getTransfers();
			for (size_t i = 0; i < transfers.size(); ++i) {
				if (tid == GetTransferID(transfers[i]))
					return transfers[i];
			}

			return nullptr;
		}

		void EthSidechainSubWallet::getGasPrice(BREthereumWallet wid, int rid) {
			nlohmann::json j;
			j["Rid"] = rid;
			ArgInfo("{} {}", GetFunName(), j.dump(4));
		}

		void EthSidechainSubWallet::getGasEstimate(BREthereumWallet wid,
												   BREthereumTransfer tid,
												   const std::string &from,
												   const std::string &to,
												   const std::string &amount,
												   const std::string &data,
												   int rid) {
			nlohmann::json j;
			j["From"] = from;
			j["To"] = to;
			j["Amount"] = amount;
			j["Data"] = data;
			j["Rid"] = rid;
			ArgInfo("{} {}", GetFunName(), j.dump(4));
		}

		void EthSidechainSubWallet::getBalance(BREthereumWallet wid, const std::string &address, int rid) {
			nlohmann::json j;
			j["Address"] = address;
			j["Rid"] = rid;
			ArgInfo("{} {}", GetFunName(), j.dump(4));
		}

		void EthSidechainSubWallet::submitTransaction(BREthereumWallet wid,
													  BREthereumTransfer tid,
													  const std::string &rawTransaction,
													  int rid) {
			nlohmann::json j;
			j["RawTx"] = rawTransaction;
			j["Rid"] = rid;
			ArgInfo("{} {}", GetFunName(), j.dump(4));
		}

		void EthSidechainSubWallet::getTransactions(const std::string &address,
													uint64_t begBlockNumber,
													uint64_t endBlockNumber,
													int rid) {
			nlohmann::json j;
			j["Address"] = address;
			j["BegBlockNumber"] = begBlockNumber;
			j["EndBlockNumber"] = endBlockNumber;
			j["Rid"] = rid;
			ArgInfo("{} {}", GetFunName(), j.dump(4));
		}

		void EthSidechainSubWallet::getLogs(const std::string &contract,
											const std::string &address,
											const std::string &event,
											uint64_t begBlockNumber,
											uint64_t endBlockNumber,
											int rid) {
			nlohmann::json j;
			j["Contract"] = contract;
			j["Address"] = address;
			j["Event"] = event;
			j["BegBlockNumber"] = begBlockNumber;
			j["EndBlockNumber"] = endBlockNumber;
			j["Rid"] = rid;
			ArgInfo("{} {}", GetFunName(), j.dump(4));
		}

		void EthSidechainSubWallet::getBlocks(const std::string &address,
											  int interests,
											  uint64_t blockNumberStart,
											  uint64_t blockNumberStop,
											  int rid) {
			nlohmann::json j;
			j["Address"] = address;
			j["Interests"] = interests;
			j["BlockNumberStart"] = blockNumberStart;
			j["BlockNumberStop"] = blockNumberStop;
			j["Rid"] = rid;
			ArgInfo("{} {}", GetFunName(), j.dump(4));
		}

		void EthSidechainSubWallet::getTokens(int rid) {
			nlohmann::json j;
			j["Rid"] = rid;
			ArgInfo("{} {}", GetFunName(), j.dump(4));
		}

		void EthSidechainSubWallet::getBlockNumber(int rid) {
			nlohmann::json j;
			j["Rid"] = rid;
			ArgInfo("{} {}", GetFunName(), j.dump(4));
		}

		void EthSidechainSubWallet::getNonce(const std::string &address, int rid) {
			nlohmann::json j;
			j["Address"] = address;
			j["Rid"] = rid;
			ArgInfo("{} {}", GetFunName(), j.dump(4));
		}

		void EthSidechainSubWallet::handleEWMEvent(EthereumEWM::EWMEvent event, EthereumEWM::Status status,
												   const std::string &errorDescription) {
			nlohmann::json eJson;
			eJson["Type"] = "EWMEvent";
			eJson["Event"] = EthereumEWM::EWMEvent2String(event);
			eJson["Status"] = EthereumEWM::Status2String(status);
			eJson["ErrorDescription"] = errorDescription;
			ArgInfo("{} {}", GetFunName(), eJson.dump(4));

			boost::mutex::scoped_lock scoped_lock(lock);
			if (_callback != nullptr) {
				_callback->OnETHSCEventHandled(eJson);
			} else {
				Log::info(CALLBACK_IS_NULL_PROMPT);
			}
		}

		void EthSidechainSubWallet::handlePeerEvent(EthereumEWM::PeerEvent event, EthereumEWM::Status status,
													const std::string &errorDescription) {
			nlohmann::json eJson;
			eJson["Type"] = "PeerEvent";
			eJson["Event"] = EthereumEWM::PeerEvent2String(event);
			eJson["Status"] = EthereumEWM::Status2String(status);
			eJson["ErrorDescription"] = errorDescription;
			ArgInfo("{} {}", GetFunName(), eJson.dump(4));

			boost::mutex::scoped_lock scoped_lock(lock);
			if (_callback != nullptr) {
				_callback->OnETHSCEventHandled(eJson);
			} else {
				Log::info(CALLBACK_IS_NULL_PROMPT);
			}
		}

		void EthSidechainSubWallet::handleWalletEvent(const EthereumWalletPtr &wallet,
													  EthereumEWM::WalletEvent event,
													  EthereumEWM::Status status,
													  const std::string &errorDescription) {
			nlohmann::json eJson;
			eJson["Type"] = "WalletEvent";
			eJson["Event"] = EthereumEWM::WalletEvent2String(event);
			eJson["Status"] = EthereumEWM::Status2String(status);
			eJson["ErrorDescription"] = errorDescription;
			eJson["WalletSymbol"] = wallet->getSymbol();
			ArgInfo("{} {}", GetFunName(), eJson.dump(4));

			boost::mutex::scoped_lock scoped_lock(lock);
			if (_callback != nullptr) {
				_callback->OnETHSCEventHandled(eJson);
			} else {
				Log::info(CALLBACK_IS_NULL_PROMPT);
			}
		}

		void EthSidechainSubWallet::handleTokenEvent(const EthereumTokenPtr &token, EthereumEWM::TokenEvent event) {
			nlohmann::json eJson;
			eJson["Type"] = "TokenEvent";
			eJson["Event"] = EthereumEWM::TokenEvent2String(event);
			eJson["WalletSymbol"] = token->getSymbol();
			ArgInfo("{} {}", GetFunName(), eJson.dump(4));

			boost::mutex::scoped_lock scoped_lock(lock);
			if (_callback != nullptr) {
				_callback->OnETHSCEventHandled(eJson);
			} else {
				Log::info(CALLBACK_IS_NULL_PROMPT);
			}
		}

		void EthSidechainSubWallet::handleBlockEvent(const EthereumBlockPtr &block,
													 EthereumEWM::BlockEvent event,
													 EthereumEWM::Status status,
													 const std::string &errorDescription) {
			nlohmann::json eJson;
			eJson["Type"] = "BlockEvent";
			eJson["Event"] = EthereumEWM::BlockEvent2String(event);
			eJson["Status"] = EthereumEWM::Status2String(status);
			eJson["ErrorDescription"] = errorDescription;
			eJson["BlockNumber"] = block->getBlockNumber();
			ArgInfo("{} {}", GetFunName(), eJson.dump(4));

			boost::mutex::scoped_lock scoped_lock(lock);
			if (_callback != nullptr) {
				_callback->OnETHSCEventHandled(eJson);
			} else {
				Log::info(CALLBACK_IS_NULL_PROMPT);
			}
		}

		void EthSidechainSubWallet::handleTransferEvent(const EthereumWalletPtr &wallet,
														const EthereumTransferPtr &transaction,
														EthereumEWM::TransactionEvent event,
														EthereumEWM::Status status,
														const std::string &errorDescription) {
			nlohmann::json eJson;
			eJson["Type"] = "TransferEvent";
			eJson["Event"] = EthereumEWM::TransactionEvent2String(event);
			eJson["Status"] = EthereumEWM::Status2String(status);
			eJson["ErrorDescription"] = errorDescription;
			eJson["WalletSymbol"] = wallet->getSymbol();
			eJson["TxHash"] = transaction->getIdentifier();
			ArgInfo("{} {}", GetFunName(), eJson.dump(4));

			boost::mutex::scoped_lock scoped_lock(lock);
			if (_callback != nullptr) {
				_callback->OnETHSCEventHandled(eJson);
			} else {
				Log::info(CALLBACK_IS_NULL_PROMPT);
			}
		}

		std::string EthSidechainSubWallet::GetChainID() const {
			return _info->GetChainID();
		}

		nlohmann::json EthSidechainSubWallet::GetBasicInfo() const {
			ArgInfo("{} {}", _walletID, GetFunName());

			EthereumWalletPtr wallet = _client->_ewm->getWallet();
			nlohmann::json j, jinfo;

			jinfo["Symbol"] = wallet->getSymbol();
			jinfo["GasLimit"] = wallet->getDefaultGasLimit();
			jinfo["GasPrice"] = wallet->getDefaultGasPrice();
			jinfo["Account"] = wallet->getAccount()->getPrimaryAddress();
			jinfo["HoldsEther"] = wallet->walletHoldsEther();

			j["Info"] = jinfo;
			j["ChainID"] = _info->GetChainID();

			ArgInfo("r => {}", j.dump());
			return j;
		}

		nlohmann::json EthSidechainSubWallet::GetBalanceInfo() const {
			ArgInfo("{} {}", _walletID, GetFunName());

			nlohmann::json j;
			j["Info"] = "not ready";
			j["Summary"] = nlohmann::json();

			ArgInfo("r => {}", j.dump());
			return j;
		}

		std::string EthSidechainSubWallet::GetBalance() const {
			ArgInfo("{} {}", _walletID, GetFunName());

			std::string balance = _client->_ewm->getWallet()->getBalance();

			ArgInfo("r => {}", balance);
			return balance;
		}

		std::string EthSidechainSubWallet::GetBalanceWithAddress(const std::string &address) const {
			ArgInfo("{} {}", _walletID, GetFunName());
			ArgInfo("addr: {}", address);

			std::string primaryAddress = _client->_ewm->getWallet()->getAccount()->getPrimaryAddress();
			std::string balance = "0";
			if (primaryAddress == address)
				balance = _client->_ewm->getWallet()->getBalance();

			ArgInfo("r => {}", balance);
			return balance;
		}

		std::string EthSidechainSubWallet::CreateAddress() {
			ArgInfo("{} {}", _walletID, GetFunName());

			std::string addr = _client->_ewm->getWallet()->getAccount()->getPrimaryAddress();

			ArgInfo("r => {}", addr);
			return addr;
		}

		nlohmann::json EthSidechainSubWallet::GetAllAddress(uint32_t start, uint32_t count, bool internal) const {
			ArgInfo("{} {}", _walletID, GetFunName());
			ArgInfo("start: {}", start);
			ArgInfo("count: {}", count);
			ArgInfo("internal: {}", internal);

			std::vector<std::string> addresses;
			addresses.push_back(_client->_ewm->getWallet()->getAccount()->getPrimaryAddress());
			nlohmann::json j;
			j["Addresses"] = addresses;
			j["MaxCount"] = 1;

			ArgInfo("r => {}", j.dump());
			return j;
		}

		nlohmann::json EthSidechainSubWallet::GetAllPublicKeys(uint32_t start, uint32_t count) const {
			ArgInfo("{} {}", _walletID, GetFunName());
			ArgInfo("s: {}", start);
			ArgInfo("c: {}", count);

			std::vector<std::string> pubkey;
			pubkey.push_back(_client->_ewm->getWallet()->getAccount()->getPrimaryAddressPublicKey().getHex());
			nlohmann::json j;
			j["PublicKeys"] = pubkey;
			j["MaxCount"] = 1;

			ArgInfo("r => {}", j.dump());
			return j;
		}

		void EthSidechainSubWallet::AddCallback(ISubWalletCallback *subCallback) {
			ArgInfo("{} {}", _walletID, GetFunName());
			ArgInfo("callback: *");

			boost::mutex::scoped_lock scoped_lock(lock);
			_callback = subCallback;
		}

		void EthSidechainSubWallet::RemoveCallback() {
			ArgInfo("{} {}", _walletID, GetFunName());
		}

		nlohmann::json EthSidechainSubWallet::CreateTransaction(const std::string &fromAddress,
																const std::string &targetAddress,
																const std::string &amount,
																const std::string &memo) {
			ArgInfo("{} {}", _walletID, GetFunName());
			ArgInfo("from: {}", fromAddress);
			ArgInfo("target: {}", targetAddress);
			ArgInfo("amount: {}", amount);
			ArgInfo("memo: {}", memo);

			nlohmann::json j;
			EthereumTransferPtr tx = _client->_ewm->getWallet()->createTransfer(targetAddress, amount, EthereumAmount::Unit::ETHER_ETHER);

			j["ID"] = GetTransferID(tx);
			j["Fee"] = tx->getFee(EthereumAmount::Unit::ETHER_ETHER);

			ArgInfo("r => {}", j.dump());

			return j;
		}

		nlohmann::json EthSidechainSubWallet::GetAllUTXOs(uint32_t start, uint32_t count,
														  const std::string &address) const {
			ArgInfo("{} {}", _walletID, GetFunName());
			ArgInfo("start: {}", start);
			ArgInfo("cnt: {}", count);
			ArgInfo("addr: {}", address);

			nlohmann::json j;

			ArgInfo("r => {}", j.dump());

			return j;
		}

		nlohmann::json EthSidechainSubWallet::CreateConsolidateTransaction(const std::string &memo) {
			ArgInfo("{} {}", _walletID, GetFunName());
			ArgInfo("memo: {}", memo);

			nlohmann::json j;

			ArgInfo("r => {}", j.dump());
			return j;
		}

		nlohmann::json EthSidechainSubWallet::SignTransaction(const nlohmann::json &tx,
															  const std::string &payPassword) const {
			ArgInfo("{} {}", _walletID, GetFunName());
			ArgInfo("tx: {}", tx.dump());
			ArgInfo("passwd: *");

			std::string tid;
			EthereumTransferPtr transfer;
			if (tx.find("ID") == tx.end())
				ErrorChecker::ThrowParamException(Error::InvalidArgument, "'ID' not found in json");

			try {
				tid = tx["ID"].get<std::string>();
				transfer = LookupTransfer(tid);
			} catch (const std::exception &e) {
				ErrorChecker::ThrowParamException(Error::InvalidArgument, "get 'ID' of json failed");
			}

			ErrorChecker::CheckParam(transfer == nullptr, Error::InvalidArgument, "transfer " + tid + " not found");

			uint512 seed = _parent->GetAccount()->GetSeed(payPassword);
			BRKey prvkey = derivePrivateKeyFromSeed(*(UInt512 *)seed.begin(), 0);
			_client->_ewm->getWallet()->signWithPrivateKey(transfer, prvkey);

			nlohmann::json j = tx;
			j["Hash"] = transfer->getOriginationTransactionHash();

			ArgInfo("r => {}", j.dump());
			return j;
		}

		nlohmann::json EthSidechainSubWallet::GetTransactionSignedInfo(const nlohmann::json &tx) const {
			ArgInfo("{} {}", _walletID, GetFunName());
			ArgInfo("tx: {}", tx.dump());

			nlohmann::json j;

			ArgInfo("r => {}", j.dump());

			return j;
		}

		nlohmann::json EthSidechainSubWallet::PublishTransaction(const nlohmann::json &tx) {
			ArgInfo("{} {}", _walletID, GetFunName());
			ArgInfo("tx: {}", tx.dump());

			std::string tid;
			EthereumTransferPtr transfer;
			if (tx.find("ID") == tx.end())
				ErrorChecker::ThrowParamException(Error::InvalidArgument, "'ID' not found in json");

			try {
				tid = tx["ID"].get<std::string>();
				transfer = LookupTransfer(tid);
			} catch (const std::exception &e) {
				ErrorChecker::ThrowParamException(Error::InvalidArgument, "get 'ID' of json failed");
			}
			ErrorChecker::CheckParam(transfer == nullptr, Error::InvalidArgument, "transfer " + tid + " not found");

			_client->_ewm->getWallet()->submit(transfer);

			nlohmann::json j = tx;
			j["Hash"] = transfer->getOriginationTransactionHash();

			ArgInfo("r => {}", j.dump());
			return j;
		}

		std::string EthSidechainSubWallet::ConvertToRawTransaction(const nlohmann::json &tx) {
			ArgInfo("{} {}", _walletID, GetFunName());
			ArgInfo("tx: {}", tx.dump());

			ArgInfo("r => ");

			return "";
		}

		nlohmann::json EthSidechainSubWallet::GetAllTransaction(uint32_t start, uint32_t count,
																const std::string &txid) const {
			ArgInfo("{} {}", _walletID, GetFunName());
			ArgInfo("start: {}, cnt: {}, txid: {}", start, count, txid);

			nlohmann::json j, jtx;
			std::vector<nlohmann::json> txList;

			std::vector<EthereumTransferPtr> transfers = _client->_ewm->getWallet()->getTransfers();
			if (!txid.empty()) {
				start = 0;
				count = transfers.size();
			}

			for (size_t i = start; i < transfers.size() && i - start < count; ++i) {
				std::string transferID = GetTransferID(transfers[i]);
				if (txid.empty() || txid == transferID || txid == transfers[i]->getIdentifier()) {
					jtx["ID"] = transferID;
					jtx["IsConfirmed"] = transfers[i]->isConfirmed();
					jtx["IsSubmitted"] = transfers[i]->isSubmitted();
					jtx["IsErrored"] = transfers[i]->isErrored();
					jtx["ErrorDesc"] = transfers[i]->isErrored() ? transfers[i]->getErrorDescription() : "";
					jtx["Hash"] = transfers[i]->getIdentifier();
					jtx["OrigTxHash"] = transfers[i]->getOriginationTransactionHash();
					jtx["Amount"] = transfers[i]->getAmount(EthereumAmount::ETHER_ETHER);
					jtx["Timestamp"] = transfers[i]->getBlockTimestamp();
					jtx["Fee"] = transfers[i]->getFee(EthereumAmount::ETHER_ETHER);
					jtx["Confirmations"] = transfers[i]->getBlockConfirmations();
					jtx["GasPrice"] = transfers[i]->getGasPrice();
					jtx["GasLimit"] = transfers[i]->getGasLimit();
					jtx["GasUsed"] = transfers[i]->getGasUsed();
					jtx["BlockNumber"] = transfers[i]->getBlockNumber();
					jtx["SourceAddress"] = transfers[i]->getSourceAddress();
					jtx["TargetAddress"] = transfers[i]->getTargetAddress();
					txList.push_back(jtx);

					if (!txid.empty())
						break;
				}
			}

			j["MaxCount"] = transfers.size();
			j["Transactions"] = txList;

			ArgInfo("r => {}", j.dump());

			return j;
		}

		nlohmann::json EthSidechainSubWallet::GetAllCoinBaseTransaction(uint32_t start, uint32_t count,
																		const std::string &txID) const {
			ArgInfo("{} {}", _walletID, GetFunName());
			ArgInfo("start: {}, cnt: {}, txid: {}", start, count, txID);

			return nlohmann::json();
		}

		nlohmann::json EthSidechainSubWallet::GetAssetInfo(const std::string &assetID) const {
			ArgInfo("{} {}", _walletID, GetFunName());
			ArgInfo("asset: {}", assetID);

			nlohmann::json j;

			ArgInfo("r => {}", j.dump());
			return j;
		}

		bool EthSidechainSubWallet::SetFixedPeer(const std::string &address, uint16_t port) {
			ArgInfo("{} {}", _walletID, GetFunName());
			ArgInfo("addr: {}, port: {}", address, port);

			ArgInfo("r => false");
			return false;
		}

		void EthSidechainSubWallet::SyncStart() {
			ArgInfo("{} {}", _walletID, GetFunName());
			_client->_ewm->connect();
		}

		void EthSidechainSubWallet::SyncStop() {
			ArgInfo("{} {}", _walletID, GetFunName());
			_client->_ewm->disconnect();
		}

		void EthSidechainSubWallet::Resync() {
			ArgInfo("{} {}", _walletID, GetFunName());

		}

		void EthSidechainSubWallet::StartP2P() {
			_client->_ewm->connect();
		}

		void EthSidechainSubWallet::StopP2P() {
			_client->_ewm->disconnect();
		}

		void EthSidechainSubWallet::FlushData() {

		}

	}
}