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
#include <ethereum/ewm/BREthereumClient.h>

namespace Elastos {
	namespace ElaWallet {

const std::string CALLBACK_IS_NULL_PROMPT = "callback is null";

		EthSidechainSubWallet::~EthSidechainSubWallet() {
		}

		nlohmann::json EthSidechainSubWallet::CreateTransfer(const std::string &targetAddress,
															 const std::string &amount,
															 EthereumAmountUnit amountUnit) const {
			ArgInfo("{} {}", _walletID, GetFunName());
			ArgInfo("target: {}", targetAddress);
			ArgInfo("amount: {}", amount);
			ArgInfo("amountUnit: {}", amountUnit);

			if (amountUnit != TOKEN_DECIMAL &&
				amountUnit != TOKEN_INTEGER &&
				amountUnit != ETHER_WEI &&
				amountUnit != ETHER_GWEI &&
				amountUnit != ETHER_ETHER) {
				ErrorChecker::ThrowParamException(Error::InvalidArgument, "invalid amount unit");
			}

			EthereumAmount::Unit unit = EthereumAmount::Unit(amountUnit);
			nlohmann::json j;
			EthereumTransferPtr tx = _client->_ewm->getWallet()->createTransfer(targetAddress, amount, unit);

			j["ID"] = GetTransferID(tx);
			j["Fee"] = tx->getFee(unit);

			ArgInfo("r => {}", j.dump());

			return j;
		}

		nlohmann::json EthSidechainSubWallet::CreateTransferGeneric(const std::string &targetAddress,
																	const std::string &amount,
																	EthereumAmountUnit amountUnit,
																	const std::string &gasPrice,
																	EthereumAmountUnit gasPriceUnit,
																	const std::string &gasLimit,
																	const std::string &data) const {
			ArgInfo("{} {}", _walletID, GetFunName());
			ArgInfo("target: {}", targetAddress);
			ArgInfo("amount: {}", amount);
			ArgInfo("amountUnit: {}", amountUnit);
			ArgInfo("gasPrice: {}", gasPrice);
			ArgInfo("gasPriceUnit: {}", gasPriceUnit);
			ArgInfo("gasLimit: {}", gasLimit);
			ArgInfo("data: {}", data);

			if (amountUnit != TOKEN_DECIMAL &&
				amountUnit != TOKEN_INTEGER &&
				amountUnit != ETHER_WEI &&
				amountUnit != ETHER_GWEI &&
				amountUnit != ETHER_ETHER) {
				ErrorChecker::ThrowParamException(Error::InvalidArgument, "invalid amount unit");
			}

			EthereumAmount::Unit unit = EthereumAmount::Unit(amountUnit);
			EthereumAmount::Unit gasUnit = EthereumAmount::Unit(gasPriceUnit);
			nlohmann::json j;
			EthereumTransferPtr tx = _client->_ewm->getWallet()->createTransferGeneric(targetAddress,
																					   amount,
																					   unit,
																					   gasPrice,
																					   gasUnit,
																					   gasLimit,
																					   data);

			j["ID"] = GetTransferID(tx);
			j["Fee"] = tx->getFee(unit);

			ArgInfo("r => {}", j.dump());

			return j;
		}

		void EthSidechainSubWallet::DeleteTransfer(const nlohmann::json &tx) {
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

			_client->_ewm->transferDelete(transfer);
		}

		nlohmann::json EthSidechainSubWallet::GetTokenTransactions(uint32_t start, uint32_t count,
																   const std::string &txid,
																   const std::string &tokenSymbol) const {
			ArgInfo("{} {}", _walletID, GetFunName());
			ArgInfo("start: {}", start);
			ArgInfo("count: {}", count);
			ArgInfo("txid: {}", txid);
			ArgInfo("tokenSymbol: {}", tokenSymbol);

			std::vector<EthereumWalletPtr> wallets = _client->_ewm->getWallets();
			nlohmann::json j;
			nlohmann::json txList = nlohmann::json::array();
			size_t maxCount = 0;

			for (const auto &w : wallets) {
				EthereumTokenPtr token = w->getToken();
				if (token && (token->getSymbol() == tokenSymbol)) {
					std::vector<EthereumTransferPtr> transfers = w->getTransfers();
					std::reverse(transfers.begin(), transfers.end());
					maxCount = transfers.size();
					for (size_t i = start; i < transfers.size() && i - start < count; ++i) {
						const EthereumTransferPtr &transfer = transfers[i];
						std::string transferID = GetTransferID(transfer);
						if (txid.empty() || txid == transferID || txid == transfer->getIdentifier()) {
							nlohmann::json jtx = transfer->ToJson();
							jtx["ID"] = transferID;
							txList.push_back(jtx);

							if (!txid.empty())
								break;
						}
					}
				}
			}

			j["MaxCount"] = maxCount;
			j["Transactions"] = txList;

			ArgInfo("r => {}", j.dump());
			return j;
		}

