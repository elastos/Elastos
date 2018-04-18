// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "PreCompiled.h"
#include <sstream>

#include "MerkleBlockDataSource.h"

namespace Elastos {
	namespace SDK {

		MerkleBlockDataSource::MerkleBlockDataSource(Sqlite *sqlite) :
			_sqlite(sqlite),
			_txType(IMMEDIATE) {
			_sqlite->transaction(_txType, MB_DATABASE_CREATE, nullptr, nullptr);
		}

		MerkleBlockDataSource::MerkleBlockDataSource(SqliteTransactionType type, Sqlite *sqlite) :
			_sqlite(sqlite),
			_txType(type) {
			_sqlite->transaction(_txType, MB_DATABASE_CREATE, nullptr, nullptr);
		}

		MerkleBlockDataSource::~MerkleBlockDataSource() {
		}

		bool MerkleBlockDataSource::putMerkleBlock(const std::string &iso, const MerkleBlockEntity &blockEntity) {
			std::stringstream ss;

			ss << "INSERT INTO " << MB_TABLE_NAME << " (" <<
				MB_BUFF   << "," <<
				MB_HEIGHT << "," <<
				MB_ISO    <<
				") VALUES (?, ?, ?);";

			_sqlite->beginTransaction(_txType);

			sqlite3_stmt *stmt;
			if (true != _sqlite->prepare(ss.str(), &stmt, nullptr)) {
				return false;
			}

			_sqlite->bindBlob(stmt, 1, blockEntity.blockBytes, nullptr);
			_sqlite->bindInt(stmt, 2, blockEntity.blockHeight);
			_sqlite->bindText(stmt, 3, iso, nullptr);

			_sqlite->step(stmt);
			_sqlite->finalize(stmt);

			return _sqlite->endTransaction();
		}

		bool MerkleBlockDataSource::putMerkleBlocks(const std::string &iso, const std::vector<MerkleBlockEntity> &blockEntities) {
			for (int i = 0; i < blockEntities.size(); ++i) {
				if (true != putMerkleBlock(iso, blockEntities[i])) {
					return false;
				}
			}

			return true;
		}

		bool MerkleBlockDataSource::deleteMerkleBlock(const std::string &iso, const MerkleBlockEntity &blockEntity) {
			std::stringstream ss;

			ss << "DELETE FROM " << MB_TABLE_NAME <<
				" WHERE " << MB_COLUMN_ID << " = " << blockEntity.id <<
				" AND " << MB_ISO << " = " << "'" << iso << "';";

			return _sqlite->transaction(_txType, ss.str(), nullptr, nullptr);
		}

		bool MerkleBlockDataSource::deleteAllBlocks(const std::string &iso) {
			std::stringstream ss;

			ss << "DELETE FROM " << MB_TABLE_NAME <<
				" WHERE " << MB_ISO << " = '" << iso << "';";

			return _sqlite->transaction(_txType, ss.str(), nullptr, nullptr);
		}

		std::vector<MerkleBlockEntity> MerkleBlockDataSource::getAllMerkleBlocks(const std::string &iso) const {
			MerkleBlockEntity merkleBlock;
			std::vector<MerkleBlockEntity> merkleBlocks;
			std::stringstream ss;

			ss << "SELECT "  <<
				MB_COLUMN_ID << ", " <<
				MB_BUFF      << ", " <<
				MB_HEIGHT    <<
				" FROM "     << MB_TABLE_NAME <<
				" WHERE "    << MB_ISO << " = '" << iso << "';";

			_sqlite->beginTransaction(_txType);

			sqlite3_stmt *stmt;
			if (true != _sqlite->prepare(ss.str(), &stmt, nullptr)) {
				return merkleBlocks;
			}

			while (SQLITE_ROW == _sqlite->step(stmt)) {
				// id
				merkleBlock.id = _sqlite->columnInt(stmt, 0);

				// blockBytes
				const uint8_t *pblob = (const uint8_t *)_sqlite->columnBlob(stmt, 1);
				size_t len = _sqlite->columnBytes(stmt, 1);
				merkleBlock.blockBytes.data = new uint8_t[len];
				merkleBlock.blockBytes.length = len;
				memcpy(merkleBlock.blockBytes.data, pblob, len);

				// blockHeight
				merkleBlock.blockHeight = _sqlite->columnInt(stmt, 2);

				merkleBlocks.push_back(merkleBlock);
			}

			_sqlite->finalize(stmt);
			_sqlite->endTransaction();

			return merkleBlocks;
		}

	}
}

