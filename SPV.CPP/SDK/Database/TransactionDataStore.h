// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef __ELASTOS_SDK_TRANSACTIONDATASTORE_H__
#define __ELASTOS_SDK_TRANSACTIONDATASTORE_H__

#include "Sqlite.h"
#include "TableBase.h"

#include <SDK/Common/uint256.h>

namespace Elastos {
	namespace ElaWallet {

		class Transaction;
		typedef boost::shared_ptr<Transaction> TransactionPtr;

		class TransactionDataStore : public TableBase {
		public:
			TransactionDataStore(Sqlite *sqlite);
			TransactionDataStore(SqliteTransactionType type, Sqlite *sqlite);
			~TransactionDataStore();

			bool PutTransaction(const std::string &iso, const TransactionPtr &tx);
			bool PutTransactions(const std::string &iso, const std::vector<TransactionPtr> &txns);
			bool DeleteAllTransactions();
			size_t GetAllTransactionsCount() const;
			std::vector<TransactionPtr> GetAllTransactions() const;
			bool UpdateTransaction(const std::vector<uint256> &hashes, uint32_t blockHeight, time_t timestamp);
			bool DeleteTxByHash(const uint256 &hash);
			bool DeleteTxByHashes(const std::vector<uint256> &hashes);

		private:
			bool SelectTxByHash(const std::string &iso, const std::string &hash, TransactionPtr &tx) const;

			void PutTransactionInternal(const std::string &iso, const TransactionPtr &tx);

		private:
			/*
			 * transaction table
			 */
			const std::string TX_TABLE_NAME = "transactionTable";
			const std::string TX_COLUMN_ID = "_id";
			const std::string TX_BUFF = "transactionBuff";
			const std::string TX_BLOCK_HEIGHT = "transactionBlockHeight";
			const std::string TX_TIME_STAMP = "transactionTimeStamp";
			const std::string TX_ISO = "transactionISO";
			const std::string TX_REMARK = "transactionRemark";
			const std::string TX_ASSETID = "assetID";

			const std::string TX_DATABASE_CREATE = "create table if not exists " + TX_TABLE_NAME + " (" +
				TX_COLUMN_ID + " text not null, " +
				TX_BUFF + " blob, " +
				TX_BLOCK_HEIGHT + " integer, " +
				TX_TIME_STAMP + " integer, " +
				TX_REMARK + " text DEFAULT '', " +
				TX_ASSETID + " text not null, " +
				TX_ISO + " text DEFAULT 'ELA' );";
		};

	}
}


#endif //SPVCLIENT_TRANSACTIONDATASTORE_H

