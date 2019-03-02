// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef __ELASTOS_SDK__GROUPEDTRANSACTIONS_H__
#define __ELASTOS_SDK__GROUPEDTRANSACTIONS_H__

#include <SDK/TransactionHub/UTXOList.h>
#include <SDK/Common/ElementSet.h>
#include <SDK/Common/UInt256ValueSet.h>
#include <SDK/Plugin/Transaction/Transaction.h>
#include <SDK/Plugin/Transaction/Asset.h>
#include <SDK/Account/ISubAccount.h>

#include <map>
#include <boost/function.hpp>
#include <boost/weak_ptr.hpp>

namespace Elastos {
	namespace ElaWallet {

	class TransactionHub;
	class GroupedAssetTransactions;

	class AssetTransactions {
		public:
			enum BalanceType {
				Default,
				Voted,
				Total,
			};

		public:
			class Listener {
			public:
				virtual void balanceChanged(const UInt256 &asset, uint64_t balance) = 0;

				virtual void onTxAdded(const TransactionPtr &transaction) = 0;

				virtual void onTxUpdated(const std::string &hash, uint32_t blockHeight, uint32_t timeStamp) = 0;

				virtual void onTxDeleted(const std::string &hash, const std::string &assetID, bool notifyUser,
							bool recommendRescan) = 0;
			};

		public:
			AssetTransactions(GroupedAssetTransactions *parent, Lockable *lockable, const SubAccountPtr &subAccount,
							  const std::vector<std::string> &listeningAddrs,
							  const boost::shared_ptr<Listener> &listener);

			AssetTransactions(const AssetTransactions &proto);

			AssetTransactions &operator=(const AssetTransactions &proto);

			bool HasTransactions() const;

			const std::vector<TransactionPtr> &GetTransactions() const;

			bool Exist(const TransactionPtr &tx);

			bool Exist(const UInt256 &hash);

			const TransactionPtr GetExistTransaction(const UInt256 &hash) const;

			void SortTransaction();

			void Append(const TransactionPtr &transaction);

			void BatchSet(const boost::function<void(const TransactionPtr &)> &fun);

			const std::vector<UTXO> &GetUTXOs() const;

			uint64_t GetBalance(BalanceType type = Default) const;

			uint64_t GetFeePerKb() const;

			void SetFeePerKb(uint64_t value);

			void CleanBalance();

			void UpdateBalance();

			nlohmann::json GetBalanceInfo();

			TransactionPtr CreateTxForFee(const std::vector<TransactionOutput> &outputs,
										  const Address &fromAddress,
										  uint64_t fee, bool useVotedUTXO);

			void UpdateTxFee(TransactionPtr &tx, uint64_t fee, const Address &fromAddress);

			std::vector<TransactionPtr> TxUnconfirmedBefore(uint32_t blockHeight);

			void SetTxUnconfirmedAfter(uint32_t blockHeight);

			bool TransactionIsValid(const TransactionPtr &tx);

			bool RegisterTransaction(const TransactionPtr &tx);

			void RemoveTransaction(const UInt256 &txHash);

			uint64_t AmountSentByTx(const TransactionPtr &tx);

			bool WalletContainsTx(const TransactionPtr &tx);

			void UpdateTransactions(const std::vector<UInt256> &txHashes, uint32_t blockHeight, uint32_t timestamp);

		protected:
			void balanceChanged(const UInt256 &asset, uint64_t balance);

			void txAdded(const TransactionPtr &tx);

			void txUpdated(const std::vector<UInt256> &txHashes, uint32_t blockHeight, uint32_t timestamp);

			void txDeleted(const UInt256 &txHash, const UInt256 &assetID, bool notifyUser, bool recommendRescan);

		private:
			uint64_t getMinOutputAmount();

			uint64_t getMaxOutputAmount();

		private:
			typedef ElementSet<TransactionPtr> TransactionSet;
			std::vector<TransactionPtr> _transactions;
			uint64_t _balance, _votedBalance, _lockedBalance, _depositBalance, _feePerKb;
			UTXOList _utxos, _utxosLocked;
			UTXOList _spentOutputs;
			TransactionSet _allTx, _invalidTx;

			Lockable *_lockable;
			SubAccountPtr _subAccount;
			std::vector<std::string> _listeningAddrs;
			boost::weak_ptr<Listener> _listener;

			GroupedAssetTransactions *_parent;
		};

		typedef boost::shared_ptr<AssetTransactions> AssetTransactionsPtr;

		class GroupedAssetTransactions {
		public:
			GroupedAssetTransactions(TransactionHub *parent, const std::vector<Asset> &assetArray,
									 const std::vector<TransactionPtr> &txns,
									 const SubAccountPtr &subAccount,
									 const boost::shared_ptr<AssetTransactions::Listener> &listener);

			void InitListeningAddresses(const std::vector<std::string> &addrs);

			std::vector<TransactionPtr> GetTransactions(const UInt256 &assetID) const;

			std::vector<TransactionPtr> GetAllTransactions() const;

			const std::vector<UTXO> &GetUTXOs(const UInt256 &assetID) const;

			const std::vector<UTXO> GetAllUTXOs() const;

			void Append(const TransactionPtr &transaction);

			bool Empty() const;

			void ForEach(const boost::function<void(const UInt256 &, const AssetTransactionsPtr &)> &fun);

			void BatchSet(const boost::function<void(const TransactionPtr &)> &fun);

			void UpdateAssets(const std::vector<Asset> &assetArray);

			AssetTransactionsPtr &operator[](const UInt256 &assetID);

			const AssetTransactionsPtr &Get(const UInt256 &assetID) const;

			TransactionPtr CreateTxForFee(const std::vector<TransactionOutput> &outputs, const Address &fromAddress,
										  uint64_t fee, bool useVotedUTXO);

			void UpdateTxFee(TransactionPtr &tx, uint64_t fee, const Address &fromAddress);

			TransactionPtr TransactionForHash(const UInt256 &transactionHash);

			TransactionPtr TransactionForHash(const UInt256 &transactionHash, const UInt256 &assetID);

			bool RegisterTransaction(const TransactionPtr &tx);

			void RemoveTransaction(const UInt256 &txHash);

			void UpdateTransactions(const std::vector<UInt256> &txHashes, uint32_t blockHeight, uint32_t timestamp);

			void SetTxUnconfirmedAfter(uint32_t blockHeight);

			bool WalletContainsTx(const TransactionPtr &tx);

			bool WalletExistTx(const TransactionPtr &tx);

			void InitWithTransactions(const std::vector<TransactionPtr> &txArray);

			nlohmann::json GetAllSupportedAssets() const;

			bool ContainsAsset(const std::string &assetID);

			bool ContainsAsset(const UInt256 &assetID);

			uint32_t GetBlockHeight() const;

			void SetBlockHeight(uint32_t height);

		private:
			UInt256 GetUniqueAssetID(const std::vector<TransactionOutput> &outputs) const;

		private:
			typedef UInt256ValueMap<AssetTransactionsPtr> AssetTransactionMap;
			mutable AssetTransactionMap _groupedTransactions;

			uint32_t _blockHeight;
			TransactionHub *_parent;
			SubAccountPtr _subAccount;
			std::vector<std::string> _listeningAddrs;
			boost::shared_ptr<AssetTransactions::Listener> _listener;
		};

	}
}


#endif //__ELASTOS_SDK__GROUPEDTRANSACTIONS_H__
