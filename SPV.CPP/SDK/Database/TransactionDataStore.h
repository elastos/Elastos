// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef __ELASTOS_SDK_TRANSACTIONDATASTORE_H__
#define __ELASTOS_SDK_TRANSACTIONDATASTORE_H__

#include "Sqlite.h"

namespace Elastos {
	namespace SDK {

		typedef struct {
			std::string buff;
			long blockheight;
			long timestamp;
			std::string txHash;
			std::string txISO;
		} BRTransactionEntity;

		class TransactionDataStore {
		public:
			TransactionDataStore();
			~TransactionDataStore();

			bool putTransaction(Sqlite& sqlite, std::string& iso, BRTransactionEntity& transactionEntity);

//		private:
//			int putTransactionCallBack(void* arg, int column, char** value, char** header);

		private:
			/**
			* Transaction table
			*/
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