        void EthSidechainSubWallet::SyncStart() {
            StartP2P();
		}

        void EthSidechainSubWallet::SyncStop() {
            StopP2P();
		}

		EthSidechainSubWallet::EthSidechainSubWallet(const CoinInfoPtr &info,
													 const ChainConfigPtr &config,
													 MasterWallet *parent,
													 const std::string &netType) :
													 _info(info),
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


            BREthereumNetwork net = NULL;
            if (info->GetChainID() == "ETHDID") {
                if (netType == "MainNet") {
                    net = ethereumDIDMainnet;
                } else if (netType == "TestNet") {
                    net = ethereumDIDTestnet;
                } else if (netType == "RegTest") {
                    net = ethereumDIDRinkeby;
                } else if (netType == "PrvNet") {
                    net = ethereumDIDPrvnet;
                } else {
                    net = NULL;
                }
            } else if (info->GetChainID() == "ETHHECO") {
                if (netType == "MainNet") {
                    net = ethereumHecoMainnet;
                } else if (netType == "TestNet") {
                    net = ethereumHecoTestnet;
                } else if (netType == "RegTest") {
                    net = ethereumHecoRinkeby;
                } else if (netType == "PrvNet") {
                    net = ethereumHecoPrvnet;
                } else {
                    net = NULL;
                }
            } else {
                if (netType == "MainNet") {
                    net = ethereumMainnet;
                } else if (netType == "TestNet") {
                    net = ethereumTestnet;
                } else if (netType == "RegTest") {
                    net = ethereumRinkeby;
                } else if (netType == "PrvNet") {
                    net = ethereumPrvnet;
                } else {
                    net = NULL;
                }
            }
			EthereumNetworkPtr network(new EthereumNetwork(net));
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
			j["rid"] = rid;
			ArgInfo("{} {}", GetFunName(), j.dump(4));

			if (_callback) {
				nlohmann::json r = _callback->GasPrice(rid);
				ArgInfo("r => {}", r.dump(4));

				if (!r.empty()) {
					int id;
					std::string gasPrice;

					try {
						id = r["id"].get<int>();
						gasPrice = r["result"].get<std::string>();
						_client->_ewm->announceGasPrice(wid, gasPrice, id);
					} catch (const std::exception &e) {
						Log::error("invalid json format: {}", e.what());
					}
				}
			}
		}

		void EthSidechainSubWallet::getGasEstimate(BREthereumWallet wid,
												   BREthereumCookie cookie,
												   const std::string &from,
												   const std::string &to,
												   const std::string &amount,
												   const std::string &gasPrice,
												   const std::string &data,
												   int rid) {
			nlohmann::json j;

			j["from"] = from;
			j["to"] = to;
			j["amount"] = amount;
			j["data"] = data;
			j["gasPrice"] = gasPrice;
			j["rid"] = rid;
			ArgInfo("{} {}", GetFunName(), j.dump(4));

			if (_callback) {
				nlohmann::json r = _callback->EstimateGas(from, to, amount, gasPrice, data, rid);
				ArgInfo("r => {}", r.dump(4));

				if (!r.empty()) {
					int id;
					std::string gasEstimate;

					try {
						id = r["id"].get<int>();
						gasEstimate = r["result"].get<std::string>();
						_client->_ewm->announceGasEstimateSuccess(wid, cookie, gasEstimate, gasPrice, id);
					} catch (const std::exception &e) {
						Log::error("invalid json format: {}", e.what());
						_client->_ewm->announceGasEstimateFailure(wid, cookie, ERROR_NODE_NOT_CONNECTED, id);
					}
				} else {
					_client->_ewm->announceGasEstimateFailure(wid, cookie, ERROR_NODE_NOT_CONNECTED, rid);
				}
			}
		}

