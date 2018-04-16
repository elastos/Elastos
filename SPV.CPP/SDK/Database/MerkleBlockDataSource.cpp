// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "PreCompiled.h"
#include <sstream>

#include "MerkleBlockDataSource.h"

namespace Elastos {
	namespace SDK {

		MerkleBlockDataSource::MerkleBlockDataSource(Sqlite *sqlite) :
			_sqlite(sqlite) {
			_sqlite->transaction(IMMEDIATE, MB_DATABASE_CREATE, nullptr, nullptr);
		}

		MerkleBlockDataSource::~MerkleBlockDataSource() {
		}

		bool MerkleBlockDataSource::putMerkleBlock(const std::string &iso, const MerkleBlockEntity &blockEntity) {
			std::stringstream ss;

			ss << "insert into " << MB_TABLE_NAME << " (" <<
				MB_BUFF   << "," <<
				MB_HEIGHT << "," <<
				MB_ISO    <<
				") values(?, ?, ?);";

			_sqlite->exec("BEGIN IMMEDIATE;", nullptr, nullptr);

			sqlite3_stmt *stmt;
			if (true != _sqlite->prepare(ss.str(), &stmt, nullptr)) {
				return false;
			}

			_sqlite->bindBlob(stmt, 1, blockEntity.blockBytes, nullptr);
			_sqlite->bindInt(stmt, 2, blockEntity.blockHeight);
			_sqlite->bindText(stmt, 3, iso, nullptr);

			_sqlite->finalize(stmt);

			return _sqlite->exec("COMMIT;", nullptr, nullptr);
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

			ss << "delete from " << MB_TABLE_NAME <<
				" where " << MB_COLUMN_ID << " = " << blockEntity.id <<
				" and " << MB_ISO << " = " << "'" << iso << "';";

			return _sqlite->transaction(IMMEDIATE, ss.str(), nullptr, nullptr);
		}

		bool MerkleBlockDataSource::deleteAllBlocks(const std::string &iso) {
			std::stringstream ss;

			ss << "delete from " << MB_TABLE_NAME <<
				" where " << MB_ISO << " = '" << iso << "';";

			return _sqlite->transaction(IMMEDIATE, ss.str(), nullptr, nullptr);
		}

		std::vector<MerkleBlockEntity> MerkleBlockDataSource::getAllMerkleBlocks(const std::string &iso) const {
			MerkleBlockEntity merkleBlock;
			std::vector<MerkleBlockEntity> merkleBlocks;
			std::stringstream ss;

			ss << "select "  <<
				MB_COLUMN_ID << ", " <<
				MB_BUFF      << ", " <<
				MB_HEIGHT    <<
				" from "     << MB_TABLE_NAME <<
				" where "    << MB_ISO << " = '" << iso << "';";

			_sqlite->exec("BEGIN IMMEDIATE;", nullptr, nullptr);

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
			_sqlite->exec("COMMIT;", nullptr, nullptr);

			return merkleBlocks;
		}

	}
}

