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

		class TransactionNormal : public TableBase {
		public:
			TransactionNormal(Sqlite *sqlite, SqliteTransactionType type = IMMEDIATE);

			~TransactionNormal();

			virtual void InitializeTable();

			bool Put(const TransactionPtr &tx);

			bool Puts(const std::vector<TransactionPtr> &txns);

			bool DeleteAll();

			size_t GetAllCount() const;

			time_t GetEarliestTxnTimestamp() const;

			std::vector<TransactionPtr> GetUniqueTxns(const std::string &chainID,
													  const std::set<std::string> &uniqueHash) const;

			TransactionPtr Get(const uint256 &hash, const std::string &chainID) const;

			std::vector<TransactionPtr> GetAfter(const std::string &chainID, uint32_t height) const;

			virtual std::vector<TransactionPtr> GetAll(const std::string &chainID) const;

			std::vector<TransactionPtr> Gets(const std::string &chainID, size_t offset, size_t limit, bool asc = false) const;

			std::vector<TransactionPtr> GetTxnBaseOnHash(const std::string &chainID,
														 const std::string &tableName,
														 const std::string &txHashColumnName) const;

			bool Update(const std::vector<TransactionPtr> &txns);

			bool DeleteByHash(const uint256 &hash);

			bool DeleteByHashes(const std::vector<uint256> &hashes);

			bool ContainHash(const uint256 &hash) const;

			// not in the transaction
		public:
			bool _Update(const TransactionPtr &txn);

			bool _DeleteByHashes(const std::vector<uint256> &hashes);

			bool _DeleteByHash(const uint256 &hash);

			bool _Puts(const std::vector<TransactionPtr> &txns, bool replace);

			bool _Put(const TransactionPtr &tx);
		private:
			TransactionPtr SelectByHash(const uint256 &hash, const std::string &chainID) const;

			void GetSelectedTxns(std::vector<TransactionPtr> &txns, const std::string &chainID, sqlite3_stmt *stmt) const;

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
