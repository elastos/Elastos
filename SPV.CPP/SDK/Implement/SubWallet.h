// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

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

			void StartP2P();

			void StopP2P();

			void FlushData();

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
					const std::string &toAddress,
					const std::string &amount,
					const std::string &memo);

			virtual nlohmann::json GetAllUTXOs(uint32_t start, uint32_t count, const std::string &address) const;

			virtual nlohmann::json CreateConsolidateTransaction(
					const std::string &memo);

			virtual nlohmann::json SignTransaction(
					const nlohmann::json &createdTx,
					const std::string &payPassword) const;

			virtual nlohmann::json GetTransactionSignedInfo(
					const nlohmann::json &rawTransaction) const;

			virtual nlohmann::json PublishTransaction(
					const nlohmann::json &signedTx);

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

		protected: //implement Wallet::Listener
			virtual void balanceChanged(const uint256 &asset, const BigInt &balance);

			virtual void onCoinbaseTxAdded(const TransactionPtr &tx);

			virtual void onCoinbaseTxMove(const std::vector<TransactionPtr> &txns);

			virtual void onCoinbaseTxUpdated(const std::vector<uint256> &hashes, uint32_t blockHeight, time_t timestamp);

			virtual void onCoinbaseTxDeleted(const uint256 &hash, bool notifyUser, bool recommendRescan);

			virtual void onTxAdded(const TransactionPtr &tx);

			virtual void onTxUpdated(const std::vector<uint256> &hashes, uint32_t blockHeight, time_t timeStamp);

			virtual void onTxDeleted(const uint256 &hash, bool notifyUser, bool recommendRescan);

			virtual void onTxUpdatedAll(const std::vector<TransactionPtr> &txns);

			virtual void onAssetRegistered(const AssetPtr &asset, uint64_t amount, const uint168 &controller);

		protected: //implement PeerManager::Listener
			virtual void syncStarted();

			virtual void syncProgress(uint32_t progress, time_t lastBlockTime, uint32_t bytesPerSecond, const std::string &downloadPeer);

			virtual void syncStopped(const std::string &error);

			virtual void txStatusUpdate() {}

			virtual void saveBlocks(bool replace, const std::vector<MerkleBlockPtr> &blocks);

			virtual void savePeers(bool replace, const std::vector<PeerInfo> &peers) {}

			virtual void saveBlackPeer(const PeerInfo &peer) {}

			virtual bool networkIsReachable() { return true; }

			virtual void txPublished(const std::string &hash, const nlohmann::json &result);

			virtual void syncIsInactive(uint32_t time) {}

			virtual void connectStatusChanged(const std::string &status);

		protected:
			friend class MasterWallet;

			SubWallet(const CoinInfoPtr &info,
					  const ChainConfigPtr &config,
					  MasterWallet *parent,
					  const std::string &netType);

			TransactionPtr CreateConsolidateTx(
				const std::string &memo,
				const uint256 &asset) const;

			virtual void publishTransaction(const TransactionPtr &tx);

			virtual void fireTransactionStatusChanged(const uint256 &txid, const std::string &status,
													  const nlohmann::json &desc, uint32_t confirms);

			const CoinInfoPtr &GetCoinInfo() const;

			void EncodeTx(nlohmann::json &result, const TransactionPtr &tx) const;

			TransactionPtr DecodeTx(const nlohmann::json &encodedTx) const;

		protected:
			WalletManagerPtr _walletManager;
			ISubWalletCallback * _callback;
			MasterWallet *_parent;
			CoinInfoPtr _info;
			ChainConfigPtr _config;
		};

	}
}

#endif //__ELASTOS_SDK_SUBWALLET_H__
