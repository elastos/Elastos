// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef __ELASTOS_SDK_MERKLEBLOCKDATASOURCE_H__
#define __ELASTOS_SDK_MERKLEBLOCKDATASOURCE_H__

#include "BRInt.h"
#include "Sqlite.h"
#include "ByteData.h"

namespace Elastos {
	namespace SDK {

		typedef struct {
			ByteData blockBytes;
			uint32_t blockHeight;
		} MerkleBlockEntity;

		class MerkleBlockDataSource {

		public:
			MerkleBlockDataSource(Sqlite *sqlite);
			~MerkleBlockDataSource();

			bool putMerkleBlocks(const std::string &iso, const std::vector<MerkleBlockEntity> &blockEntities);
			bool deleteMerkleBlock(const std::string &iso, const MerkleBlockEntity &blockEntity);
			bool deleteAllBlocks(const std::string &iso);
			std::vector<MerkleBlockEntity> getAllMerkleBlocks(const std::string &iso) const;

		private:
			Sqlite *_sqlite;

			const std::string MB_TABLE_NAME_OLD = "merkleBlockTable";
			const std::string MB_TABLE_NAME = "merkleBlockTable_v2";
			const std::string MB_COLUMN_ID = "_id";
			const std::string MB_BUFF = "merkleBlockBuff";
			const std::string MB_HEIGHT = "merkleBlockHeight";
			const std::string MB_ISO = "merkleBlockIso";

			const std::string MB_DATABASE_CREATE = "create table if not exists " + MB_TABLE_NAME + " (" +
				MB_COLUMN_ID + " integer primary key autoincrement, " +
				MB_BUFF + " blob, " +
				MB_ISO + " text DEFAULT 'BTC' , " +
				MB_HEIGHT + " integer);";
		};

	}
}


#endif //__ELASTOS_SDK_MERKLEBLOCKDATASOURCE_H__
