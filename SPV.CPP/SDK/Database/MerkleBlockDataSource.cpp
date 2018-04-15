// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "MerkleBlockDataSource.h"

namespace Elastos {
	namespace SDK {

		MerkleBlockDataSource::MerkleBlockDataSource(Sqlite *sqlite) :
			_sqlite(sqlite) {
			std::string sqlUpper = MB_DATABASE_CREATE;
			std::transform(sqlUpper.begin(), sqlUpper.end(), sqlUpper.begin(), ::toupper);
			_sqlite->execSql(sqlUpper, nullptr, nullptr);
		}

		MerkleBlockDataSource::~MerkleBlockDataSource() {
		}

		bool MerkleBlockDataSource::putMerkleBlocks(const std::string &iso, const std::vector<MerkleBlockEntity> &blockEntities) {

			return true;
		}

		bool MerkleBlockDataSource::deleteMerkleBlock(const std::string &iso, const MerkleBlockEntity &blockEntity) {
			return true;
		}

		bool MerkleBlockDataSource::deleteAllBlocks(const std::string &iso) {
			return true;
		}

		std::vector<MerkleBlockEntity> MerkleBlockDataSource::getAllMerkleBlocks(const std::string &iso) const {
			std::vector<MerkleBlockEntity> merkleBlocks;

			return merkleBlocks;
		}

	}
}
