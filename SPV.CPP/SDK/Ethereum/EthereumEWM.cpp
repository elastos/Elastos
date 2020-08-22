/*
 * EthereumEWM
 *
 * Created by Ed Gamble <ed@breadwallet.com> on 3/7/18.
 * Copyright (c) 2018 Breadwinner AG.  All right reserved.
 * Copyright (c) 2020 Elastos Foundation
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

#include "EthereumEWM.h"

#include <Common/ErrorChecker.h>
#include <ethereum/BREthereum.h>
#include <support/BRBIP39Mnemonic.h>
#include <vector>
#include <ethereum/ewm/BREthereumClient.h>

namespace Elastos {
	namespace ElaWallet {

		static void clientGetGasPrice(BREthereumClientContext context, BREthereumEWM node,
									  BREthereumWallet wid, int id) {
			if (NULL == context) return;
			EthereumEWM *c = (EthereumEWM *) context;
			c->trampolineGetGasPrice(node, wid, id);
		}

		static void clientEstimateGas(BREthereumClientContext context,
									  BREthereumEWM ewm,
									  BREthereumWallet wid,
									  BREthereumCookie cookie,
									  const char *from,
									  const char *to,
									  const char *amount,
									  const char *gasPrice,
									  const char *data,
									  int rid) {
			if (NULL == context) return;
			EthereumEWM *c = (EthereumEWM *) context;
			c->trampolineGetGasEstimate(ewm, wid, cookie,
										from ? from : "",
										to ? to : "",
										amount ? amount : "",
										gasPrice ? gasPrice : "",
										data ? data : "", rid);
		}

		static void clientGetBalance(BREthereumClientContext context, BREthereumEWM node,
									 BREthereumWallet wid, const char *account, int id) {
			if (NULL == context) return;
			EthereumEWM *c = (EthereumEWM *) context;
			c->trampolineGetBalance(node, wid, account ? account : "", id);
		}

		static void clientSubmitTransaction(BREthereumClientContext context, BREthereumEWM node,
											BREthereumWallet wid, BREthereumTransfer tid,
											const char *transaction, int id) {
			if (NULL == context) return;
			EthereumEWM *c = (EthereumEWM *) context;
			c->trampolineSubmitTransaction(node, wid, tid, transaction ? transaction : "", id);
		}

		static void clientGetTransactions(BREthereumClientContext context, BREthereumEWM node,
										  const char *address, uint64_t begBlockNumber, uint64_t endBlockNumber,
										  int id) {
			if (NULL == context) return;
			EthereumEWM *c = (EthereumEWM *) context;
			c->trampolineGetTransactions(node, address ? address : "", begBlockNumber, endBlockNumber, id);
		}

		static void clientGetLogs(BREthereumClientContext context, BREthereumEWM node,
								  const char *contract, const char *address, const char *event,
								  uint64_t begBlockNumber, uint64_t endBlockNumber, int rid) {
			if (NULL == context) return;
			EthereumEWM *c = (EthereumEWM *) context;
			c->trampolineGetLogs(node, contract ? contract : "",
								 address ? address : "",
								 event ? event : "",
								 begBlockNumber, endBlockNumber, rid);
		}

		static void clientGetBlocks(BREthereumClientContext context, BREthereumEWM ewm,
									const char *address, BREthereumSyncInterestSet interests,
									uint64_t blockNumberStart, uint64_t blockNumberStop, int rid) {
			if (NULL == context) return;
			EthereumEWM *c = (EthereumEWM *) context;
			c->trampolineGetBlocks(ewm, address ? address : "",
								   interests, blockNumberStart, blockNumberStop, rid);
		}

		static void clientGetTokens(BREthereumClientContext context, BREthereumEWM ewm, int rid) {
			if (NULL == context) return;
			EthereumEWM *c = (EthereumEWM *) context;
			c->trampolineGetTokens(ewm, rid);
		}


		static void clientGetBlockNumber(BREthereumClientContext context, BREthereumEWM node, int id) {
			if (NULL == context) return;
			EthereumEWM *c = (EthereumEWM *) context;
			c->trampolineGetBlockNumber(node, id);
		}

		static void clientGetNonce(BREthereumClientContext context, BREthereumEWM node, const char *address, int id) {
			if (NULL == context) return;
			EthereumEWM *c = (EthereumEWM *) context;
			c->trampolineGetNonce(node, address ? address : "", id);
		}


		static void
		clientEWMEventHandler(BREthereumClientContext context, BREthereumEWM ewm, BREthereumEWMEvent event) {
			if (NULL == context) return;
			EthereumEWM *c = (EthereumEWM *) context;
			c->trampolineEWMEvent(ewm, event);
		}

		static void clientPeerEventHandler(BREthereumClientContext context,
										   BREthereumEWM ewm,
										   BREthereumPeerEvent event) {
			if (NULL == context) return;
			EthereumEWM *c = (EthereumEWM *) context;
			c->trampolinePeerEvent(ewm, event);
		}

		static void clientWalletEventHandler(BREthereumClientContext context,
											 BREthereumEWM ewm,
											 BREthereumWallet wid,
											 BREthereumWalletEvent event) {
			if (NULL == context) return;
			EthereumEWM *c = (EthereumEWM *) context;
			c->trampolineWalletEvent(ewm, wid, event);
		}

		static void clientTokenEventHandler(BREthereumClientContext context, BREthereumEWM ewm,
											BREthereumToken token, BREthereumTokenEvent event) {
			if (NULL == context) return;
			EthereumEWM *c = (EthereumEWM *) context;
			c->trampolineTokenEvent(ewm, token, event);
		}

		static void clientTransferEventHandler(BREthereumClientContext context,
											   BREthereumEWM ewm,
											   BREthereumWallet wid,
											   BREthereumTransfer tid,
											   BREthereumTransferEvent event) {
			if (NULL == context) return;
			EthereumEWM *c = (EthereumEWM *) context;
			c->trampolineTransferEvent(ewm, wid, tid, event);
		}

		std::string EthereumEWM::Status2String(const BREthereumStatus &s) {
			std::string status;
			switch (s) {
				case SUCCESS:
					status = "SUCCESS";
					break;
				case ERROR_FAILED:
					status = "ERROR_FAILED";
					break;
					// Reference access,
				case ERROR_UNKNOWN_NODE:
					status = "ERROR_UNKNOWN_NODE";
					break;
				case ERROR_UNKNOWN_TRANSACTION:
					status = "ERROR_UNKNOWN_TRANSACTION";
					break;
				case ERROR_UNKNOWN_ACCOUNT:
					status = "ERROR_UNKNOWN_ACCOUNT";
					break;
				case ERROR_UNKNOWN_WALLET:
					status = "ERROR_UNKNOWN_WALLET";
					break;
				case ERROR_UNKNOWN_BLOCK:
					status = "ERROR_UNKNOWN_BLOCK";
					break;
				case ERROR_UNKNOWN_LISTENER:
					status = "ERROR_UNKNOWN_LISTENER";
					break;

					// Node
				case ERROR_NODE_NOT_CONNECTED:
					status = "ERROR_NODE_NOT_CONNECTED";
					break;

					// Transaction
				case ERROR_TRANSACTION_HASH_MISMATCH:
					status = "ERROR_TRANSACTION_HASH_MISMATCH";
					break;
				case ERROR_TRANSACTION_SUBMISSION:
					status = "ERROR_TRANSACTION_SUBMISSION";
					break;

					// Acount
					// Wallet
					// Block
					// Listener

					// Numeric
				case ERROR_NUMERIC_PARSE:
					status = "ERROR_NUMERIC_PARSE";
					break;
				default:
					status = "UNKNOWN";
					break;
			}

			return status;
		}

		nlohmann::json EthereumEWM::WalletEvent2Json(const BREthereumWalletEvent &e) {
			std::string event;
			nlohmann::json j;

			switch (e.type) {
				case WALLET_EVENT_CREATED:
					event = "CREATED";
					break;
				case WALLET_EVENT_BALANCE_UPDATED:
					event = "BALANCE_UPDATED";
					break;
				case WALLET_EVENT_DEFAULT_GAS_LIMIT_UPDATED:
					event = "DEFAULT_GAS_LIMIT_UPDATED";
					break;
				case WALLET_EVENT_DEFAULT_GAS_PRICE_UPDATED:
					event = "DEFAULT_GAS_PRICE_UPDATED";
					break;
				case WALLET_EVENT_FEE_ESTIMATED:
					event = "FEE_ESTIMATED";
//					j["Cookie"] = event.u.feeEstimate.cookie;
					j["GasEstimate"] = e.u.feeEstimate.gasEstimate.amountOfGas;
					j["GasPrice"] = e.u.feeEstimate.gasPrice.etherPerGas.valueInWEI.u64[0];
					break;
				case WALLET_EVENT_DELETED:
					event = "DELETED";
					break;
				default:
					event = "UNDEFINE";
					break;
			}

			j["Type"] = "WalletEvent";
			j["Event"] = event;
			j["Status"] = EthereumEWM::Status2String(e.status);
			j["ErrorDescription"] = e.errorDescription;

			return j;
		}

		nlohmann::json EthereumEWM::TokenEvent2Json(const BREthereumTokenEvent &e) {
			std::string event;
			nlohmann::json j;

			switch (e.type) {
				case TOKEN_EVENT_CREATED:
					event = "CREATED";
					break;
				case TOKEN_EVENT_DELETED:
					event = "DELETED";
					break;
				default:
					event = "UNDEFINE";
					break;
			}

			j["Type"] = "TokenEvent";
			j["Event"] = event;
			j["Status"] = EthereumEWM::Status2String(e.status);
			j["ErrorDescription"] = e.errorDescription;

			return j;
		}

		nlohmann::json EthereumEWM::TransferEvent2Json(const BREthereumTransferEvent &e) {
			std::string event;
			nlohmann::json j;

			switch (e.type) {
				case TRANSFER_EVENT_CREATED:
					event = "CREATED";
					break;
				case TRANSFER_EVENT_SIGNED:
					event = "SIGNED";
					break;
				case TRANSFER_EVENT_SUBMITTED:
					event = "SUBMITTED";
					break;
				case TRANSFER_EVENT_INCLUDED:
					event = "INCLUDED";
					break;  // aka confirmed
				case TRANSFER_EVENT_ERRORED:
					event = "ERRORED";
					break;
				case TRANSFER_EVENT_GAS_ESTIMATE_UPDATED:
					event = "GAS_ESTIMATE_UPDATED";
					break;
				case TRANSFER_EVENT_DELETED:
					event = "DELETED";
					break;
				default:
					event = "UNDEFINE";
					break;
			}

			j["Type"] = "TransferEvent";
			j["Event"] = event;
			j["Status"] = EthereumEWM::Status2String(e.status);
			j["ErrorDescription"] = e.errorDescription;

			return j;
		}

		nlohmann::json EthereumEWM::EWMEvent2Json(const BREthereumEWMEvent &e) {
			std::string event;
			nlohmann::json j;

			switch (e.type) {
				case EWM_EVENT_CREATED:
					event = "CREATED";
					break;
				case EWM_EVENT_CHANGED:
					event = "CHANGED";
					j["OldState"] = EthereumEWM::EWMState2String(e.u.changed.oldState);
					j["NewState"] = EthereumEWM::EWMState2String(e.u.changed.newState);
					break;
				case EWM_EVENT_SYNC_PROGRESS:
					event = "PROGRESS";
					j["Timestamp"] = e.u.syncProgress.timestamp;
					j["PercentComplete"] = e.u.syncProgress.percentComplete;
					break;
				case EWM_EVENT_BLOCK_HEIGHT_UPDATED:
					event = "HEIGHT_UPDATED";
					j["BlockHeight"] = e.u.blockHeight.value;
					break;
				case EWM_EVENT_NETWORK_UNAVAILABLE:
					event = "NETWORK_UNAVAILABLE";
					break;
				case EWM_EVENT_DELETED:
					event = "DELETED";
					break;
				default:
					event = "UNDEFINE";
					break;
			}

			j["Type"] = "EWMEvent";
			j["Event"] = event;
			j["Status"] = Status2String(e.status);
			j["ErrorDescription"] = e.errorDescription;
			return j;
		}

		nlohmann::json EthereumEWM::PeerEvent2Json(const BREthereumPeerEvent &e) {
			std::string event;
			switch (e.type) {
				case PEER_EVENT_CREATED:
					event = "CREATED";
					break;
				case PEER_EVENT_DELETED:
					event = "DELETED";
					break;
				default:
					event = "UNDEFINE";
					break;
			}
			nlohmann::json j;
			j["Type"] = "PeerEvent";
			j["Event"] = event;
			j["Status"] = EthereumEWM::Status2String(e.status);
			j["ErrorDescription"] = e.errorDescription;
			return j;
		}

		std::string EthereumEWM::EWMState2String(const BREthereumEWMState &s) {
			std::string state;
			switch (s) {
				case EWM_STATE_CREATED:
					state = "CREATED";
					break;
				case EWM_STATE_CONNECTED:
					state = "CONNECTED";
					break;
				case EWM_STATE_SYNCING:
					state = "SYNCING";
					break;
				case EWM_STATE_DISCONNECTED:
					state = "DISCONNECTED";
					break;
				case EWM_STATE_DELETED:
					state = "DELETED";
					break;
				default:
					state = "UNDEFINE";
					break;
			}
			return state;
		}

		// Client Announcers
		void EthereumEWM::announceBalance(BREthereumWallet wid, const std::string &balance, int rid) {
			ewmAnnounceWalletBalance(_ewm, wid, balance.c_str(), rid);
		}

		void EthereumEWM::announceGasPrice(BREthereumWallet wid, const std::string &gasPrice, int rid) {
			ewmAnnounceGasPrice(_ewm, wid, gasPrice.c_str(), rid);
		}

		void
		EthereumEWM::announceGasEstimate(BREthereumWallet wid, BREthereumTransfer tid, const std::string &gasEstimate,
										 int rid) {
//			ewmAnnounceGasEstimate(_ewm, wid, tid, gasEstimate.c_str(), rid);
		}

		void
		EthereumEWM::announceSubmitTransaction(BREthereumWallet wid, BREthereumTransfer tid, const std::string &hash,
											   int errorCode,
											   const std::string &errorMessage, int rid) {
			ewmAnnounceSubmitTransfer(_ewm, wid, tid, hash.c_str(), errorCode, errorMessage.c_str(), rid);
		}

		void EthereumEWM::announceTransaction(int id,
											  const std::string &hash,
											  const std::string &from,
											  const std::string &to,
											  const std::string &contract,
											  const std::string &amount, // value
											  const std::string &gasLimit,
											  const std::string &gasPrice,
											  const std::string &data,
											  const std::string &nonce,
											  const std::string &gasUsed,
											  const std::string &blockNumber,
											  const std::string &blockHash,
											  const std::string &blockConfirmations,
											  const std::string &blockTransactionIndex,
											  const std::string &blockTimestamp,
			// cumulative gas used,
			// confirmations
			// txreceipt_status
											  const std::string &isError) {
			ewmAnnounceTransaction(_ewm, id, hash.data(), from.data(), to.data(), contract.data(), amount.data(),
								   gasLimit.data(), gasPrice.data(), data.data(), nonce.data(), gasUsed.data(),
								   blockNumber.data(), blockHash.data(), blockConfirmations.data(),
								   blockTransactionIndex.data(), blockTimestamp.data(), isError.data());
		}

		void EthereumEWM::announceTransactionComplete(int id, bool success) {
			ewmAnnounceTransactionComplete(_ewm, id, AS_ETHEREUM_BOOLEAN(success));
		}

		void EthereumEWM::announceLog(int id,
									  const std::string &hash,
									  const std::string &contract,
									  const std::vector<std::string> &topics,
									  const std::string &data,
									  const std::string &gasPrice,
									  const std::string &gasUsed,
									  const std::string &logIndex,
									  const std::string &blockNumber,
									  const std::string &blockTransactionIndex,
									  const std::string &blockTimestamp) {
			size_t topicsCount = topics.size();
			const char *ctopics[topicsCount];
			for (size_t i = 0; i < topicsCount; ++i)
				ctopics[i] = topics[i].data();

			ewmAnnounceLog(_ewm, id, hash.data(), contract.data(), topics.size(), ctopics, data.data(),
						   gasPrice.data(), gasUsed.data(), logIndex.data(), blockNumber.data(),
						   blockTransactionIndex.data(), blockTimestamp.data());
		}

		void EthereumEWM::announceLogComplete(int id, bool success) {
			ewmAnnounceLogComplete(_ewm, id, AS_ETHEREUM_BOOLEAN(success));
		}

		void EthereumEWM::announceBlockNumber(const std::string &blockNumber, int rid) {
			ewmAnnounceBlockNumber(_ewm, blockNumber.data(), rid);
		}

		void EthereumEWM::announceNonce(const std::string &address, const std::string &nonce, int rid) {
			ewmAnnounceNonce(_ewm, address.data(), nonce.data(), rid);
		}

		void EthereumEWM::announceToken(const std::string &address,
										int rid,
										const std::string &symbol,
										const std::string &name,
										const std::string &description,
										int decimals,
										const std::string &defaultGasLimit,
										const std::string &defaultGasPrice) {
			ewmAnnounceToken(_ewm, rid, address.data(), symbol.data(), name.data(), description.data(), decimals,
							 defaultGasLimit.data(), defaultGasPrice.data());
		}

		void EthereumEWM::announceTokenComplete(int rid, bool success) {
			ewmAnnounceTokenComplete(_ewm, rid, AS_ETHEREUM_BOOLEAN(success));
		}

		EthereumNetworkPtr EthereumEWM::getNetwork() const {
			return _network;
		}

		EthereumAccountPtr EthereumEWM::getAccount() const {
			return _account;
		}

		std::string EthereumEWM::getAddress() const {
			return _account->getPrimaryAddress();
		}

		bytes_t EthereumEWM::getAddressPublicKey() const {
			return _account->getPrimaryAddressPublicKey();
		}

		EthereumWalletPtr EthereumEWM::walletLookupOrCreate(BREthereumWallet wid, const EthereumTokenPtr &token) {
			WalletMap::iterator it = _wallets.find(wid);
			if (it != _wallets.end()) {
				return it->second;
			}

			EthereumTokenPtr t;
			if (nullptr == token) {
				BREthereumToken tokenRef = ewmWalletGetToken(_ewm, (BREthereumWallet) wid);
				if (NULL != tokenRef) {
					t = lookupTokenByReference(tokenRef);
				}
			}

			EthereumWalletPtr wallet;
			if (nullptr != t) {
				wallet = EthereumWalletPtr(new EthereumWallet(this, (BREthereumWallet) wid, _account, _network, t));
			} else {
				wallet = EthereumWalletPtr(new EthereumWallet(this, (BREthereumWallet) wid, _account, _network));
			}

			_wallets[wid] = wallet;

			return wallet;
		}

		EthereumWalletPtr EthereumEWM::getWallet() {
			BREthereumWallet wid = ewmGetWallet(_ewm);
			return walletLookupOrCreate(wid, nullptr);
		}

		EthereumWalletPtr EthereumEWM::getWallet(const EthereumTokenPtr &token) {
			BREthereumWallet wid = ewmGetWalletHoldingToken(_ewm, token->getRaw());
			return walletLookupOrCreate(wid, token);
		}

		EthereumWalletPtr EthereumEWM::getWalletByIdentifier(BREthereumWallet wid) {
			return walletLookupOrCreate(wid, nullptr);
		}

		EthereumTransferPtr EthereumEWM::transactionLookupOrCreate(BREthereumTransfer tid) {
			TransferMap::iterator it = _transactions.find(tid);

			if (it != _transactions.end()) {
				return it->second;
			}

			BREthereumToken token = ewmTransferGetToken(_ewm, tid);
			EthereumTransferPtr transfer(new EthereumTransfer(this, tid,
															  (NULL == token ? EthereumAmount::Unit::ETHER_ETHER
																			 : EthereumAmount::Unit::TOKEN_DECIMAL)));
			_transactions[tid] = transfer;
			return transfer;
		}

		EthereumBlockPtr EthereumEWM::blockLookupOrCreate(BREthereumBlock bid) {
			BlockMap::iterator it = _blocks.find(bid);
			if (it != _blocks.end())
				return it->second;

			EthereumBlockPtr block(new EthereumBlock(this, bid));
			_blocks[bid] = block;

			return block;
		}

		uint64_t EthereumEWM::getBlockHeight() const {
			return ewmGetBlockHeight(_ewm);
		}

		std::vector<EthereumTokenPtr> EthereumEWM::getTokens() const {
			std::vector<EthereumTokenPtr> tokens;

			for (TokenAddressMap::const_iterator it = _tokensByAddress.cbegin(); it != _tokensByAddress.cend(); ++it) {
				tokens.push_back(it->second);
			}

			return tokens;
		}

		EthereumTokenPtr EthereumEWM::lookupTokenByReference(BREthereumToken reference) const {
			if (_tokensByReference.find(reference) == _tokensByReference.end())
				return nullptr;

			return _tokensByReference.at(reference);
		}

		EthereumTokenPtr EthereumEWM::addTokenByReference(BREthereumToken reference) {
			EthereumTokenPtr token(new EthereumToken(reference));
			_tokensByReference[reference] = token;
			std::string address = token->getAddressLowerCase();
			_tokensByAddress[address] = token;
			return token;
		}

		EthereumTokenPtr EthereumEWM::lookupToken(const std::string &address) const {
			std::string addr = address;
			std::transform(addr.begin(), addr.end(), addr.begin(),
						   [](unsigned char c) { return std::tolower(c); });
			if (_tokensByAddress.find(addr) == _tokensByAddress.end())
				return nullptr;

			return _tokensByAddress.at(addr);
		}

		void EthereumEWM::updateTokens() {
			ewmUpdateTokens(_ewm);
		}

		EthereumEWM::EthereumEWM(Client *client, BRCryptoSyncMode mode, const EthereumNetworkPtr &network,
								 const std::string &storagePath, const std::string &paperKey,
								 const std::vector<std::string> &wordList,
								 uint64_t blockHeight,
								 uint64_t confirmationsUntilFinal) :
			EthereumEWM(createRawEWM(mode, network->getRaw(), storagePath, paperKey, wordList, blockHeight,
									 confirmationsUntilFinal), client, network) {
		}

		EthereumEWM::EthereumEWM(Client *client, BRCryptoSyncMode mode, const EthereumNetworkPtr &network,
								 const std::string &storagePath, const bytes_t &publicKey,
								 uint64_t blockHeight,
								 uint64_t confirmationsUntilFinal) :
			EthereumEWM(createRawEWMPublicKey(mode, network->getRaw(), storagePath, publicKey, blockHeight,
											  confirmationsUntilFinal), client, network) {
		}

		EthereumEWM::EthereumEWM(BREthereumEWM ewm, EthereumEWM::Client *client, const EthereumNetworkPtr &network) :
			_ewm(ewm),
			_client(client),
			_network(network),
			_executor(1),
			_account(EthereumAccountPtr(new EthereumAccount(this, ewmGetAccount(ewm)))) {
		}

		BREthereumEWM
		EthereumEWM::createRawEWM(BRCryptoSyncMode mode, BREthereumNetwork network, const std::string &storagePath,
								  BREthereumAccount account,
								  uint64_t blockHeight,
								  uint64_t confirmationsUntilFinal) {
			BREthereumClient brClient = {
				this,
				clientGetBalance,
				clientGetGasPrice,
				clientEstimateGas,
				clientSubmitTransaction,
				clientGetTransactions,
				clientGetLogs,
				clientGetBlocks,
				clientGetTokens,
				clientGetBlockNumber,
				clientGetNonce,

				clientEWMEventHandler,
				clientPeerEventHandler,
				clientWalletEventHandler,
				clientTokenEventHandler,
				clientTransferEventHandler
			};

			return ewmCreate(network, account, ETHEREUM_TIMESTAMP_UNKNOWN, mode, brClient, storagePath.data(),
							 blockHeight, confirmationsUntilFinal);
		}

		BREthereumEWM EthereumEWM::createRawEWM(BRCryptoSyncMode mode, BREthereumNetwork network,
												const std::string &storagePath, const std::string &paperKey,
												const std::vector<std::string> &wordList,
												uint64_t blockHeight,
												uint64_t confirmationsUntilFinal) {
			int wordsCount = wordList.size();
			assert (BIP39_WORDLIST_COUNT == wordsCount);
			static const char *wordListPtr[BIP39_WORDLIST_COUNT];

			for (int i = 0; i < wordsCount; i++)
				wordListPtr[i] = wordList[i].c_str();

			installSharedWordList((const char **) wordListPtr, BIP39_WORDLIST_COUNT);
			BREthereumClient brClient = {
				this,
				clientGetBalance,
				clientGetGasPrice,
				clientEstimateGas,
				clientSubmitTransaction,
				clientGetTransactions,
				clientGetLogs,
				clientGetBlocks,
				clientGetTokens,
				clientGetBlockNumber,
				clientGetNonce,

				clientEWMEventHandler,
				clientPeerEventHandler,
				clientWalletEventHandler,
				clientTokenEventHandler,
				clientTransferEventHandler
			};

			return ewmCreateWithPaperKey((BREthereumNetwork) network, paperKey.data(), ETHEREUM_TIMESTAMP_UNKNOWN,
										 mode, brClient, storagePath.data(), blockHeight, confirmationsUntilFinal);
		}

		BREthereumEWM EthereumEWM::createRawEWMPublicKey(BRCryptoSyncMode mode, BREthereumNetwork network,
														 const std::string &storagePath, const bytes_t &pubkey,
														 uint64_t blockHeight,
														 uint64_t confirmationsUntilFinal) {
			assert (65 == pubkey.size());

			BRKey key = {
				UINT256_ZERO,
				{0},
				0
			};

			memcpy(key.pubKey, pubkey.data(), 65);

			BREthereumClient brClient = {
				this,
				clientGetBalance,
				clientGetGasPrice,
				clientEstimateGas,
				clientSubmitTransaction,
				clientGetTransactions,
				clientGetLogs,
				clientGetBlocks,
				clientGetTokens,
				clientGetBlockNumber,
				clientGetNonce,

				clientEWMEventHandler,
				clientPeerEventHandler,
				clientWalletEventHandler,
				clientTokenEventHandler,
//				clientBlockEventHandler,
				clientTransferEventHandler
			};

			// TODO: set to correct time to replace ETHEREUM_TIMESTAMP_UNKNOWN
			return ewmCreateWithPublicKey((BREthereumNetwork) network, key, time(NULL),
										  mode, brClient, storagePath.data(), blockHeight, confirmationsUntilFinal);
		}

		bool EthereumEWM::connect() {
			ewmStart(_ewm);
			return ETHEREUM_BOOLEAN_IS_TRUE(ewmConnect(_ewm));
		}

		bool EthereumEWM::disconnect() {
			return ETHEREUM_BOOLEAN_IS_TRUE(ewmDisconnect(_ewm));
		}

		bool EthereumEWM::addressIsValid(const std::string &address) {
			return ETHEREUM_BOOLEAN_IS_TRUE(addressValidateString(address.data()));
		}

		void EthereumEWM::ensureValidAddress(const std::string &address) {
			ErrorChecker::CheckCondition(!addressIsValid(address), Error::Code::InvalidEthereumAddress,
										 "Invalid Ethereum Address");
		}

		void EthereumEWM::trampolineGetGasPrice(BREthereumEWM eid, BREthereumWallet wid, int rid) {
			_executor.Execute(Runnable([this, wid, rid]() -> void {
				_client->getGasPrice(wid, rid);
			}));
		}

		void EthereumEWM::trampolineGetGasEstimate(BREthereumEWM ewm, BREthereumWallet wid, BREthereumCookie cookie,
												   const std::string &from, const std::string &to,
												   const std::string &amount, const std::string &gasPrice,
												   const std::string &data, int rid) {
			_executor.Execute(Runnable([this, wid, cookie, from, to, amount, gasPrice, data, rid]() -> void {
				_client->getGasEstimate(wid, cookie, from, to, amount, gasPrice, data, rid);
			}));
		}

		void EthereumEWM::trampolineGetBalance(BREthereumEWM eid, BREthereumWallet wid, const std::string &address,
											   int rid) {
			_executor.Execute(Runnable([this, wid, address, rid]() -> void {
				_client->getBalance(wid, address, rid);
			}));
		}

		void EthereumEWM::trampolineSubmitTransaction(BREthereumEWM eid, BREthereumWallet wid, BREthereumTransfer tid,
													  const std::string &rawTransaction, int rid) {
			_executor.Execute(Runnable([this, wid, tid, rawTransaction, rid]() -> void {
				_client->submitTransaction(wid, tid, rawTransaction, rid);
			}));
		}

		void
		EthereumEWM::trampolineGetTransactions(BREthereumEWM eid, const std::string &address, uint64_t begBlockNumber,
											   uint64_t endBlockNumber, int rid) {
			_executor.Execute(Runnable([this, address, begBlockNumber, endBlockNumber, rid]() -> void {
				_client->getTransactions(address, begBlockNumber, endBlockNumber, rid);
			}));
		}

		void EthereumEWM::trampolineGetLogs(BREthereumEWM eid, const std::string &contract, const std::string &address,
											const std::string &event, uint64_t begBlockNumber, uint64_t endBlockNumber,
											int rid) {
			_executor.Execute(Runnable([this, contract, address, event, begBlockNumber, endBlockNumber, rid]() -> void {
				_client->getLogs(contract, address, event, begBlockNumber, endBlockNumber, rid);
			}));
		}

		void EthereumEWM::trampolineGetBlocks(BREthereumEWM eid, const std::string &address, int interests,
											  uint64_t blockNumberStart, uint64_t blockNumberStop, int rid) {
			_executor.Execute(Runnable([this, address, interests, blockNumberStart, blockNumberStop, rid]() -> void {
				_client->getBlocks(address, interests, blockNumberStart, blockNumberStop, rid);
			}));
		}

		void EthereumEWM::trampolineGetTokens(BREthereumEWM eid, int rid) {
			_executor.Execute(Runnable([this, rid]() -> void {
				_client->getTokens(rid);
			}));
		}

		void EthereumEWM::trampolineGetBlockNumber(BREthereumEWM eid, int rid) {
			_executor.Execute(Runnable([this, rid]() -> void {
				_client->getBlockNumber(rid);
			}));
		}

		void EthereumEWM::trampolineGetNonce(BREthereumEWM ewm, const std::string &address, int rid) {
			_executor.Execute(Runnable([this, address, rid]() -> void {
				_client->getNonce(address, rid);
			}));
		}

		void EthereumEWM::trampolineEWMEvent(BREthereumEWM ewm, const BREthereumEWMEvent &event) {
			_executor.Execute(Runnable([this, event]() -> void {
				_client->handleEWMEvent(event);
			}));
		}

		void EthereumEWM::trampolinePeerEvent(BREthereumEWM ewm, const BREthereumPeerEvent &event) {
			_executor.Execute(Runnable([this, event]() -> void {
				_client->handlePeerEvent(event);
			}));
		}

		void EthereumEWM::trampolineWalletEvent(BREthereumEWM eid, BREthereumWallet wid,
												const BREthereumWalletEvent &event) {
			EthereumWalletPtr wallet = walletLookupOrCreate(wid, nullptr);
			_executor.Execute(Runnable([this, wallet, event]() -> void {
				_client->handleWalletEvent(wallet, event);
			}));
		}

		void EthereumEWM::trampolineTokenEvent(BREthereumEWM eid, BREthereumToken tokenId,
											   const BREthereumTokenEvent &event) {
			EthereumTokenPtr token = addTokenByReference(tokenId);
			_executor.Execute(Runnable([this, token, event]() -> void {
				_client->handleTokenEvent(token, event);
			}));
		}

		void EthereumEWM::trampolineTransferEvent(BREthereumEWM eid, BREthereumWallet wid, BREthereumTransfer tid,
												  const BREthereumTransferEvent &event) {
			EthereumWalletPtr wallet = walletLookupOrCreate(wid, nullptr);
			EthereumTransferPtr transaction = transactionLookupOrCreate(tid);
			_executor.Execute(Runnable([this, wallet, transaction, event]() -> void {
				_client->handleTransferEvent(wallet, transaction, event);
			}));
		}

		BREthereumEWM EthereumEWM::getRaw() const {
			return _ewm;
		}

	}
}