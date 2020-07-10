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

#ifndef __ELASTOS_SDK_ETHSIDECHAINSUBWALLET_H__
#define __ELASTOS_SDK_ETHSIDECHAINSUBWALLET_H__

#include "IEthSidechainSubWallet.h"
#include "ISubWallet.h"
#include "Ethereum/EthereumEWM.h"
#include "Common/Lockable.h"

#include <boost/shared_ptr.hpp>
#include <boost/filesystem/path.hpp>
#include <nlohmann/json.hpp>

namespace Elastos {
	namespace ElaWallet {

		class MasterWallet;
		class ChainConfig;
		class CoinInfo;
		class EthereumClient;

		typedef boost::shared_ptr<ChainConfig> ChainConfigPtr;
		typedef boost::shared_ptr<CoinInfo> CoinInfoPtr;
		typedef boost::shared_ptr<EthereumClient> ClientPtr;

		class EthSidechainSubWallet : public IEthSidechainSubWallet,
									  public EthereumEWM::Client,
									  public Lockable {
		public: // implement IEthSidechainSubWallet
			virtual ~EthSidechainSubWallet();

		public:
			// implement callback of Client
			virtual void getGasPrice(BREthereumWallet wid, int rid);

			virtual void getGasEstimate(BREthereumWallet wid,
										BREthereumTransfer tid,
										const std::string &from,
										const std::string &to,
										const std::string &amount,
										const std::string &data,
										int rid);

			virtual void getBalance(BREthereumWallet wid, const std::string &address, int rid);

			virtual void
			submitTransaction(BREthereumWallet wid, BREthereumTransfer tid, const std::string &rawTransaction,
							  int rid);

			virtual void
			getTransactions(const std::string &address, uint64_t begBlockNumber, uint64_t endBlockNumber,
							int rid);

			virtual void getLogs(const std::string &contract,
								 const std::string &address,
								 const std::string &event,
								 uint64_t begBlockNumber,
								 uint64_t endBlockNumber,
								 int rid);

			virtual void getBlocks(const std::string &address,
								   int interests,
								   uint64_t blockNumberStart,
								   uint64_t blockNumberStop,
								   int rid);

			virtual void getTokens(int rid);

			virtual void getBlockNumber(int rid);

			virtual void getNonce(const std::string &address, int rid);

			virtual void handleEWMEvent(EthereumEWM::EWMEvent event, EthereumEWM::Status status,
										const std::string &errorDescription);

			virtual void handlePeerEvent(EthereumEWM::PeerEvent event, EthereumEWM::Status status,
										 const std::string &errorDescription);

			virtual void handleWalletEvent(const EthereumWalletPtr &wallet,
										   EthereumEWM::WalletEvent event, EthereumEWM::Status status,
										   const std::string &errorDescription);

			virtual void handleTokenEvent(const EthereumTokenPtr &token, EthereumEWM::TokenEvent event);

			virtual void handleBlockEvent(const EthereumBlockPtr &block,
										  EthereumEWM::BlockEvent event, EthereumEWM::Status status,
										  const std::string &errorDescription);

			virtual void handleTransferEvent(const EthereumWalletPtr &wallet,
											 const EthereumTransferPtr &transaction,
											 EthereumEWM::TransactionEvent event, EthereumEWM::Status status,
											 const std::string &errorDescription);
			// implement ISubWallet
		public:
			virtual std::string GetChainID() const;

			virtual nlohmann::json GetBasicInfo() const;

			virtual nlohmann::json GetBalanceInfo() const;

			virtual std::string GetBalance() const;

			virtual std::string GetBalanceWithAddress(const std::string &address) const;

			virtual std::string CreateAddress();

			virtual nlohmann::json GetAllAddress(
				uint32_t start,
				uint32_t count,
				bool internal = false) const;

			virtual nlohmann::json GetAllPublicKeys(
				uint32_t start,
				uint32_t count) const;

			virtual void AddCallback(ISubWalletCallback *subCallback);

			virtual void RemoveCallback();

			virtual nlohmann::json CreateTransaction(
				const std::string &fromAddress,
				const std::string &toAddress,
				const std::string &amount,
				const std::string &memo);

			virtual nlohmann::json GetAllUTXOs(
				uint32_t start,
				uint32_t count,
				const std::string &address) const;

			virtual nlohmann::json CreateConsolidateTransaction(
				const std::string &memo);

			virtual nlohmann::json SignTransaction(
				const nlohmann::json &tx,
				const std::string &payPassword) const;

			virtual nlohmann::json GetTransactionSignedInfo(
				const nlohmann::json &tx) const;

			virtual nlohmann::json PublishTransaction(
				const nlohmann::json &tx);

			virtual std::string ConvertToRawTransaction(const nlohmann::json &tx);

			virtual nlohmann::json GetAllTransaction(
				uint32_t start,
				uint32_t count,
				const std::string &txid) const;

			virtual nlohmann::json GetAllCoinBaseTransaction(
				uint32_t start,
				uint32_t count,
				const std::string &txID) const;

			virtual nlohmann::json GetAssetInfo(
				const std::string &assetID) const;

			virtual bool SetFixedPeer(const std::string &address, uint16_t port);

			virtual void SyncStart();

			virtual void SyncStop();

			virtual void Resync();

			virtual void StartP2P();

			virtual void StopP2P();

			virtual void FlushData();
		protected:
			friend class MasterWallet;

			EthSidechainSubWallet(const CoinInfoPtr &info,
								  const ChainConfigPtr &config,
								  MasterWallet *parent,
								  const std::string &netType);

			std::string GetTransferID(const EthereumTransferPtr &tx) const;

			EthereumTransferPtr LookupTransfer(const std::string &tid) const;

		protected:
			std::string _walletID;
			ClientPtr _client;
			MasterWallet *_parent;
			CoinInfoPtr _info;
			ChainConfigPtr _config;
			ISubWalletCallback *_callback;
		};

	}
}

#endif //ELASTOS_SPVSDK_ETHSIDECHAINSUBWALLET_H