		void EthSidechainSubWallet::getBalance(BREthereumWallet wid, const std::string &address, int rid) {
			nlohmann::json j;
			j["address"] = address;
			j["rid"] = rid;
			ArgInfo("{} {}", GetFunName(), j.dump(4));

			if (_callback) {
				nlohmann::json r = _callback->GetBalance(address, rid);
				ArgInfo("r => {}", r.dump(4));

				if (!r.empty()) {
					int id;
					std::string balance;

					try {
						id = r["id"].get<int>();
						balance = r["result"].get<std::string>();
						_client->_ewm->announceBalance(wid, balance, id);
					} catch (const std::exception &e) {
						Log::error("invalid json format: {}", e.what());
					}
				}
			}
		}

		void EthSidechainSubWallet::submitTransaction(BREthereumWallet wid,
													  BREthereumTransfer tid,
													  const std::string &tx,
													  int rid) {
			nlohmann::json j;
			j["tx"] = tx;
			j["rid"] = rid;
			ArgInfo("{} {}", GetFunName(), j.dump(4));

			if (_callback) {
				nlohmann::json r = _callback->SubmitTransaction(tx, rid);
				ArgInfo("r => {}", r.dump(4));

				if (!r.empty()) {
					int id = rid;
					std::string hash;

					try {
						id = r["id"].get<int>();
						hash = r["result"].get<std::string>();
						_client->_ewm->announceSubmitTransaction(wid, tid, hash, -1, "", id);
					} catch (const std::exception &e) {
						Log::error("invalid json format: {}", e.what());
						_client->_ewm->announceSubmitTransaction(wid, tid, "", 0, "parse result failure", id);
					}
				} else {
					_client->_ewm->announceSubmitTransaction(wid, tid, "", 0, "unknown failure", rid);
				}
			}
		}

		void EthSidechainSubWallet::getTransactions(const std::string &address,
													uint64_t begBlockNumber,
													uint64_t endBlockNumber,
													int rid) {
			nlohmann::json j;
			j["address"] = address;
			j["begBlockNumber"] = begBlockNumber;
			j["endBlockNumber"] = endBlockNumber;
			j["rid"] = rid;
			ArgInfo("{} {}", GetFunName(), j.dump(4));

			if (_callback) {
				nlohmann::json r = _callback->GetTransactions(address, begBlockNumber, endBlockNumber, rid);
				ArgInfo("r => {}", r.dump(4));

				if (!r.empty()) {
					int id = rid;
					std::string hash, from, to, contract, amount, gasLimit, gasPrice, data, nonce, gasUsed, isError;
					std::string blockNumber, blockHash, blockConfirmations, blockTransactionIndex, blockTimestamp;

					try {
						id = r["id"].get<int>();
						nlohmann::json txns = r["result"];
						for (nlohmann::json::iterator it = txns.begin(); it != txns.end(); ++it) {
							nlohmann::json tx = *it;
							hash = tx["hash"].get<std::string>();
							from = tx["from"].get<std::string>();
							if (tx["to"].is_null())
								to.clear();
							else
								to = tx["to"].get<std::string>();
							contract = tx["contract"].get<std::string>();
							amount = tx["amount"].get<std::string>();
							gasLimit = tx["gasLimit"].get<std::string>();
							gasPrice = tx["gasPrice"].get<std::string>();
							data = tx["data"].get<std::string>();
							nonce = tx["nonce"].get<std::string>();
							gasUsed = tx["gasUsed"].get<std::string>();
							blockNumber = tx["blockNumber"].get<std::string>();
							blockHash = tx["blockHash"].get<std::string>();
							blockConfirmations = tx["blockConfirmations"].get<std::string>();
							blockTransactionIndex = tx["blockTransactionIndex"].get<std::string>();
							blockTimestamp = tx["blockTimestamp"].get<std::string>();
							isError = tx["isError"].get<std::string>();

							_client->_ewm->announceTransaction(id, hash, from, to, contract, amount, gasLimit, gasPrice, data, nonce, gasUsed, blockNumber, blockHash, blockConfirmations, blockTransactionIndex, blockTimestamp, isError);
						}
						_client->_ewm->announceTransactionComplete(id, true);
					} catch (const std::exception &e) {
						Log::error("invalid json format: {}", e.what());
						_client->_ewm->announceTransactionComplete(id, false);
					}
				} else {
					_client->_ewm->announceTransactionComplete(rid, false);
				}
			}
		}

