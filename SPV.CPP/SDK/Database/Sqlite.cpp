// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "PreCompiled.h"
#include <boost/filesystem.hpp>

#include "Sqlite.h"
#include "Log.h"

namespace Elastos {
	namespace SDK {

		Sqlite::Sqlite(const boost::filesystem::path path) {
			open(path);
		}

		Sqlite::~Sqlite() {
			close();
		}

		bool Sqlite::isValid() {
			return _dataBasePtr != NULL;
		}

		bool Sqlite::exec(const std::string &sql, ExecCallBack callBack, void *arg) {
			char *errmsg;

			if (!isValid()) {
				Log::error("sqlite is invalid");
				return false;
			}

			SPDLOG_DEBUG(Log::getLogger(), "sqlite exec \"{}\"", sql);

			int r = sqlite3_exec(_dataBasePtr, sql.c_str(), callBack, arg, &errmsg);
			if (r != SQLITE_OK) {
				if (errmsg) {
					Log::getLogger()->error("sqlite exec \"{}\" error: {}", sql, errmsg);
					sqlite3_free(errmsg);
				}
				return false;
			}

			return true;
		}

		bool Sqlite::beginTransaction(SqliteTransactionType type) {
			return exec("BEGIN " + getTxTypeString(type) + ";", nullptr, nullptr);
		}

		bool Sqlite::endTransaction() {
			return exec("COMMIT;", nullptr, nullptr);
		}

		bool Sqlite::transaction(SqliteTransactionType type, const std::string &sql, ExecCallBack callBack, void *arg) {
			char* errmsg;
			std::string typeStr;

			typeStr = "BEGIN " + getTxTypeString(type) + ";";

			if (true != exec(typeStr.c_str(), NULL, NULL)) {
				return false;
			}

			if (true != exec(sql.c_str(), callBack, arg)) {
				return false;
			}

			if (true != exec("COMMIT;", NULL, NULL)) {
				return false;
			}

			return true;
		}

		bool Sqlite::prepare(const std::string &sql, sqlite3_stmt **ppStmt, const char **pzTail) {
			int r = 0;

			if (!isValid()) {
				Log::error("sqlite is invalid");
				return false;
			}

			r = sqlite3_prepare_v2(_dataBasePtr, sql.c_str(), sql.length(), ppStmt, pzTail);
			if (r != SQLITE_OK) {
				Log::error("sqlite prepare error");
				return false;
			}

			return true;
		}

		int Sqlite::step(sqlite3_stmt *pStmt) {
			return sqlite3_step(pStmt);
		}

		bool Sqlite::finalize(sqlite3_stmt *pStmt) {
			return isValid() && SQLITE_OK == sqlite3_finalize(pStmt);
		}

		bool Sqlite::bindBlob(sqlite3_stmt *pStmt, int idx, ByteData blob, BindCallBack callBack) {
			return isValid() && SQLITE_OK == sqlite3_bind_blob(pStmt, idx, blob.data, blob.length, callBack);
		}

		bool Sqlite::bindDouble(sqlite3_stmt *pStmt, int idx, double d) {
			return isValid() && SQLITE_OK == sqlite3_bind_double(pStmt, idx, d);
		}

		bool Sqlite::bindInt(sqlite3_stmt *pStmt, int idx, int i) {
			return isValid() && SQLITE_OK == sqlite3_bind_int(pStmt, idx, i);
		}

		bool Sqlite::bindInt64(sqlite3_stmt *pStmt, int idx, int64_t i) {
			return isValid() && SQLITE_OK == sqlite3_bind_int64(pStmt, idx, i);
		}

		bool Sqlite::bindNull(sqlite3_stmt *pStmt, int idx) {
			return isValid() && SQLITE_OK == sqlite3_bind_null(pStmt, idx);
		}

		bool Sqlite::bindText(sqlite3_stmt *pStmt, int idx, const std::string &text, BindCallBack callBack) {
			return isValid() && SQLITE_OK == sqlite3_bind_text(pStmt, idx, text.c_str(), text.length(), callBack);
		}

		const void *Sqlite::columnBlob(sqlite3_stmt *pStmt, int iCol) {
			return sqlite3_column_blob(pStmt, iCol);
		}

		double Sqlite::columnDouble(sqlite3_stmt *pStmt, int iCol) {
			return sqlite3_column_double(pStmt, iCol);
		}

		int Sqlite::columnInt(sqlite3_stmt *pStmt, int iCol) {
			return sqlite3_column_int(pStmt, iCol);
		}

		int64_t Sqlite::columnInt64(sqlite3_stmt *pStmt, int iCol) {
			return sqlite3_column_int64(pStmt, iCol);
		}

		std::string Sqlite::columnText(sqlite3_stmt *pStmt, int iCol) {
			return std::string((char *)sqlite3_column_text(pStmt, iCol));
		}

		int Sqlite::columnBytes(sqlite3_stmt *pStmt, int iCol) {
			return sqlite3_column_bytes(pStmt, iCol);
		}

		std::string Sqlite::getTxTypeString(SqliteTransactionType type) {
			if (type == DEFERRED) {
				return "DEFERRED";
			} else if (type == IMMEDIATE) {
				return "IMMEDIATE";
			} else if (type == EXCLUSIVE) {
				return "EXCLUSIVE";
			}

			return "IMMEDIATE";
		}

		bool Sqlite::open(const boost::filesystem::path &path) {
			// If the SQLITE_OPEN_NOMUTEX flag is set, then the database connection opens in the multi-thread
			// threading mode as long as the single-thread mode has not been set at compile-time or start-time.
			// If the SQLITE_OPEN_FULLMUTEX flag is set then the database connection opens in the serialized
			// threading mode unless single-thread was previously selected at compile-time or start-time.
			int r = sqlite3_open_v2(path.c_str(), &_dataBasePtr, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE | SQLITE_OPEN_FULLMUTEX, NULL);
			if (r != SQLITE_OK) {
				close();
				return false;
			}

			return true;
		}

		void Sqlite::close() {
			if (_dataBasePtr != NULL) {
				sqlite3_close_v2(_dataBasePtr);
				_dataBasePtr = NULL;
			}
		}

	}
}

