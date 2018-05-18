// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef __ELASTOS_SDK_TRANSACTIONDATASTORE_H__
#define __ELASTOS_SDK_TRANSACTIONDATASTORE_H__

#include <boost/thread/mutex.hpp>
#include <boost/thread/shared_mutex.hpp>

#include "BRInt.h"
#include "Sqlite.h"
#include "c_util.h"

namespace Elastos {
	namespace SDK {

		struct TransactionEntity {
			TransactionEntity() :
				blockHeight(0),
				timeStamp(0),
				txHash("")
			{
			}

			TransactionEntity(CMBlock b, uint32_t bh, uint32_t ts, const std::string &th) :
				buff(b),
				blockHeight(bh),
				timeStamp(ts),
				txHash(th)
			{
			}

			CMBlock buff;
			uint32_t blockHeight;
			uint32_t timeStamp;
			std::string txHash;
		};

		class TransactionDataStore {
		public:
			TransactionDataStore(Sqlite *sqlite);
			TransactionDataStore(SqliteTransactionType type, Sqlite *sqlite);
			~TransactionDataStore();

			bool putTransaction(const std::string &iso, const TransactionEntity &transactionEntity);
			bool deleteAllTransactions(const std::string &iso);
			std::vector<TransactionEntity> getAllTransactions(const std::string &iso) const;
			bool updateTransaction(const std::string &iso, const TransactionEntity &transactionEntity);
			bool deleteTxByHash(const std::string &iso, const std::string &hash);

		private:
			Sqlite *_sqlite;
			SqliteTransactionType _txType;
			mutable boost::mutex _lockMutex;
			/*
			 * transaction table
			 */
			const std::string TX_TABLE_NAME = "transactionTable";
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
				TX_ISO + " text DEFAULT 'ELA' );";
		};

	}
}


#endif //SPVCLIENT_TRANSACTIONDATASTORE_H

