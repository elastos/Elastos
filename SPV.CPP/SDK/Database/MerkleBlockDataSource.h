// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef __ELASTOS_SDK_MERKLEBLOCKDATASOURCE_H__
#define __ELASTOS_SDK_MERKLEBLOCKDATASOURCE_H__

#include "Sqlite.h"
#include "TableBase.h"

#include <boost/thread/mutex.hpp>
#include <boost/thread/shared_mutex.hpp>

namespace Elastos {
	namespace ElaWallet {

		class IMerkleBlock;
		typedef boost::shared_ptr<IMerkleBlock> MerkleBlockPtr;

		class MerkleBlockDataSource : public TableBase {

		public:
			MerkleBlockDataSource(Sqlite *sqlite);
			MerkleBlockDataSource(SqliteTransactionType type, Sqlite *sqlite);
			~MerkleBlockDataSource();

			bool PutMerkleBlock(const std::string &iso, const MerkleBlockPtr &blockPtr);
			bool PutMerkleBlocks(const std::string &iso, const std::vector<MerkleBlockPtr> &blocks);
			bool DeleteMerkleBlock(const std::string &iso, long id);
			bool DeleteAllBlocks(const std::string &iso);
			std::vector<MerkleBlockPtr> GetAllMerkleBlocks(const std::string &iso, const std::string &pluginType) const;

		private:
			bool PutMerkleBlockInternal(const std::string &iso, const MerkleBlockPtr &blockPtr);


		private:
			/*
			 * merkle block table
			 */
			const std::string MB_TABLE_NAME = "merkleBlockTable";
			const std::string MB_COLUMN_ID = "_id";
			const std::string MB_BUFF = "merkleBlockBuff";
			const std::string MB_HEIGHT = "merkleBlockHeight";
			const std::string MB_ISO = "merkleBlockISO";

			const std::string MB_DATABASE_CREATE = "create table if not exists " + MB_TABLE_NAME + " (" +
				MB_COLUMN_ID + " integer primary key autoincrement, " +
				MB_BUFF + " blob, " +
				MB_HEIGHT + " integer, " +
				MB_ISO + " text DEFAULT 'ELA');";
		};

	}
}


#endif //__ELASTOS_SDK_MERKLEBLOCKDATASOURCE_H__

