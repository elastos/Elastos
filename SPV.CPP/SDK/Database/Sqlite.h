// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef __ELASTOS_SDK_SQLITE_H__
#define __ELASTOS_SDK_SQLITE_H__

#include <SDK/Common/typedefs.h>

#include <sqlite3.h>
#include <boost/filesystem.hpp>
#include <boost/thread/mutex.hpp>

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

			bool IsValid();
			/*
			 * The sqlite3_exec() interface is a convenience wrapper around sqlite3_prepare_v2(),
			 * sqlite3_step(), and sqlite3_finalize(), that allows an application to run multiple
			 * statements of SQL without having to use a lot of C code.
			 */
			bool exec(const std::string &sql, ExecCallBack callBack, void *arg);

			bool BeginTransaction(SqliteTransactionType type);
			bool EndTransaction();

			bool Prepare(const std::string &sql, sqlite3_stmt **ppStmt, const char **pzTail);
			int Step(sqlite3_stmt *pStmt);
			bool Finalize(sqlite3_stmt *pStmt);
			bool BindBlob(sqlite3_stmt *pStmt, int idx, const bytes_t &blob, BindCallBack callBack);
			bool BindBlob(sqlite3_stmt *pStmt, int idx, const void *blob, size_t size, BindCallBack callBack);
			bool BindDouble(sqlite3_stmt *pStmt, int idx, double d);
			bool BindInt(sqlite3_stmt *pStmt, int idx, int i);
			bool BindInt64(sqlite3_stmt *pStmt, int idx, int64_t i);
			bool BindNull(sqlite3_stmt *pStmt, int idx);
			bool BindText(sqlite3_stmt *pStmt, int idx, const std::string &text, BindCallBack callBack);

			void flush();

			bytes_ptr ColumnBlobBytes(sqlite3_stmt *pStmt, int iCol);
			const void *ColumnBlob(sqlite3_stmt *pStmt, int iCol);
			double ColumnDouble(sqlite3_stmt *pStmt, int iCol);
			int ColumnInt(sqlite3_stmt *pStmt, int iCol);
			int64_t ColumnInt64(sqlite3_stmt *pStmt, int iCol);
			std::string ColumnText(sqlite3_stmt *pStmt, int iCol);
			int ColumnBytes(sqlite3_stmt *pStmt, int iCol);

		private:
			std::string GetTxTypeString(SqliteTransactionType type);
			bool open(const boost::filesystem::path &path);
			void close();

		private:
			sqlite3 *_dataBasePtr;
			mutable boost::mutex _lockMutex;
		};

	}
}


#endif //__ELASTOS_SDK_SQLITE_H__

