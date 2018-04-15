// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "PreCompiled.h"
#include <boost/filesystem.hpp>

#include "Sqlite.h"

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

		bool Sqlite::execSql(const std::string &sql, execCallBack callBack, void* arg) {
			char* errmsg;

			int r = sqlite3_exec(_dataBasePtr, "BEGIN IMMEDIATE;", NULL, NULL, NULL);
			if (r != SQLITE_OK) {
				return false;
			}

			r = sqlite3_exec(_dataBasePtr, sql.c_str(), callBack, arg, &errmsg);
			if (r != SQLITE_OK) {
				if (errmsg) {
					// TODO how to complain the error
					sqlite3_free(errmsg);
				}
				return false;
			}

			r = sqlite3_exec(_dataBasePtr, "COMMIT;", NULL, NULL, NULL);
			if (r != SQLITE_OK) {
				return false;
			}

			return true;
		}

		bool Sqlite::sqlitePrepare(const std::string &sql, sqlite3_stmt **ppStmt, const char **pzTail) {
			int r = 0;

			r = sqlite3_prepare_v2(_dataBasePtr, sql.c_str(), sql.length(), ppStmt, pzTail);
			if (r != SQLITE_OK) {
				return false;
			}

			return true;
		}

		bool Sqlite::sqliteStep(sqlite3_stmt *pStmt) {
			return SQLITE_OK == sqlite3_step(pStmt);
		}


		void Sqlite::close() {
			if (_dataBasePtr != NULL) {
				sqlite3_close_v2(_dataBasePtr);
				_dataBasePtr = NULL;
			}
		}

	}
}
