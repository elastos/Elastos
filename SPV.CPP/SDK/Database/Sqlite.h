// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef __ELASTOS_SDK_SQLITE_H__
#define __ELASTOS_SDK_SQLITE_H__

#include <sqlite3.h>
#include <boost/filesystem.hpp>

#include "CMemBlock.h"

namespace Elastos {
	namespace ElaWallet {

		typedef int (*ExecCallBack)(void*,int,char**,char**);
		typedef void (*BindCallBack)(void*);

		typedef enum {
			DEFERRED,
			IMMEDIATE,
			EXCLUSIVE
		} SqliteTransactionType;

		class Sqlite {
		public:
			Sqlite(const boost::filesystem::path &path);
			~Sqlite();

			bool isValid();
			/*
			 * The sqlite3_exec() interface is a convenience wrapper around sqlite3_prepare_v2(),
			 * sqlite3_step(), and sqlite3_finalize(), that allows an application to run multiple
			 * statements of SQL without having to use a lot of C code.
			 */
			bool exec(const std::string &sql, ExecCallBack callBack, void *arg);

			bool beginTransaction(SqliteTransactionType type);
			bool endTransaction();
			/*
			 * Transactions created using BEGIN...COMMIT do not nest. For nested transactions,
			 * use the SAVEPOINT and RELEASE commands.
			 * Transactions can be deferred, immediate, or exclusive.The default transaction
			 * behavior is deferred. Deferred means that no locks are acquired on the database
			 * until the database is first accessed.
			 * After a BEGIN IMMEDIATE, no other database connection will be able to write to
			 * the database or do a BEGIN IMMEDIATE or BEGIN EXCLUSIVE.
			 */
//			bool transaction(SqliteTransactionType type, const std::string &sql, ExecCallBack callBack, void *arg);

			bool prepare(const std::string &sql, sqlite3_stmt **ppStmt, const char **pzTail);
			int step(sqlite3_stmt *pStmt);
			bool finalize(sqlite3_stmt *pStmt);
			bool bindBlob(sqlite3_stmt *pStmt, int idx, CMBlock blob, BindCallBack callBack);
			bool bindDouble(sqlite3_stmt *pStmt, int idx, double d);
			bool bindInt(sqlite3_stmt *pStmt, int idx, int i);
			bool bindInt64(sqlite3_stmt *pStmt, int idx, int64_t i);
			bool bindNull(sqlite3_stmt *pStmt, int idx);
			bool bindText(sqlite3_stmt *pStmt, int idx, const std::string &text, BindCallBack callBack);

			const void *columnBlob(sqlite3_stmt *pStmt, int iCol);
			double columnDouble(sqlite3_stmt *pStmt, int iCol);
			int columnInt(sqlite3_stmt *pStmt, int iCol);
			int64_t columnInt64(sqlite3_stmt *pStmt, int iCol);
			std::string columnText(sqlite3_stmt *pStmt, int iCol);
			int columnBytes(sqlite3_stmt *pStmt, int iCol);

		private:
			std::string getTxTypeString(SqliteTransactionType type);
			bool open(const boost::filesystem::path &path);
			void close();

		private:
			sqlite3 *_dataBasePtr;
		};

	}
}


#endif //__ELASTOS_SDK_SQLITE_H__

