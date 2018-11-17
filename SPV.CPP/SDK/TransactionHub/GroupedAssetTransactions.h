// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef __ELASTOS_SDK__GROUPEDTRANSACTIONS_H__
#define __ELASTOS_SDK__GROUPEDTRANSACTIONS_H__

#include <map>
#include <boost/function.hpp>

#include "SDK/TransactionHub/UTXOList.h"
#include "SDK/Common/ElementSet.h"
#include "UInt256ValueSet.h"
#include "Plugin/Transaction/Transaction.h"
#include "Plugin/Transaction/Asset.h"
#include "SDK/Account/ISubAccount.h"

namespace Elastos {
	namespace ElaWallet {

		class AssetTransactions {
		public:
			AssetTransactions(Lockable *lockable, const SubAccountPtr &subAccount,
							  const std::vector<std::string> &listeningAddrs);

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

			uint64_t GetBalance() const;

			void SetBalance(uint64_t balance);

			uint64_t GetTotalSent() const;

			void SetTotalSent(uint64_t value);

			uint64_t GetTotalReceived() const;

			void SetTotalReceived(uint64_t value);

			uint64_t GetFeePerKb() const;

			void SetFeePerKb(uint64_t value);

			const std::vector<uint64_t> &GetBalanceHistory() const;

			void CleanBalance();

			void UpdateBalance(uint32_t blockHeight);

			TransactionPtr
			CreateTxForOutputs(const std::vector<TransactionOutput> &outputs,
							   const std::string &fromAddress,
							   const boost::function<bool(const std::string &, const std::string &)> &filter);

			std::vector<TransactionPtr> TxUnconfirmedBefore(uint32_t blockHeight);

			std::vector<UInt256> SetTxUnconfirmedAfter(uint32_t blockHeight);

			bool TransactionIsValid(const TransactionPtr &transaction);

			bool RegisterTransaction(const TransactionPtr &transaction, uint32_t blockHeight, bool &wasAdded);

			bool RemoveTransaction(const UInt256 &transactionHash, uint32_t blockHeight,
								   std::vector<UInt256> &removedTransactions, UInt256 &removedAssetID, bool &notifyUser,
								   bool &recommendRescan);

			uint64_t AmountSentByTx(const TransactionPtr &tx);

			bool WalletContainsTx(const TransactionPtr &tx);

			std::vector<UInt256> UpdateTransactions(const std::vector<UInt256> &transactionsHashes, uint32_t blockHeight,
									uint32_t lastBlockHeight, uint32_t timestamp);

		private:
			uint64_t getMinOutputAmount();

			uint64_t getMaxOutputAmount();

		private:
			typedef ElementSet<TransactionPtr> TransactionSet;
			std::vector<TransactionPtr> _transactions;
			uint64_t _balance, _totalSent, _totalReceived, _feePerKb;
			UTXOList _utxos;
			UTXOList _spentOutputs;
			TransactionSet _allTx, _invalidTx, _pendingTx;
			std::vector<uint64_t> _balanceHist;

			Lockable *_lockable;
			SubAccountPtr _subAccount;
			std::vector<std::string> _listeningAddrs;
		};

		typedef boost::shared_ptr<AssetTransactions> AssetTransactionsPtr;

		class GroupedAssetTransactions {
		public:
			GroupedAssetTransactions(Lockable *lockable, const SubAccountPtr &subAccount);

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

			TransactionPtr
			CreateTxForOutputs(const std::vector<TransactionOutput> &outputs,
							   const std::string &fromAddress,
							   const boost::function<bool(const std::string &, const std::string &)> &filter);

			TransactionPtr TransactionForHash(const UInt256 &transactionHash);

			TransactionPtr TransactionForHash(const UInt256 &transactionHash, const UInt256 &assetID);

			void RemoveTransaction(const UInt256 &transactionHash, uint32_t blockHeight,
								   std::vector<UInt256> &removedTransactions, UInt256 &removedAssetID, bool &notifyUser,
								   bool &recommendRescan);

			std::vector<UInt256> UpdateTransactions(const std::vector<UInt256> &transactionsHashes, uint32_t blockHeight,
									uint32_t lastBlockHeight, uint32_t timestamp);

			bool WalletContainsTx(const TransactionPtr &tx);

			void InitWithTransactions(const std::vector<TransactionPtr> &txArray);

			nlohmann::json GetAllSupportedAssets() const;

			bool ContainsAsset(const std::string &assetID);

			bool ContainsAsset(const UInt256 &assetID);

		private:
			UInt256 GetUniqueAssetID(const std::vector<TransactionOutput> &outputs) const;

		private:
			typedef UInt256ValueMap<AssetTransactionsPtr> AssetTransactionMap;
			mutable AssetTransactionMap _groupedTransactions;

			Lockable *_lockable;
			SubAccountPtr _subAccount;
			std::vector<std::string> _listeningAddrs;
		};

	}
}


#endif //__ELASTOS_SDK__GROUPEDTRANSACTIONS_H__
