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

#ifndef __ELASTOS_SDK_SQLITE_H__
#define __ELASTOS_SDK_SQLITE_H__

#include <Common/typedefs.h>

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

/*
** The maximum value of a ?nnn wildcard that the parser will accept.
*/
#ifndef SQLITE_MAX_VARIABLE_NUMBER
# define SQLITE_MAX_VARIABLE_NUMBER 999
#endif

		class Sqlite {
		public:
			Sqlite(const boost::filesystem::path &path);
			~Sqlite();

			bool IsValid();

			int ExtendedEerrCode() const;
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

