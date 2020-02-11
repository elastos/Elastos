// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef __ELASTOS_SDK_TRANSACTIONDATASTORE_H__
#define __ELASTOS_SDK_TRANSACTIONDATASTORE_H__

#include "Sqlite.h"
#include "TableBase.h"

#include <Common/uint256.h>

namespace Elastos {
	namespace ElaWallet {

		class Transaction;

		typedef boost::shared_ptr<Transaction> TransactionPtr;

		class TransactionDataStore : public TableBase {
		public:
			TransactionDataStore();

			TransactionDataStore(Sqlite *sqlite);

			TransactionDataStore(SqliteTransactionType type, Sqlite *sqlite);

			~TransactionDataStore();

			bool PutTransaction(const std::string &iso, const TransactionPtr &tx);

			bool PutTransactions(const std::string &iso, const std::vector<TransactionPtr> &txns);

			bool DeleteAllTransactions();

			size_t GetAllTransactionsCount() const;

			TransactionPtr GetTransaction(const uint256 &hash, const std::string &chainID);

			std::vector<TransactionPtr> GetAllTransactions(const std::string &chainID) const;

			bool UpdateTransaction(const std::vector<uint256> &hashes, uint32_t blockHeight, time_t timestamp);

			bool DeleteTxByHash(const uint256 &hash);

			bool DeleteTxByHashes(const std::vector<uint256> &hashes);

		private:
			TransactionPtr SelectTxByHash(const std::string &hash, const std::string &chainID) const;

			bool ContainHash(const std::string &hash) const;

			bool PutTransactionInternal(const std::string &iso, const TransactionPtr &tx);

			void Init();

		protected:
			std::string _tableName;
			std::string _txHash;
			std::string _buff;
			std::string _blockHeight;
			std::string _timestamp;
			std::string _iso;
			std::string _remark;
			std::string _assetID;
			std::string _tableCreation;
		};

	} // namespace ElaWallet
} // namespace Elastos

#endif // SPVCLIENT_TRANSACTIONDATASTORE_H
