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

#include "Sqlite.h"

#include <Common/Log.h>
#include <Common/typedefs.h>

#include <boost/filesystem.hpp>

namespace Elastos {
	namespace ElaWallet {

		Sqlite::Sqlite(const boost::filesystem::path &path) {
			open(path);
		}

		Sqlite::~Sqlite() {
			close();
		}

		int Sqlite::ExtendedEerrCode() const {
			return sqlite3_extended_errcode(_dataBasePtr);
		}

		bool Sqlite::IsValid() {
			return _dataBasePtr != NULL;
		}

		bool Sqlite::exec(const std::string &sql, ExecCallBack callBack, void *arg) {
			char *errmsg;

			if (!IsValid()) {
				Log::error("sqlite is invalid");
				return false;
			}

			int r = sqlite3_exec(_dataBasePtr, sql.c_str(), callBack, arg, &errmsg);
			if (r != SQLITE_OK) {
				if (errmsg) {
					Log::error("sqlite exec \"{}\" error: {}", sql, errmsg);
					sqlite3_free(errmsg);
				}
				return false;
			}

			return true;
		}

		bool Sqlite::BeginTransaction(SqliteTransactionType type) {
			_lockMutex.lock();
			return exec("BEGIN " + GetTxTypeString(type) + " TRANSACTION;", nullptr, nullptr);
		}

		bool Sqlite::EndTransaction() {
			bool result = exec("COMMIT;", nullptr, nullptr);
			_lockMutex.unlock();
			return result;
		}

		bool Sqlite::Prepare(const std::string &sql, sqlite3_stmt **ppStmt, const char **pzTail) {
			int r = 0;

			if (!IsValid()) {
				Log::error("sqlite is invalid");
				return false;
			}

			r = sqlite3_prepare_v2(_dataBasePtr, sql.c_str(), sql.length(), ppStmt, pzTail);
			if (r != SQLITE_OK) {
				Log::error("sqlite prepare error");
				if (ppStmt && *ppStmt) {
					sqlite3_finalize(*ppStmt);
				}
				return false;
			}

			return true;
		}

		int Sqlite::Step(sqlite3_stmt *pStmt) {
			return sqlite3_step(pStmt);
		}

		bool Sqlite::Finalize(sqlite3_stmt *pStmt) {
			return IsValid() && SQLITE_OK == sqlite3_finalize(pStmt);
		}

		bool Sqlite::BindBlob(sqlite3_stmt *pStmt, int idx, const void *blob, size_t size, BindCallBack callBack) {
			return IsValid() && SQLITE_OK == sqlite3_bind_blob(pStmt, idx, blob, size, callBack);
		}

		bool Sqlite::BindBlob(sqlite3_stmt *pStmt, int idx, const bytes_t &blob, BindCallBack callBack) {
			return IsValid() && SQLITE_OK == sqlite3_bind_blob(pStmt, idx, &blob[0], blob.size(), callBack);
		}

		bool Sqlite::BindDouble(sqlite3_stmt *pStmt, int idx, double d) {
			return IsValid() && SQLITE_OK == sqlite3_bind_double(pStmt, idx, d);
		}

		bool Sqlite::BindInt(sqlite3_stmt *pStmt, int idx, int i) {
			return IsValid() && SQLITE_OK == sqlite3_bind_int(pStmt, idx, i);
		}

		bool Sqlite::BindInt64(sqlite3_stmt *pStmt, int idx, int64_t i) {
			return IsValid() && SQLITE_OK == sqlite3_bind_int64(pStmt, idx, i);
		}

		bool Sqlite::BindNull(sqlite3_stmt *pStmt, int idx) {
			return IsValid() && SQLITE_OK == sqlite3_bind_null(pStmt, idx);
		}

		bool Sqlite::BindText(sqlite3_stmt *pStmt, int idx, const std::string &text, BindCallBack callBack) {
			return IsValid() && SQLITE_OK == sqlite3_bind_text(pStmt, idx, text.c_str(), text.length(), callBack);
		}

		void Sqlite::flush() {
			if (SQLITE_OK != sqlite3_db_cacheflush(_dataBasePtr)) {
				Log::error("sqlite flush to disk error");
			}
		}

		bytes_ptr Sqlite::ColumnBlobBytes(sqlite3_stmt *pStmt, int iCol) {
			uint8_t *data = (uint8_t *)ColumnBlob(pStmt, iCol);
			size_t len = (size_t) ColumnBytes(pStmt, iCol);

			if (len > 0) {
				bytes_ptr blob(new bytes_t());
				blob->assign(data, data + len);
				return blob;
			}

			return nullptr;
		}

		const void *Sqlite::ColumnBlob(sqlite3_stmt *pStmt, int iCol) {
			return sqlite3_column_blob(pStmt, iCol);
		}

		double Sqlite::ColumnDouble(sqlite3_stmt *pStmt, int iCol) {
			return sqlite3_column_double(pStmt, iCol);
		}

		int Sqlite::ColumnInt(sqlite3_stmt *pStmt, int iCol) {
			return sqlite3_column_int(pStmt, iCol);
		}

		int64_t Sqlite::ColumnInt64(sqlite3_stmt *pStmt, int iCol) {
			return sqlite3_column_int64(pStmt, iCol);
		}

		std::string Sqlite::ColumnText(sqlite3_stmt *pStmt, int iCol) {
			return std::string((char *)sqlite3_column_text(pStmt, iCol));
		}

		int Sqlite::ColumnBytes(sqlite3_stmt *pStmt, int iCol) {
			return sqlite3_column_bytes(pStmt, iCol);
		}

		std::string Sqlite::GetTxTypeString(SqliteTransactionType type) {
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

			boost::filesystem::path parentPath = path.parent_path();
			if (!parentPath.empty() && !boost::filesystem::exists(parentPath)) {
				if (!boost::filesystem::create_directories(parentPath)) {
					Log::error("create directory \"{}\" error", parentPath.string());
					return false;
				}
			}

//			path.imbue(boost::locale::generator().generate("UTF-8"));
			int r = sqlite3_open_v2(path.string().c_str(), &_dataBasePtr, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE | SQLITE_OPEN_FULLMUTEX, NULL);
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

