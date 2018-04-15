// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef __ELASTOS_SDK_SQLITE_H__
#define __ELASTOS_SDK_SQLITE_H__

#include <sqlite3.h>
#include <boost/filesystem.hpp>

namespace Elastos {
	namespace SDK {

		typedef int (*execCallBack)(void*,int,char**,char**);

		class Sqlite {
		public:
			Sqlite(const boost::filesystem::path path);
			~Sqlite();

			bool isValid();
			// Wrapper of sqlitePrepare(), sqliteStep(), sqlite...
			bool execSql(const std::string &sql, execCallBack callBack, void *arg);

			bool sqlitePrepare(const std::string &sql, sqlite3_stmt **ppStmt, const char **pzTail);
			bool sqliteStep(sqlite3_stmt *pStmt);

		private:
			bool open(const boost::filesystem::path &path);
			void close();

		private:
			sqlite3 *_dataBasePtr;
		};

	}
}


#endif //__ELASTOS_SDK_SQLITE_H__
