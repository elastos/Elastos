// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "MerkleBlockDataSource.h"

#include <SDK/Common/Utils.h>
#include <SDK/Common/ParamChecker.h>

#include <sstream>

namespace Elastos {
	namespace ElaWallet {

		MerkleBlockDataSource::MerkleBlockDataSource(Sqlite *sqlite) :
			TableBase(sqlite) {
			initializeTable(MB_DATABASE_CREATE);
		}

		MerkleBlockDataSource::MerkleBlockDataSource(SqliteTransactionType type, Sqlite *sqlite) :
			TableBase(type, sqlite) {
			initializeTable(MB_DATABASE_CREATE);
		}

		MerkleBlockDataSource::~MerkleBlockDataSource() {
		}

		bool MerkleBlockDataSource::putMerkleBlock(const std::string &iso, const MerkleBlockEntity &blockEntity) {
			return doTransaction([&iso, &blockEntity, this]() {
				this->putMerkleBlockInternal(iso, blockEntity);
			});
		}

		bool MerkleBlockDataSource::putMerkleBlocks(const std::string &iso,
													const std::vector<MerkleBlockEntity> &blockEntities) {
			return doTransaction([&iso, &blockEntities, this]() {
				for (size_t i = 0; i < blockEntities.size(); ++i) {
					this->putMerkleBlockInternal(iso, blockEntities[i]);
				}
			});
		}

		bool
		MerkleBlockDataSource::putMerkleBlockInternal(const std::string &iso, const MerkleBlockEntity &blockEntity) {
			std::stringstream ss;

			ss << "INSERT INTO " << MB_TABLE_NAME << " (" <<
			   MB_BUFF << "," <<
			   MB_HEIGHT << "," <<
			   MB_ISO <<
			   ") VALUES (?, ?, ?);";

			sqlite3_stmt *stmt;
			ParamChecker::checkCondition(!_sqlite->prepare(ss.str(), &stmt, nullptr), Error::SqliteError,
										 "prepare sql " + ss.str());
#ifdef NDEBUG
			_sqlite->bindBlob(stmt, 1, blockEntity.blockBytes, nullptr);
#else
			std::string str = Utils::encodeHex(blockEntity.blockBytes);
			CMBlock bytes;
			bytes.SetMemFixed((const uint8_t *) str.c_str(), str.length() + 1);
			_sqlite->bindBlob(stmt, 1, bytes, nullptr);
#endif
			_sqlite->bindInt(stmt, 2, blockEntity.blockHeight);
			_sqlite->bindText(stmt, 3, iso, nullptr);

			_sqlite->step(stmt);
			_sqlite->finalize(stmt);
			return true;
		}

		bool MerkleBlockDataSource::deleteMerkleBlock(const std::string &iso, const MerkleBlockEntity &blockEntity) {
			return doTransaction([&iso, &blockEntity, this]() {
				std::stringstream ss;

				ss << "DELETE FROM " << MB_TABLE_NAME <<
				   " WHERE " << MB_COLUMN_ID << " = " << blockEntity.id <<
				   " AND " << MB_ISO << " = " << "'" << iso << "';";

				ParamChecker::checkCondition(!_sqlite->exec(ss.str(), nullptr, nullptr), Error::SqliteError,
											 "exec sql " + ss.str());
			});
		}

		bool MerkleBlockDataSource::deleteAllBlocks(const std::string &iso) {
			return doTransaction([&iso, this]() {
				std::stringstream ss;

				ss << "DELETE FROM " << MB_TABLE_NAME <<
				   " WHERE " << MB_ISO << " = '" << iso << "';";

				ParamChecker::checkCondition(!_sqlite->exec(ss.str(), nullptr, nullptr), Error::SqliteError,
											 "exec sql " + ss.str());
			});
		}

		std::vector<MerkleBlockEntity> MerkleBlockDataSource::getAllMerkleBlocks(const std::string &iso) const {
			std::vector<MerkleBlockEntity> merkleBlocks;

			doTransaction([&iso, &merkleBlocks, this]() {
				MerkleBlockEntity merkleBlock;
				std::stringstream ss;
				ss << "SELECT " <<
				   MB_COLUMN_ID << ", " <<
				   MB_BUFF << ", " <<
				   MB_HEIGHT <<
				   " FROM " << MB_TABLE_NAME <<
				   " WHERE " << MB_ISO << " = '" << iso << "';";

				sqlite3_stmt *stmt;
				ParamChecker::checkCondition(!_sqlite->prepare(ss.str(), &stmt, nullptr), Error::SqliteError,
											 "prepare sql " + ss.str());

				while (SQLITE_ROW == _sqlite->step(stmt)) {
					CMBlock blockBytes;
					// id
					merkleBlock.id = _sqlite->columnInt(stmt, 0);

					// blockBytes
					const uint8_t *pblob = (const uint8_t *) _sqlite->columnBlob(stmt, 1);
					size_t len = _sqlite->columnBytes(stmt, 1);
#ifdef NDEBUG
					blockBytes.Resize(len);
					memcpy(blockBytes, pblob, len);

					merkleBlock.blockBytes = blockBytes;
#else
					std::string str((char *) pblob);
					merkleBlock.blockBytes = Utils::decodeHex(str);
#endif

					// blockHeight
					merkleBlock.blockHeight = _sqlite->columnInt(stmt, 2);

					merkleBlocks.push_back(merkleBlock);
				}

				_sqlite->finalize(stmt);
			});

			return merkleBlocks;
		}

	}
}

