// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef __ELASTOS_SDK_EXTERNALADDRESSES_H__
#define __ELASTOS_SDK_EXTERNALADDRESSES_H__

#include "TableBase.h"

namespace Elastos {
	namespace ElaWallet {

		class ExternalAddresses : public TableBase {
		public:
			ExternalAddresses(Sqlite *sqlite);

			ExternalAddresses(SqliteTransactionType type, Sqlite *sqlite);

			~ExternalAddresses();

			bool putAddress(uint32_t startIndex, const std::string &address);

			bool putAddresses(uint32_t startIndex, const std::vector<std::string> &addresses);

			bool clearAddresses();

			std::vector<std::string> getAddresses(uint32_t startIndex, uint32_t count) const;

			uint32_t getAvailableAddresses(uint32_t startIndex) const;

		private:
			bool putAddressInternal(uint32_t startIndex, const std::string &address);

		private:
			/*
			 * External addresses table
			 */
			const std::string EA_TABLE_NAME = "externalAddresses";
			const std::string EA_COLUMN_ID = "_id";
			const std::string EA_ADDRESS = "address";

			const std::string MB_DATABASE_CREATE = "create table if not exists " + EA_TABLE_NAME + " (" +
				EA_COLUMN_ID + " integer primary key, " + EA_ADDRESS + " text);";
		};

	}
}


#endif //__ELASTOS_SDK_EXTERNALADDRESSES_H__
