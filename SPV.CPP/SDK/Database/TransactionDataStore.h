// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef __ELASTOS_SDK_TRANSACTIONDATASTORE_H__
#define __ELASTOS_SDK_TRANSACTIONDATASTORE_H__

#include "ByteData.h"
#include "BRInt.h"
#include "Sqlite.h"

namespace Elastos {
	namespace SDK {

		typedef struct {
			ByteData buff;
			uint32_t blockheight;
			uint32_t timestamp;
			UInt256 txHash;
			std::string txISO;
		} TransactionEntity;

		class TransactionDataStore {
		public:
			TransactionDataStore(Sqlite *sqlite);
			~TransactionDataStore();

			bool putTransaction(const std::string &iso, const TransactionEntity &transactionEntity);
			bool deleteAllTransactions(const std::string &iso);
			std::vector<TransactionEntity> getAllTransactions(const std::string &iso) const;
			bool updateTransaction(const std::string &iso, const TransactionEntity &transactionEntity);
			bool deleteTxByHash(const std::string &iso, const std::string &hash);

		private:
			Sqlite *_sqlite;
			/*
			 * Transaction table
			 */
			const std::string TX_TABLE_NAME_OLD = "transactionTable";
			const std::string TX_TABLE_NAME = "transactionTable_v2";
			const std::string TX_COLUMN_ID = "_id";
			const std::string TX_BUFF = "transactionBuff";
			const std::string TX_BLOCK_HEIGHT = "transactionBlockHeight";
			const std::string TX_TIME_STAMP = "transactionTimeStamp";
			const std::string TX_ISO = "transactionISO";

			const std::string TX_DATABASE_CREATE = "create table if not exists " + TX_TABLE_NAME + " (" +
				TX_COLUMN_ID + " text, " +
				TX_BUFF + " blob, " +
				TX_BLOCK_HEIGHT + " integer, " +
				TX_TIME_STAMP + " integer, " +
				TX_ISO + " text DEFAULT 'BTC' );";
		};

	}
}


#endif //SPVCLIENT_TRANSACTIONDATASTORE_H
