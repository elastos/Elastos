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

#ifndef __ELASTOS_SDK_ETHEREUMEWM_H__
#define __ELASTOS_SDK_ETHEREUMEWM_H__

#include "EthereumNetwork.h"
#include "EthereumAccount.h"
#include "EthereumTransfer.h"
#include "EthereumAmount.h"
#include "EthereumToken.h"
#include "EthereumWallet.h"
#include "EthereumBlock.h"

#include <ethereum/ewm/BREthereumEWM.h>
#include <ethereum/blockchain/BREthereumBlock.h>
#include <SpvService/BackgroundExecutor.h>

#include <string>
#include <boost/weak_ptr.hpp>
#include <nlohmann/json.hpp>

namespace Elastos {
	namespace ElaWallet {

		class EthereumEWM {
		public:
			static std::string Status2String(const BREthereumStatus &s);

			static nlohmann::json WalletEvent2Json(const BREthereumWalletEvent &e);

			static nlohmann::json TokenEvent2Json(const BREthereumTokenEvent &e);

			static nlohmann::json TransferEvent2Json(const BREthereumTransferEvent &e);

			static nlohmann::json EWMEvent2Json(const BREthereumEWMEvent &e);

			static nlohmann::json PeerEvent2Json(const BREthereumPeerEvent &e);

			static std::string EWMState2String(const BREthereumEWMState &s);

		public:
			// Client
			class Client {
			public:
				virtual void getGasPrice(BREthereumWallet wid, int rid) = 0;

				virtual void getGasEstimate(BREthereumWallet wid,
											BREthereumCookie cookie,
											const std::string &from,
											const std::string &to,
											const std::string &amount,
											const std::string &gasPrice,
											const std::string &data,
											int rid) = 0;

				virtual void getBalance(BREthereumWallet wid, const std::string &address, int rid) = 0;

				virtual void
				submitTransaction(BREthereumWallet wid, BREthereumTransfer tid, const std::string &rawTransaction,
								  int rid) = 0;

				virtual void
				getTransactions(const std::string &address, uint64_t begBlockNumber, uint64_t endBlockNumber,
								int rid) = 0;

				virtual void getLogs(const std::string &contract,
									 const std::string &address,
									 const std::string &event,
									 uint64_t begBlockNumber,
									 uint64_t endBlockNumber,
									 int rid) = 0;


				virtual void getBlocks(const std::string &address,
									   int interests,
									   uint64_t blockNumberStart,
									   uint64_t blockNumberStop,
									   int rid) = 0;

				virtual void getTokens(int rid) = 0;

				virtual void getBlockNumber(int rid) = 0;

				virtual void getNonce(const std::string &address, int rid) = 0;

				virtual void handleEWMEvent(const BREthereumEWMEvent &event) = 0;

				virtual void handlePeerEvent(const BREthereumPeerEvent &event) = 0;

				virtual void handleWalletEvent(const EthereumWalletPtr &wallet,
											   const BREthereumWalletEvent &event) = 0;

				virtual void handleTokenEvent(const EthereumTokenPtr &token, const BREthereumTokenEvent &event) = 0;

				virtual void handleTransferEvent(const EthereumWalletPtr &wallet,
												 const EthereumTransferPtr &transaction,
												 const BREthereumTransferEvent &event) = 0;
			};

		public:
			// Client Announcers
			void announceBalance(BREthereumWallet wid, const std::string &balance, int rid);

			void announceGasPrice(BREthereumWallet wid, const std::string &gasPrice, int rid);

			void announceGasEstimate(BREthereumWallet wid,
									 BREthereumTransfer tid,
									 const std::string &gasEstimate,
									 int rid);

			void announceSubmitTransaction(BREthereumWallet wid,
										   BREthereumTransfer tid,
										   const std::string &hash,
										   int errorCode,
										   const std::string &errorMessage,
										   int rid);

			void announceTransaction(int id,
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
									 const std::string &isError);

			void announceTransactionComplete(int id, bool success);

			void announceLog(int id,
							 const std::string &hash,
							 const std::string &contract,
							 const std::vector<std::string> &topics,
							 const std::string &data,
							 const std::string &gasPrice,
							 const std::string &gasUsed,
							 const std::string &logIndex,
							 const std::string &blockNumber,
							 const std::string &blockTransactionIndex,
							 const std::string &blockTimestamp);

			void announceLogComplete(int id, bool success);

			void announceBlockNumber(const std::string &blockNumber, int rid);

			void announceNonce(const std::string &address, const std::string &nonce, int rid);

			void announceToken(const std::string &address,
							   int rid,
							   const std::string &symbol,
							   const std::string &name,
							   const std::string &description,
							   int decimals,
							   const std::string &defaultGasLimit,
							   const std::string &defaultGasPrice);

			void announceTokenComplete(int rid, bool success);

			// Account
			EthereumNetworkPtr getNetwork() const;

			EthereumAccountPtr getAccount() const;

			std::string getAddress() const;

			bytes_t getAddressPublicKey() const;

			// Wallet
		protected:
			EthereumWalletPtr walletLookupOrCreate(BREthereumWallet wid, const EthereumTokenPtr &token);

		public:
			EthereumWalletPtr getWallet();

			EthereumWalletPtr getWallet(const EthereumTokenPtr &token);

			EthereumWalletPtr getWalletByIdentifier(BREthereumWallet wid);

			// Transaction
		protected:
			EthereumTransferPtr transactionLookupOrCreate(BREthereumTransfer tid);

