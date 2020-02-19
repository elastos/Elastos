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

#include "TransactionCoinbase.h"

#include <Common/Log.h>
#include <WalletCore/Address.h>
#include <Plugin/Transaction/TransactionOutput.h>
#include <Plugin/Transaction/Transaction.h>
#include <Plugin/Transaction/Payload/CoinBase.h>
#include <cstdint>

namespace Elastos {
	namespace ElaWallet {

		TransactionCoinbase::TransactionCoinbase(Sqlite *sqlite, SqliteTransactionType type) :
			TransactionDataStore(sqlite, type) {
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
		}

		TransactionCoinbase::~TransactionCoinbase() {
		}

		void TransactionCoinbase::InitializeTable() {
			TransactionDataStore::InitializeTable();
			ConvertFromOldData();
			TableBase::InitializeTable("drop table if exists " + _tableNameOld + ";");
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

				PutTransactions(txns);
			}
		}

	}
}