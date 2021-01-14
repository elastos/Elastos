/*
 * Copyright (c) 2020 Elastos Foundation
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

#ifndef __ELASTOS_SPVSDK_TXTABLE_H__
#define __ELASTOS_SPVSDK_TXTABLE_H__

#include "Sqlite.h"
#include "TableBase.h"

namespace Elastos {
	namespace ElaWallet {

		class TxTable;

		// will delete later
		class TxOldEntity {
		public:
			const std::string &GetTxHash() const;

			const bytes_t &GetBuf() const;

			uint32_t GetBlockHeight() const;

			time_t GetTimestamp() const;

			const std::string &GetISO() const;

		private:
			friend class TxTable;

			std::string _id;
			bytes_t _buf;
			uint32_t _blockHeight;
			time_t _timestamp;
			std::string _iso;
		};

		class TxEntity {
		public:
			TxEntity() :
				_height(0),
				_timestamp(0),
				_type(0),
				_version(0),
				_lockTime(0),
				_payloadVersion(0)
			{}

			void SetTxHash(const std::string &h);

			const std::string &GetTxHash() const;

			void SetHeight(uint32_t h);

			uint32_t GetHeight() const;

			void SetTimestamp(time_t t);

			time_t GetTimestamp() const;

			void SetType(uint8_t t);

			uint8_t GetType() const;

			void SetVersion(uint8_t v);

			uint8_t GetVersion() const;

			void SetLockTime(time_t t);

			time_t GetLockTime() const;

			void SetPayloadVersion(uint8_t v);

			uint8_t GetPayloadVersion() const;

			void SetPayload(const bytes_t &p);

			const bytes_t &GetPayload() const;

			void SetOutputs(const bytes_t &o);

			const bytes_t &GetOutputs() const;

			void SetAttributes(const bytes_t &a);

			const bytes_t &GetAttributes() const;

			void SetInputs(const bytes_t &in);

			const bytes_t &GetInputs() const;

			void SetPrograms(const bytes_t &p);

			const bytes_t &GetPrograms() const;

		private:
			friend class TxTable;

			std::string _txHash;
			uint32_t _height;
			time_t _timestamp;
			uint8_t _type;
			uint8_t _version;
			time_t _lockTime;
			uint8_t _payloadVersion;
			bytes_t _payload;
			bytes_t _outputs;
			bytes_t _attributes;
			bytes_t _inputs;
			bytes_t _programs;
		};

		class TxTable : public TableBase {
		public:
			TxTable(Sqlite *sqlite, SqliteTransactionType type = IMMEDIATE);

			~TxTable();

			const std::string &GetTableName() const;

			bool ContainTx(const std::string &hash) const;

			bool GetTx(std::vector<TxEntity> &entities, uint32_t height) const;

			bool GetTx(std::vector<TxEntity> &entities, const std::vector<uint8_t> &types) const;

			bool GetTx(std::vector<TxEntity> &entities) const;

			bool GetTx(std::vector<TxEntity> &entities, const std::set<std::string> &hashes) const;

			bool GetTx(std::vector<TxEntity> &entities, uint8_t type, bool invertMatch, size_t offset, size_t limit, bool desc) const;

			bool GetTx(std::vector<TxEntity> &entities, const std::string &utxoTable, const std::string &utxoColumn) const;

			size_t GetTxCnt(uint8_t type, bool invertMatch) const;

			size_t GetAllTxCnt() const;

			time_t GetEarliestTxTimestamp() const;

			bool PutTx(const std::vector<TxEntity> &entities);

			bool UpdateTx(const std::vector<std::string> &hashes, uint32_t height, time_t timestamp);

			bool DeleteTx(const std::string &hash);

			bool DeleteAll();

		private:
			bool CreateIndexTxHash();

			bool CreateIndexType();

			bool CreateIndexTimestamp();

			bool CreateIndexHeight();

			bool GetFullTxCommon(const std::string &sql, std::vector<TxEntity> &entities, const boost::function<bool(sqlite3_stmt *stmt)> &bindArgs) const;

		private:
			const std::string _indexTxHash = "indexTxHash";
			const std::string _indexType = "indexType";
			const std::string _indexTimestamp = "indexTimestamp";
			const std::string _indexHeight = "indexHeight";

			const std::string _tableName = "txTable";
			const std::string _txHash = "_txHash";
			const std::string _height = "height";
			const std::string _timestamp = "timestamp";
			const std::string _type = "type";
			const std::string _version = "version";
			const std::string _lockTime = "lockTime";
			const std::string _payloadVersion = "payloadVersion";
			const std::string _payload = "payload";
			const std::string _outputs = "outputs";
			const std::string _attributes = "attributes";
			const std::string _inputs = "inputs";
			const std::string _programs = "programs";
			const std::string _tableCreation = "CREATE TABLE IF NOT EXISTS " +
											   _tableName + "(" +
											   _txHash + " CHAR(64) NOT NULL," +
											   _height + " INTEGER," +
											   _timestamp + " INTEGER," +
											   _type + " INTEGER," +
											   _version + " INTEGER," +
											   _lockTime + " INTEGER," +
											   _payloadVersion + " INTEGER," +
											   _payload + " BLOB," +
											   _outputs + " BLOB," +
											   _attributes + " BLOB," +
											   _inputs + " BLOB," +
											   _programs + " BLOB);";

			// old table. will remove later
		public:
			bool GetAllOldTx(std::vector<TxOldEntity> &entities) const;

			bool RemoveOldTxTableInner();

		private:
			bool GetOldTxCommon(const std::string &sql, std::vector<TxOldEntity> &entities) const;

		private:
			const std::string _txHashProposalTable = "txHashProposalTable";
			const std::string _txHashDPoSTable = "txHashDPoSTable";
			const std::string _txHashDIDTable = "txHashDIDTable";
			const std::string _txHashCRCTable = "txHashCRCTable";
			const std::string _tableNameNormal = "transactionTable";
			const std::string _tableNamePending = "transactionPending";
			const std::string _tableNameCoinbase = "transactionCoinbase";
			const std::string _oldTxHash = "_id";
			const std::string _oldBuff = "transactionBuff";
			const std::string _oldBlockHeight = "transactionBlockHeight";
			const std::string _oldTimestamp = "transactionTimeStamp";
			const std::string _oldiso = "transactionISO";

		};

	}
}

#endif