		void EthSidechainSubWallet::getLogs(const std::string &contract,
											const std::string &address,
											const std::string &event,
											uint64_t begBlockNumber,
											uint64_t endBlockNumber,
											int rid) {
			nlohmann::json j;
			j["contract"] = contract;
			j["address"] = address;
			j["event"] = event;
			j["begBlockNumber"] = begBlockNumber;
			j["endBlockNumber"] = endBlockNumber;
			j["rid"] = rid;
			ArgInfo("{} {}", GetFunName(), j.dump(4));

			if (_callback) {
				nlohmann::json r = _callback->GetLogs(contract, address, event, begBlockNumber, endBlockNumber, rid);
				ArgInfo("r => {}", r.dump(4));

				if (!r.empty()) {
					int id = rid;
					std::string hash, c, data, gasPrice, gasUsed, logIndex, blockNumber, blockTransactionIndex, blockTimestamp;
					std::vector<std::string> topics;

					try {
						id = r["id"].get<int>();
						nlohmann::json logs = r["result"];
						for (nlohmann::json::iterator it = logs.begin(); it != logs.end(); ++it) {
							nlohmann::json log = *it;
							hash = log["hash"].get<std::string>();
							c = log["contract"].get<std::string>();
							topics = log["topics"].get<std::vector<std::string>>();
							data = log["data"].get<std::string>();
							gasPrice = log["gasPrice"].get<std::string>();
							gasUsed = log["gasUsed"].get<std::string>();
							logIndex = log["logIndex"].get<std::string>();
							blockNumber = log["blockNumber"].get<std::string>();
							blockTransactionIndex = log["blockTransactionIndex"].get<std::string>();
							blockTimestamp = log["blockTimestamp"].get<std::string>();

							_client->_ewm->announceLog(id, hash, c, topics, data, gasPrice, gasUsed, logIndex, blockNumber, blockTransactionIndex, blockTimestamp);
						}
						_client->_ewm->announceLogComplete(id, true);
					} catch (const std::exception &e) {
						Log::error("invalid json format: {}", e.what());
						_client->_ewm->announceLogComplete(id, false);
					}
				} else {
					_client->_ewm->announceLogComplete(rid, false);
				}
			}
		}

