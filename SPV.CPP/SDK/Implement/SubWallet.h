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
#ifndef __ELASTOS_SDK_SUBWALLET_H__
#define __ELASTOS_SDK_SUBWALLET_H__

#include <P2P/ChainParams.h>
#include <SpvService/SpvService.h>
#include <Account/SubAccount.h>

#include <ISubWallet.h>
#include <ISubWalletCallback.h>

#include <map>
#include <boost/shared_ptr.hpp>
#include <boost/filesystem/path.hpp>

namespace Elastos {
	namespace ElaWallet {

#define SELA_PER_ELA 100000000
#define DEPOSIT_OR_WITHDRAW_FEE 10000

		class MasterWallet;

		class Transaction;

		class ChainConfig;

		class CoinInfo;

		typedef boost::shared_ptr<Transaction> TransactionPtr;
		typedef boost::shared_ptr<ChainConfig> ChainConfigPtr;
		typedef boost::shared_ptr<CoinInfo> CoinInfoPtr;

		class SubWallet : public virtual ISubWallet,
						  public Wallet::Listener,
						  public PeerManager::Listener,
						  public Lockable {
		public:
			typedef boost::shared_ptr<SpvService> WalletManagerPtr;

			virtual ~SubWallet();

			const WalletManagerPtr &GetWalletManager() const;

			virtual void StartP2P();

			virtual void StopP2P();

			virtual void FlushData();

			time_t GetFirstTxnTimestamp() const;

			virtual const std::string &GetInfoChainID() const;

		public: //implement ISubWallet
			virtual std::string GetChainID() const;

			virtual nlohmann::json GetBasicInfo() const;

			virtual nlohmann::json GetBalanceInfo() const;

			virtual std::string GetBalance() const;

			virtual std::string GetBalanceWithAddress(const std::string &address) const;

			virtual std::string CreateAddress();

			virtual nlohmann::json GetAllAddress(uint32_t start, uint32_t count, bool internal = false) const;

			virtual nlohmann::json GetAllPublicKeys(uint32_t start, uint32_t count) const;

			virtual void AddCallback(ISubWalletCallback *subCallback);

			virtual void RemoveCallback();

			virtual nlohmann::json CreateTransaction(
				const std::string &fromAddress,
				const std::string &targetAddress,
				const std::string &amount,
				const std::string &memo);

			virtual nlohmann::json GetAllUTXOs(uint32_t start, uint32_t count, const std::string &address) const;

			virtual nlohmann::json CreateConsolidateTransaction(
				const std::string &memo);

			virtual nlohmann::json SignTransaction(
				const nlohmann::json &tx,
				const std::string &payPassword) const;

			virtual nlohmann::json GetTransactionSignedInfo(
				const nlohmann::json &rawTransaction) const;

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

		protected: //implement Wallet::Listener
			virtual void onBalanceChanged(const uint256 &asset, const BigInt &balance);

			virtual void onTxAdded(const TransactionPtr &tx);

			virtual void onTxUpdated(const std::vector<TransactionPtr> &txns);

			virtual void onTxDeleted(const TransactionPtr &tx, bool notifyUser, bool recommendRescan);

			virtual void onAssetRegistered(const AssetPtr &asset, uint64_t amount, const uint168 &controller);

		protected: //implement PeerManager::Listener
			virtual void syncStarted();

			virtual void syncProgress(uint32_t progress, time_t lastBlockTime, uint32_t bytesPerSecond,
									  const std::string &downloadPeer);

			virtual void syncStopped(const std::string &error);

			virtual void txStatusUpdate() {}

			virtual void saveBlocks(bool replace, const std::vector<MerkleBlockPtr> &blocks);

			virtual void savePeers(bool replace, const std::vector<PeerInfo> &peers) {}

			virtual void saveBlackPeer(const PeerInfo &peer) {}

			virtual bool networkIsReachable() { return true; }

			virtual void txPublished(const std::string &hash, const nlohmann::json &result);

			virtual void connectStatusChanged(const std::string &status);

		protected:
			friend class MasterWallet;

			SubWallet(const CoinInfoPtr &info,
					  const ChainConfigPtr &config,
					  MasterWallet *parent,
					  const std::string &netType);

			SubWallet(const std::string &netType,
					  MasterWallet *parent,
					  const ChainConfigPtr &config,
					  const CoinInfoPtr &info);

			TransactionPtr CreateConsolidateTx(
				const std::string &memo,
				const uint256 &asset) const;

			nlohmann::json GetAllTransactionCommon(uint32_t start,
												   uint32_t count,
												   const std::string &txid,
												   TxnType type) const;

			virtual void publishTransaction(const TransactionPtr &tx);

			virtual void fireTransactionStatusChanged(const uint256 &txid, const std::string &status,
													  const nlohmann::json &desc, uint32_t confirms);

			const CoinInfoPtr &GetCoinInfo() const;

			void EncodeTx(nlohmann::json &result, const TransactionPtr &tx) const;

			TransactionPtr DecodeTx(const nlohmann::json &encodedTx) const;

		protected:
			WalletManagerPtr _walletManager;
			ISubWalletCallback *_callback;
			MasterWallet *_parent;
			CoinInfoPtr _info;
			ChainConfigPtr _config;
		};

	}
}

#endif //__ELASTOS_SDK_SUBWALLET_H__
