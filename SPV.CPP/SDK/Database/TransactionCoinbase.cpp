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
#include <Plugin/Registry.h>
#include <cstdint>
#include <Plugin/Transaction/IDTransaction.h>

namespace Elastos {
	namespace ElaWallet {

		TransactionCoinbase::TransactionCoinbase(Sqlite *sqlite, SqliteTransactionType type) :
			TransactionNormal(sqlite, type) {
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
			if (!ContainOldData()) {
				TransactionNormal::InitializeTable();
			}
		}

		std::vector<TransactionPtr> TransactionCoinbase::GetAll(const std::string &chainID) const {
			if (!ContainOldData()) {
				return TransactionNormal::GetAll(chainID);
			}
			return GetAllOld(chainID);
		}

		void TransactionCoinbase::RemoveOld() {
			if (ContainOldData()) {
				TransactionNormal::InitializeTable();
				TableBase::InitializeTable("drop table if exists " + _tableNameOld + ";");
			}
		}

		std::vector<TransactionPtr> TransactionCoinbase::GetAllOld(const std::string &chainID) const {
			std::string sql;
			std::vector<TransactionPtr> txns;

			sql = "SELECT " + _txHashOld + ", " + _blockHeightOld + ", " + _timestampOld + ", " + _indexOld + ", " +
				  _programHashOld + ", " + _assetIDOld + ", " + _outputLockOld + ", " + _amountOld + ", " +
				  _payloadOld + ", " + _spentOld + " FROM " + _tableNameOld + ";";

			sqlite3_stmt *stmt;
			if (!_sqlite->Prepare(sql, &stmt, nullptr)) {
				Log::error("prepare sql: {}", sql);
				return {};
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

				TransactionPtr tx;
				if (chainID == CHAINID_MAINCHAIN) {
					tx = TransactionPtr(new Transaction(Transaction::coinBase, PayloadPtr(new CoinBase())));
				} else if (chainID == CHAINID_IDCHAIN || chainID == CHAINID_TOKENCHAIN) {
					tx = TransactionPtr(new IDTransaction(Transaction::coinBase, PayloadPtr(new CoinBase())));
				}

				tx->SetHash(txHash);
				tx->SetBlockHeight(blockHeight);
				tx->SetTimestamp(timestamp);

				tx->AddOutput(o);

				txns.push_back(tx);
			}

			if (!_sqlite->Finalize(stmt)) {
				Log::error("Coinbase get all finalize");
				return {};
			}

			return txns;
		}

		bool TransactionCoinbase::ContainOldData() const {
			return TableBase::ContainTable(_tableNameOld);
		}

		void TransactionCoinbase::ConvertFromOldData() {
			if (ContainOldData()) {
				std::vector<TransactionPtr> txns = GetAllOld(CHAINID_MAINCHAIN);
				Puts(txns);
			}
		}

	}
}