		void EthSidechainSubWallet::getBlocks(const std::string &address,
											  int interests,
											  uint64_t blockNumberStart,
											  uint64_t blockNumberStop,
											  int rid) {
			nlohmann::json j;
			j["address"] = address;
			j["interests"] = interests;
			j["blockNumberStart"] = blockNumberStart;
			j["blockNumberStop"] = blockNumberStop;
			j["rid"] = rid;
			ArgInfo("{} {}", GetFunName(), j.dump(4));

			if (_callback) {
				int id = rid;
				std::set<uint64_t> numberSet;
				nlohmann::json r = _callback->GetTransactions(address, blockNumberStart, blockNumberStop, rid);
				ArgInfo("getTransactions => {}", r.dump(4));

				if (!r.empty()) {
					std::string from, to, blockNumber;

					try {
						id = r["id"].get<int>();
						nlohmann::json txns = r["result"];
						for (nlohmann::json::iterator it = txns.begin(); it != txns.end(); ++it) {
							nlohmann::json tx = *it;
							from = tx["from"].get<std::string>();
							to = tx["to"].get<std::string>();
							blockNumber = tx["blockNumber"].get<std::string>();

							std::string addressLower = address, fromLower = from, toLower = to;
							std::transform(addressLower.begin(), addressLower.end(), addressLower.begin(), tolower);
							std::transform(fromLower.begin(), fromLower.end(), fromLower.begin(), tolower);
							std::transform(toLower.begin(), toLower.end(), toLower.begin(), tolower);

							bool include = (0 != (interests & (1)) && addressLower == fromLower) ||
										   (0 != (interests & (1 << 1)) && addressLower == toLower);
							if (include) {
								std::stringstream ss;
								uint64_t blockNum;
								ss << blockNumber;
								ss >> blockNum;
								numberSet.insert(blockNum);
							}
						}
					} catch (const std::exception &e) {
						Log::error("invalid json format: {}", e.what());
						return;
					}
				} else {
					Log::error("empty json");
					return;
				}

				char *pEncodedAddress = eventERC20TransferEncodeAddress(eventERC20Transfer, address.c_str());
				std::string encodedAddress = pEncodedAddress;
				free(pEncodedAddress);
				std::string selector = eventGetSelector(eventERC20Transfer);
				ArgInfo("address: {}", encodedAddress);
				ArgInfo("event: {}", selector);
				r = _callback->GetLogs("", encodedAddress, selector, blockNumberStart, blockNumberStop, rid);
				ArgInfo("getLogs => {}", r.dump(4));

				if (!r.empty()) {
					std::string blockNumber;
					std::vector<std::string> topics;

					try {
						id = r["id"].get<int>();
						nlohmann::json logs = r["result"];
						for (nlohmann::json::iterator it = logs.begin(); it != logs.end(); ++it) {
							nlohmann::json log = *it;
							topics = log["topics"].get<std::vector<std::string>>();
							blockNumber = log["blockNumber"].get<std::string>();

							if (topics.size() >= 3) {
								std::string addressLower = address, topicsLower1 = topics[1], topicsLower2 = topics[2];
								topicsLower1.erase(2, 24);
								topicsLower2.erase(2, 24);
								std::transform(addressLower.begin(), addressLower.end(), addressLower.begin(), tolower);
								std::transform(topicsLower1.begin(), topicsLower1.end(), topicsLower1.begin(), tolower);
								std::transform(topicsLower2.begin(), topicsLower2.end(), topicsLower2.begin(), tolower);
								bool include = ((0 != (interests & (1 << 2)) && addressLower == topicsLower1) ||
												(0 != (interests & (1 << 3)) && addressLower == topicsLower2));

								if (include) {
									std::stringstream ss;
									uint64_t blockNum = strtoull(blockNumber.c_str(), NULL, 0);
									numberSet.insert(blockNum);
								}
							}
						}
					} catch (const std::exception &e) {
						Log::error("invalid json format: {}", e.what());
						return;
					}
				} else {
					Log::error("empty json");
					return;
				}

				std::vector<uint64_t> numbers(numberSet.begin(), numberSet.end());
				_client->_ewm->announceBlocks(id, numbers);
			}
		}

		void EthSidechainSubWallet::getTokens(int rid) {
			nlohmann::json j;
			j["rid"] = rid;
			ArgInfo("{} {}", GetFunName(), j.dump(4));

			if (_callback) {
				nlohmann::json r = _callback->GetTokens(rid);
				ArgInfo("r => {}", r.dump(4));

				if (!r.empty()) {
					int id = rid;
					/*
						*   "address": "0x407d73d8a49eeb85d32cf465507dd71d507100c1",
						*   "symbol": "ELA",
						*   "name": "elastos",
						*   "description": "desc",
						*   "decimals": 18,
						*   "defaultGasLimit": "0x1388",
						*   "defaultGasPrice": "0x1dfd14000" // 8049999872 Wei*/
					std::string address, symbol, name, description, defaultGasLimit, defaultGasPrice;
					int decimals;

					try {
						id = r["id"].get<int>();
						nlohmann::json tokens = r["result"];
						for (nlohmann::json::iterator it = tokens.begin(); it != tokens.end(); ++it) {
							nlohmann::json token = *it;
							address = token["address"].get<std::string>();
							symbol = token["symbol"].get<std::string>();
							name = token["name"].get<std::string>();
							decimals = token["decimals"].get<int>();
							if (token.contains("description"))
								description = token["description"].get<std::string>();
							if (token.contains("defaultGasLimit"))
								defaultGasLimit = token["defaultGasLimit"].get<std::string>();
							if (token.contains("defaultGasPrice"))
								defaultGasPrice = token["defaultGasPrice"].get<std::string>();

							_client->_ewm->announceToken(address, id, symbol, name, description, decimals, defaultGasLimit, defaultGasPrice);
						}
						_client->_ewm->announceTokenComplete(id, true);
					} catch (const std::exception &e) {
						Log::error("invalid json format: {}", e.what());
						_client->_ewm->announceTokenComplete(id, false);
					}
				} else {
					_client->_ewm->announceTokenComplete(rid, false);
				}
			}
		}

