// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef __ELASTOS_SDK_INTERNALADDRESSES_H__
#define __ELASTOS_SDK_INTERNALADDRESSES_H__

#include "TableBase.h"

namespace Elastos {
	namespace ElaWallet {

		class InternalAddresses : public TableBase {
		public:
			InternalAddresses(Sqlite *sqlite);

			InternalAddresses(SqliteTransactionType type, Sqlite *sqlite);

			~InternalAddresses();

			bool putAddress(uint32_t startIndex, const std::string &address);

			bool putAddresses(uint32_t startIndex, const std::vector<std::string> &addresses);

			bool clearAddresses();

			std::vector<std::string> getAddresses(uint32_t startIndex, uint32_t count);

			uint32_t getAvailableAddresses(uint32_t startIndex);

		private:
			/*
			 * Internal addresses table
			 */
			const std::string MB_TABLE_NAME = "internalAddresses";
			const std::string MB_COLUMN_ID = "_id";
			const std::string MB_ADDRESS = "address";

			const std::string MB_DATABASE_CREATE = "create table if not exists " + MB_TABLE_NAME + " (" +
												   MB_COLUMN_ID + " integer primary key autoincrement, " +
												   MB_ADDRESS + " text);";
		};

	}
}

#endif //__ELASTOS_SDK_INTERNALADDRESSES_H__