			// Block
		protected:
			EthereumBlockPtr blockLookupOrCreate(BREthereumBlock bid);

		public:
			uint64_t getBlockHeight() const;

			// Tokens
		public:
			std::vector<EthereumTokenPtr> getTokens() const;

		protected:
			EthereumTokenPtr lookupTokenByReference(BREthereumToken reference) const;

			EthereumTokenPtr addTokenByReference(BREthereumToken reference);

		public:
			EthereumTokenPtr lookupToken(const std::string &address) const;

			void updateTokens();

			// Constructor
		public:
			EthereumEWM(Client *client, BRCryptoSyncMode mode, const EthereumNetworkPtr &network,
						const std::string &storagePath, const std::string &paperKey,
						const std::vector<std::string> &wordList,
						uint64_t blockHeight,
						uint64_t confirmationsUntilFinal);

			EthereumEWM(Client *client, BRCryptoSyncMode mode, const EthereumNetworkPtr &network,
						const std::string &storagePath, const bytes_t &publicKey,
						uint64_t blockHeight,
						uint64_t confirmationsUntilFinal);

		private:
			EthereumEWM(BREthereumEWM identifier, Client *client, const EthereumNetworkPtr &network);

			BREthereumEWM createRawEWM(BRCryptoSyncMode mode, BREthereumNetwork network,
									   const std::string &storagePath, BREthereumAccount account,
									   uint64_t blockHeight,
									   uint64_t confirmationsUntilFinal);

			BREthereumEWM createRawEWM(BRCryptoSyncMode mode, BREthereumNetwork network,
									   const std::string &storagePath, const std::string &paperKey,
									   const std::vector<std::string> &wordList,
									   uint64_t blockHeight,
									   uint64_t confirmationsUntilFinal);

			BREthereumEWM createRawEWMPublicKey(BRCryptoSyncMode mode, BREthereumNetwork network,
												const std::string &storagePath, const bytes_t &pubkey,
												uint64_t blockHeight,
												uint64_t confirmationsUntilFinal);

			// Connect / Disconnect
		public:
			bool connect();

			bool disconnect();

			static bool addressIsValid(const std::string &address);

			static void ensureValidAddress(const std::string &address);

		public:
			//
			// Callback Announcements
			//
			// These methods also give us a chance to convert the `event`, as a `long`, to the Event.
			//
			void trampolineGetGasPrice(BREthereumEWM eid, BREthereumWallet wid, int rid);

			void trampolineGetGasEstimate(BREthereumEWM ewm, BREthereumWallet wid, BREthereumCookie cookie,
										  const std::string &from, const std::string &to,
										  const std::string &amount, const std::string &gasPrice,
										  const std::string &data, int rid);

			void trampolineGetBalance(BREthereumEWM eid, BREthereumWallet wid, const std::string &address, int rid);

			void trampolineSubmitTransaction(BREthereumEWM eid, BREthereumWallet wid, BREthereumTransfer tid,
											 const std::string &rawTransaction, int rid);

			void trampolineGetTransactions(BREthereumEWM eid, const std::string &address, uint64_t begBlockNumber,
										   uint64_t endBlockNumber, int rid);

			void trampolineGetLogs(BREthereumEWM eid, const std::string &contract, const std::string &address,
								   const std::string &event, uint64_t begBlockNumber, uint64_t endBlockNumber, int rid);

			void trampolineGetBlocks(BREthereumEWM eid, const std::string &address, int interests,
									 uint64_t blockNumberStart, uint64_t blockNumberStop, int rid);

			void trampolineGetTokens(BREthereumEWM eid, int rid);

			void trampolineGetBlockNumber(BREthereumEWM eid, int rid);

			void trampolineGetNonce(BREthereumEWM ewm, const std::string &address, int rid);

			void trampolineEWMEvent(BREthereumEWM ewm, const BREthereumEWMEvent &event);

			void trampolinePeerEvent(BREthereumEWM ewm, const BREthereumPeerEvent &event);

			void trampolineWalletEvent(BREthereumEWM eid, BREthereumWallet wid, const BREthereumWalletEvent &event);

			void trampolineTokenEvent(BREthereumEWM eid, BREthereumToken tokenId, const BREthereumTokenEvent &event);

			void trampolineTransferEvent(BREthereumEWM eid, BREthereumWallet wid, BREthereumTransfer tid,
										 const BREthereumTransferEvent &event);

		public:
			BREthereumEWM getRaw() const;

		private:
			typedef std::map<BREthereumWallet, EthereumWalletPtr> WalletMap;
			typedef std::map<BREthereumTransfer, EthereumTransferPtr> TransferMap;
			typedef std::map<BREthereumBlock, EthereumBlockPtr> BlockMap;
			typedef std::map<std::string, EthereumTokenPtr> TokenAddressMap;
			typedef std::map<BREthereumToken, EthereumTokenPtr> TokenReferenceMap;
			WalletMap _wallets;
			TransferMap _transactions;
			BlockMap _blocks;
			TokenAddressMap _tokensByAddress;
			TokenReferenceMap _tokensByReference;

		private:
			BackgroundExecutor _executor;
			BREthereumEWM _ewm;
			Client *_client;
			EthereumNetworkPtr _network;
			EthereumAccountPtr _account;
		};

		typedef boost::shared_ptr<EthereumEWM> EthereumEWMPtr;

	}
}

#endif //ELASTOS_SPVSDK_ETHEREUMEWM_H