		void EthSidechainSubWallet::getBlockNumber(int rid) {
			nlohmann::json j;
			j["rid"] = rid;
			ArgInfo("{} {}", GetFunName(), j.dump(4));

			if (_callback) {
				nlohmann::json r = _callback->GetBlockNumber(rid);
				ArgInfo("r => {}", r.dump(4));
				int id = rid;

				if (!r.empty()) {
					std::string blockNumber;

					try {
						id = r["id"].get<int>();
						blockNumber = r["result"].get<std::string>();
						_client->_ewm->announceBlockNumber(blockNumber, id);
					} catch (const std::exception &e) {
						Log::error("invalid json format: {}", e.what());
					}
				}
			}
		}

		void EthSidechainSubWallet::getNonce(const std::string &address, int rid) {
			nlohmann::json j;
			j["address"] = address;
			j["rid"] = rid;
			ArgInfo("{} {}", GetFunName(), j.dump(4));

			if (_callback) {
				nlohmann::json r = _callback->GetNonce(address, rid);
				ArgInfo("r => {}", r.dump(4));
				int id = rid;

				if (!r.empty()) {
					std::string nonce;

					try {
						id = r["id"].get<int>();
						nonce = r["result"].get<std::string>();
						_client->_ewm->announceNonce(address, nonce, id);
					} catch (const std::exception &e) {
						Log::error("invalid json format: {}", e.what());
					}
				}
			}
		}

		void EthSidechainSubWallet::handleEWMEvent(const BREthereumEWMEvent &event) {
			nlohmann::json eJson = EthereumEWM::EWMEvent2Json(event);
			ArgInfo("{} {}", GetFunName(), eJson.dump(4));

			if (_callback != nullptr) {
				_callback->OnETHSCEventHandled(eJson);
			} else {
				Log::info(CALLBACK_IS_NULL_PROMPT);
			}
		}

		void EthSidechainSubWallet::handlePeerEvent(const BREthereumPeerEvent &event) {
			nlohmann::json eJson = EthereumEWM::PeerEvent2Json(event);;
			ArgInfo("{} {}", GetFunName(), eJson.dump(4));

			if (_callback != nullptr) {
				_callback->OnETHSCEventHandled(eJson);
			} else {
				Log::info(CALLBACK_IS_NULL_PROMPT);
			}
		}

		void EthSidechainSubWallet::handleWalletEvent(const EthereumWalletPtr &wallet,
													  const BREthereumWalletEvent &event) {
			nlohmann::json eJson = EthereumEWM::WalletEvent2Json(event);
			eJson["WalletSymbol"] = wallet->getSymbol();
			ArgInfo("{} {}", GetFunName(), eJson.dump(4));

			if (_callback != nullptr) {
				_callback->OnETHSCEventHandled(eJson);
			} else {
				Log::info(CALLBACK_IS_NULL_PROMPT);
			}
		}

		void EthSidechainSubWallet::handleTokenEvent(const EthereumTokenPtr &token, const BREthereumTokenEvent &event) {
			nlohmann::json eJson = EthereumEWM::TokenEvent2Json(event);
			eJson["WalletSymbol"] = token->getSymbol();
			ArgInfo("{} {}", GetFunName(), eJson.dump(4));

			if (_callback != nullptr) {
				_callback->OnETHSCEventHandled(eJson);
			} else {
				Log::info(CALLBACK_IS_NULL_PROMPT);
			}
		}

