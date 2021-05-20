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

		class EthSidechainSubWallet : public virtual IEthSidechainSubWallet,
									  public EthereumEWM::Client,
									  public Lockable {
		public: // implement IEthSidechainSubWallet
			virtual ~EthSidechainSubWallet();

			virtual nlohmann::json CreateTransfer(const std::string &targetAddress,
												  const std::string &amount,
												  EthereumAmountUnit amountUnit) const;

			virtual nlohmann::json CreateTransferGeneric(const std::string &targetAddress,
														 const std::string &amount,
														 EthereumAmountUnit amountUnit,
														 const std::string &gasPrice,
														 EthereumAmountUnit gasPriceUnit,
														 const std::string &gasLimit,
														 const std::string &data) const;

			virtual void DeleteTransfer(const nlohmann::json &tx);

			virtual nlohmann::json GetTokenTransactions(uint32_t start, uint32_t count, const std::string &txid,
														const std::string &tokenSymbol) const;

            virtual void SyncStart();

            virtual void SyncStop();

		public:
			// implement callback of Client
			virtual void getGasPrice(BREthereumWallet wid, int rid);

			virtual void getGasEstimate(BREthereumWallet wid,
										BREthereumCookie cookie,
										const std::string &from,
										const std::string &to,
										const std::string &amount,
										const std::string &gasPrice,
										const std::string &data,
										int rid);

			virtual void getBalance(BREthereumWallet wid, const std::string &address, int rid);

			virtual void
			submitTransaction(BREthereumWallet wid, BREthereumTransfer tid, const std::string &rawTransaction,
							  int rid);

			// announce one-by-one
			virtual void
			getTransactions(const std::string &address, uint64_t begBlockNumber, uint64_t endBlockNumber,
							int rid);

			// announce one-by-one
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

			// announce one-by-one
			virtual void getTokens(int rid);

			virtual void getBlockNumber(int rid);

			virtual void getNonce(const std::string &address, int rid);

			virtual void handleEWMEvent(const BREthereumEWMEvent &event);

			virtual void handlePeerEvent(const BREthereumPeerEvent &event);

			virtual void handleWalletEvent(const EthereumWalletPtr &wallet,
										   const BREthereumWalletEvent &event);

			virtual void handleTokenEvent(const EthereumTokenPtr &token, const BREthereumTokenEvent &event);

			virtual void handleTransferEvent(const EthereumWalletPtr &wallet,
											 const EthereumTransferPtr &transaction,
											 const BREthereumTransferEvent &event);
			// implement ISubWallet
		public:
			virtual std::string GetChainID() const;

			virtual nlohmann::json GetBasicInfo() const;

			virtual std::string GetBalance() const;

			virtual std::string CreateAddress();

			virtual nlohmann::json GetAllAddress(
				uint32_t start,
				uint32_t count,
				bool internal = false) const;

            virtual std::vector<std::string> GetLastAddresses(bool internal) const;

            virtual void UpdateUsedAddress(const std::vector<std::string> &usedAddresses) const;

			virtual nlohmann::json GetAllPublicKeys(
				uint32_t start,
				uint32_t count) const;

			virtual void AddCallback(ISubWalletCallback *subCallback);

			virtual void RemoveCallback();

			virtual nlohmann::json CreateTransaction(
                    const nlohmann::json &inputs,
                    const nlohmann::json &outputs,
                    const std::string &fee,
                    const std::string &memo);

			virtual nlohmann::json SignTransaction(
				const nlohmann::json &tx,
				const std::string &payPassword) const;

			virtual nlohmann::json GetTransactionSignedInfo(
				const nlohmann::json &tx) const;

			virtual nlohmann::json PublishTransaction(
				const nlohmann::json &tx);

			virtual std::string ConvertToRawTransaction(const nlohmann::json &tx);

			virtual void StartP2P();

			virtual void StopP2P();

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
			ISubWalletCallback *_callback;
		};

	}
}

#endif //ELASTOS_SPVSDK_ETHSIDECHAINSUBWALLET_H
