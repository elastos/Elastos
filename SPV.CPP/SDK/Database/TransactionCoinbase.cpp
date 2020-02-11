// Copyright (c) 2012-2019 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "TransactionCoinbase.h"

#include <Common/Log.h>
#include <WalletCore/Address.h>
#include <Plugin/Transaction/TransactionOutput.h>
#include <Plugin/Transaction/Transaction.h>
#include <Plugin/Transaction/Payload/CoinBase.h>
#include <cstdint>

namespace Elastos {
	namespace ElaWallet {

		TransactionCoinbase::TransactionCoinbase(Sqlite *sqlite) : TransactionDataStore() {
			_sqlite = sqlite;
			_tableNameOld = "coinBaseUTXOTable";
			_txHashOld = "txHash";
			_blockHeightOld = "blockHeight";
			_timestampOld = "timestamp";
			_indexOld = "outputIndex";
			_programHashOld = "programHash";
			_assetIDOld = "assetID";
			_outputLockOld = "outputLock";
			_amountOld = "amount";
			_payloadOld = "payload";
			_spentOld = "spent";

			// new table
			_tableName = "transactionCoinbase";
			_tableCreation = "create table if not exists " +
							 _tableName + "(" +
							 _txHash + " text not null, " +
							 _buff + " blob, " +
							 _blockHeight + " integer, " +
							 _timestamp + " integer, " +
							 _remark + " text DEFAULT '', " +
							 _assetID + " text not null, " +
							 _iso + " text DEFAULT 'ELA');";
			InitializeTable(_tableCreation);
			ConvertFromOldData();
			InitializeTable("drop table if exists " + _tableNameOld + ";");
		}

		TransactionCoinbase::~TransactionCoinbase() {

		}

		bool TransactionCoinbase::ContainOldData() const {
			std::string sql;
			int count = 0;

			sql = "select count(*)  from sqlite_master where type='table' and name = '" + _tableNameOld + "';";

			sqlite3_stmt *stmt;
			if (!_sqlite->Prepare(sql, &stmt, nullptr)) {
				Log::error("prepare sql: {}", sql);
				return false;
			}

			if (SQLITE_ROW == _sqlite->Step(stmt)) {
				count = _sqlite->ColumnInt(stmt, 0);
			}

			if (!_sqlite->Finalize(stmt)) {
				Log::error("Coinbase update finalize");
				return false;
			}

			return count > 0;
		}

		void TransactionCoinbase::ConvertFromOldData() {
			if (ContainOldData()) {
				std::vector<TransactionPtr> txns;

				std::string sql;

				sql = "SELECT " + _txHashOld + ", " + _blockHeightOld + ", " + _timestampOld + ", " + _indexOld + ", " +
					  _programHashOld + ", " + _assetIDOld + ", " + _outputLockOld + ", " + _amountOld + ", " +
					  _payloadOld + ", " + _spentOld + " FROM " + _tableNameOld + ";";

				sqlite3_stmt *stmt;
				if (!_sqlite->Prepare(sql, &stmt, nullptr)) {
					Log::error("prepare sql: {}", sql);
					return ;
				}

				while (SQLITE_ROW == _sqlite->Step(stmt)) {
					uint256 txHash(_sqlite->ColumnText(stmt, 0));
					uint32_t blockHeight = _sqlite->ColumnInt(stmt, 1);
					time_t timestamp = _sqlite->ColumnInt64(stmt, 2);
					uint16_t index = (uint16_t) _sqlite->ColumnInt(stmt, 3);
					uint168 programHash(*_sqlite->ColumnBlobBytes(stmt, 4));
					uint256 assetID(*_sqlite->ColumnBlobBytes(stmt, 5));
					uint32_t outputLock = _sqlite->ColumnInt(stmt, 6);
					BigInt amount;
					amount.setDec(_sqlite->ColumnText(stmt, 7));
					_sqlite->ColumnBlobBytes(stmt, 8);
					bool spent = _sqlite->ColumnInt(stmt, 9) != 0;

					OutputPtr o(new TransactionOutput(amount, Address(programHash), assetID));
					o->SetOutputLock(outputLock);
					o->SetFixedIndex(index);

					TransactionPtr tx(new Transaction(Transaction::coinBase, PayloadPtr(new CoinBase())));
					tx->SetHash(txHash);
					tx->SetBlockHeight(blockHeight);
					tx->SetTimestamp(timestamp);

					tx->AddOutput(o);

					txns.push_back(tx);
				}

				if (!_sqlite->Finalize(stmt)) {
					Log::error("Coinbase get all finalize");
					return ;
				}

				PutTransactions("ela1", txns);
			}
		}

	}
}