		void EthSidechainSubWallet::handleTransferEvent(const EthereumWalletPtr &wallet,
														const EthereumTransferPtr &transaction,
														const BREthereumTransferEvent &event) {
			nlohmann::json eJson = EthereumEWM::TransferEvent2Json(event);
			eJson["WalletSymbol"] = wallet->getSymbol();
			eJson["TxHash"] = transaction->getIdentifier();

			BREthereumTransfer rawTransfer = transaction->getRaw();
			BREthereumTransaction rawTx = transferGetBasisTransaction(rawTransfer);

			if (rawTx != nullptr) {
				BREthereumContractFunction function = contractLookupFunctionForEncoding(contractERC20, transactionGetData(rawTx));
				if (NULL != function && functionERC20Transfer == function) {
					BRCoreParseStatus status;
					UInt256 funcAmount = functionERC20TransferDecodeAmount(function, transactionGetData(rawTx), &status);
					char *funcAddr = functionERC20TransferDecodeAddress(function, transactionGetData(rawTx));
					char *funcAmt = coerceString(funcAmount, 10);
					eJson["Token"] = transaction->getTargetAddress();
					eJson["TokenFunction"] = "ERC20Transfer";
					eJson["TokenAmount"] = funcAmt;
					eJson["TokenAddress"] = funcAddr;
					free(funcAmt);
					free(funcAddr);
				}
			} else {
				BREthereumLog rawLog = transferGetBasisLog(rawTransfer);
				if (rawLog == nullptr) {
					Log::warn("Transaction & Log is null");
				} else {
					BREthereumHash logHash = logGetHash (rawLog);
					char *logHashString = hashAsString(logHash);

					BREthereumAddress logAddress = logGetAddress(rawLog);
					char *logAddressString = addressGetEncodedString(&logAddress, 1);

					nlohmann::json topicArray = nlohmann::json::array();
					size_t topicCount = logGetTopicsCount(rawLog);
					for (size_t i = 0; i < topicCount; ++i) {
						BREthereumLogTopic topic = logGetTopic(rawLog, i);
						BREthereumLogTopicString topicString = logTopicAsString (topic);
						topicArray.push_back(topicString.chars);
					}

					eJson["LogHash"] = logHashString;
					eJson["LogAddress"] = logAddressString;
					eJson["LogTopics"] = topicArray;

					free(logHashString);
					free(logAddressString);
				}
			}

			ArgInfo("{} {}", GetFunName(), eJson.dump(4));
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

		std::string EthSidechainSubWallet::GetBalance() const {
			ArgInfo("{} {}", _walletID, GetFunName());

			std::string balance = _client->_ewm->getWallet()->getBalance();

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

        std::vector<std::string> EthSidechainSubWallet::GetLastAddresses(bool internal) const {
            return {};
		}

        void EthSidechainSubWallet::UpdateUsedAddress(const std::vector<std::string> &usedAddresses) const {

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

			_callback = subCallback;
		}

		void EthSidechainSubWallet::RemoveCallback() {
			ArgInfo("{} {}", _walletID, GetFunName());

			_callback = nullptr;
		}

		nlohmann::json EthSidechainSubWallet::CreateTransaction(const nlohmann::json &inputs,
                                                                const nlohmann::json &outputs,
                                                                const std::string &fee,
                                                                const std::string &memo) {
			ErrorChecker::ThrowParamException(Error::UnsupportOperation, "use IEthSidechainSubWallet::CreateTransfer() instead");
			return nlohmann::json();
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
			j["TxHash"] = transfer->getOriginationTransactionHash();

			ArgInfo("r => {}", j.dump());
			return j;
		}

       nlohmann::json EthSidechainSubWallet::GetAllTransaction(uint32_t start, uint32_t count, const std::string &txid) const {
               ArgInfo("{} {}", _walletID, GetFunName());
               ArgInfo("start: {}, cnt: {}, txid: {}", start, count, txid);

               nlohmann::json j, jtx;
               std::vector<nlohmann::json> txList;

               std::vector<EthereumTransferPtr> transfers = _client->_ewm->getWallet()->getTransfers();
               if (!txid.empty()) {
                       start = 0;
                       count = transfers.size();
               }

               std::reverse(transfers.begin(), transfers.end());

               for (size_t i = start; i < transfers.size() && i - start < count; ++i) {
                       std::string transferID = GetTransferID(transfers[i]);
                       if (txid.empty() || txid == transferID || txid == transfers[i]->getIdentifier()) {
                               jtx = transfers[i]->ToJson();
                               jtx ["ID"] = transferID;
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

		std::string EthSidechainSubWallet::ConvertToRawTransaction(const nlohmann::json &tx) {
			ArgInfo("{} {}", _walletID, GetFunName());
			ArgInfo("tx: {}", tx.dump());

			ArgInfo("r => ");

			return "";
		}

		void EthSidechainSubWallet::StartP2P() {
			_client->_ewm->connect();
		}

		void EthSidechainSubWallet::StopP2P() {
			_client->_ewm->disconnect();
		}

	}
}