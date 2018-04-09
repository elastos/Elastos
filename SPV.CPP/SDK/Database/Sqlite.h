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
			bool open(const boost::filesystem::path path);
			bool execSql(std::string sql, execCallBack callBack, void* arg);
			void close();

		private:
			sqlite3* _dataBasePtr;
		};

	}
}


#endif //__ELASTOS_SDK_SQLITE_H__
