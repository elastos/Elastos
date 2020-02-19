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
			TransactionDataStore(Sqlite *sqlite, SqliteTransactionType type = IMMEDIATE);

			~TransactionDataStore();

			virtual void InitializeTable();

			bool PutTransaction(const TransactionPtr &tx);

			bool PutTransactions(const std::vector<TransactionPtr> &txns);

			bool DeleteAllTransactions();

			size_t GetAllTransactionsCount() const;

			TransactionPtr GetTransaction(const uint256 &hash, const std::string &chainID);

			std::vector<TransactionPtr> GetAllConfirmedTxns(const std::string &chainID) const;

			bool UpdateTransaction(const std::vector<uint256> &hashes, uint32_t blockHeight, time_t timestamp);

			bool DeleteTxByHash(const uint256 &hash);

			bool DeleteTxByHashes(const std::vector<uint256> &hashes);

		private:
			TransactionPtr SelectTxByHash(const std::string &hash, const std::string &chainID) const;

			bool ContainHash(const std::string &hash) const;

			bool PutTransactionInternal(const TransactionPtr &tx);

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
