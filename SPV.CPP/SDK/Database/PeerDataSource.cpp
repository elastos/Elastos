/*
 * Copyright (c) 2019 Elastos Foundation
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include "PeerDataSource.h"

#include <Common/Log.h>
#include <Common/ErrorChecker.h>

#include <sstream>

namespace Elastos {
	namespace ElaWallet {

		PeerDataSource::PeerDataSource(Sqlite *sqlite, SqliteTransactionType type) :
			TableBase(type, sqlite) {
			TableBase::ExecInTransaction("DROP TABLE IF EXISTS " + PEER_OLD_TABLE_NAME + ";");
			TableBase::ExecInTransaction(PEER_DATABASE_CREATE);
		}

		PeerDataSource::~PeerDataSource() {
		}

		bool PeerDataSource::PutPeer(const PeerEntity &peerEntity) {
			return DoTransaction([&peerEntity, this]() {
				return this->PutPeerInternal(peerEntity);
			});
		}

		bool PeerDataSource::PutPeers(const std::vector<PeerEntity> &peerEntities) {
			if (peerEntities.empty())
				return true;

			return DoTransaction([&peerEntities, this] {
				for (size_t i = 0; i < peerEntities.size(); ++i) {
					if (!this->PutPeerInternal(peerEntities[i]))
						return false;
				}

				return true;
			});
		}

		bool PeerDataSource::PutPeerInternal(const PeerEntity &peerEntity) {
			std::string sql = "INSERT INTO " + PEER_TABLE_NAME + " (" + PEER_ADDRESS + "," + PEER_PORT + "," +
				  PEER_TIMESTAMP + "," + PEER_SPEED + ") VALUES (?, ?, ?, ?);";

			return SqliteWrapper(sql, [&peerEntity, this](sqlite3_stmt *stmt) {
				if (!_sqlite->BindBlob(stmt, 1, peerEntity.address.begin(), peerEntity.address.size(), nullptr) ||
					!_sqlite->BindInt(stmt, 2, peerEntity.port) ||
					!_sqlite->BindInt64(stmt, 3, peerEntity.timeStamp) ||
					!_sqlite->BindInt(stmt, 4, peerEntity.speed)) {
					Log::error("bind args");
					return false;
				}

				if (SQLITE_DONE != _sqlite->Step(stmt)) {
					Log::error("step");
					return false;
				}

				return true;
			});
		}

		bool PeerDataSource::DeletePeer(const PeerEntity &peerEntity) {
			return DoTransaction([&peerEntity, this]() {
				std::string sql = "DELETE FROM " + PEER_TABLE_NAME + " WHERE " + PEER_ADDRESS + " = ? AND " + PEER_PORT + " = ?;";

				return SqliteWrapper(sql, [&peerEntity, this](sqlite3_stmt *stmt) {
					if (!_sqlite->BindBlob(stmt, 1, peerEntity.address.begin(), peerEntity.address.size(), nullptr) ||
						!_sqlite->BindInt(stmt, 2, peerEntity.port)) {
						Log::error("bind args");
						return false;
					}

					if (SQLITE_DONE != _sqlite->Step(stmt)) {
						Log::error("step");
						return false;
					}

					return true;
				});
			});
		}

		bool PeerDataSource::DeleteAllPeers() {
			return DeleteAll(PEER_TABLE_NAME);
		}

		bool PeerDataSource::Contain(const PeerEntity &entity) const {
			bool contain = false;

			std::string sql = "SELECT " +
				  PEER_ADDRESS + "," +
				  PEER_PORT +
				  " FROM " + PEER_TABLE_NAME +
				  " WHERE " + PEER_ADDRESS + " = ? AND " + PEER_PORT + " = ?;";

			if (!SqliteWrapper(sql, [&contain, &entity, this](sqlite3_stmt *stmt) {
				if (!_sqlite->BindBlob(stmt, 1, entity.address.begin(), entity.address.size(), nullptr) ||
					!_sqlite->BindInt(stmt, 2, entity.port)) {
					Log::error("bind args");
					return false;
				}

				if (SQLITE_ROW == _sqlite->Step(stmt))
					contain = true;

				return true;
			})) {
				return false;
			}

			return contain;
		}

		std::vector<PeerEntity> PeerDataSource::GetAllPeers() const {
			std::vector<PeerEntity> peers;

			std::string sql = "SELECT * FROM " + PEER_TABLE_NAME + ";";

			if (!SqliteWrapper(sql, [&peers, this](sqlite3_stmt *stmt) {
				while (SQLITE_ROW == _sqlite->Step(stmt)) {
					PeerEntity peer;
					// id
					peer.id = _sqlite->ColumnInt(stmt, 0);

					// address
					const uint8_t *paddr = (const uint8_t *) _sqlite->ColumnBlob(stmt, 1);
					size_t len = _sqlite->ColumnBytes(stmt, 1);
					assert(len == peer.address.size());
					len = len <= peer.address.size() ? len : peer.address.size();
					memcpy(peer.address.begin(), paddr, len);

					// port
					peer.port = _sqlite->ColumnInt(stmt, 2);

					// timestamp
					peer.timeStamp = _sqlite->ColumnInt64(stmt, 3);

					// speed
					peer.speed = _sqlite->ColumnInt(stmt, 4);

					peers.push_back(peer);
				}

				return true;
			})) {
				return {};
			}

			return peers;
		}

		size_t PeerDataSource::GetAllPeersCount() const {
			size_t count = 0;
			std::string sql = "SELECT COUNT(" + PEER_COLUMN_ID + ") AS nums FROM " + PEER_TABLE_NAME + ";";

			if (!SqliteWrapper(sql, [&count, this](sqlite3_stmt *stmt) {
				if (SQLITE_ROW == _sqlite->Step(stmt)) {
					count = (uint32_t) _sqlite->ColumnInt(stmt, 0);
				}

				return true;
			})) {
				return 0;
			}

			return count;
		}

	}
}

