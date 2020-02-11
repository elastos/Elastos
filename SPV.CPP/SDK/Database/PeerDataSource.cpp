// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "PeerDataSource.h"

#include <Common/Log.h>
#include <Common/ErrorChecker.h>

#include <sstream>

namespace Elastos {
	namespace ElaWallet {

		PeerDataSource::PeerDataSource(Sqlite *sqlite) :
			TableBase(sqlite) {
			InitializeTable(PEER_DATABASE_CREATE);
		}

		PeerDataSource::PeerDataSource(SqliteTransactionType type, Sqlite *sqlite) :
			TableBase(type, sqlite) {
			InitializeTable(PEER_DATABASE_CREATE);
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
			std::string sql;

			sql = "INSERT INTO " + PEER_TABLE_NAME + " (" + PEER_ADDRESS + "," + PEER_PORT + "," +
				  PEER_TIMESTAMP + "," + PEER_ISO + ") VALUES (?, ?, ?, ?);";

			sqlite3_stmt *stmt;
			if (!_sqlite->Prepare(sql, &stmt, nullptr)) {
				Log::error("prepare sql: {}" + sql);
				return false;
			}

			if (!_sqlite->BindBlob(stmt, 1, peerEntity.address.begin(), peerEntity.address.size(), nullptr) ||
				!_sqlite->BindInt(stmt, 2, peerEntity.port) ||
				!_sqlite->BindInt64(stmt, 3, peerEntity.timeStamp) ||
				!_sqlite->BindText(stmt, 4, "", nullptr)) {
				Log::error("bind args");
			}

			if (SQLITE_DONE != _sqlite->Step(stmt)) {
				Log::error("step");
			}

			if (!_sqlite->Finalize(stmt)) {
				Log::error("Peer put finalize");
				return false;
			}

			return true;
		}

		bool PeerDataSource::DeletePeer(const PeerEntity &peerEntity) {
			return DoTransaction([&peerEntity, this]() {
				std::string sql;

				sql = "DELETE FROM " + PEER_TABLE_NAME + " WHERE " + PEER_ADDRESS + " = ? AND " + PEER_PORT + " = ?;";

				sqlite3_stmt *stmt;
				if (!_sqlite->Prepare(sql, &stmt, nullptr)) {
					Log::error("prepare sql: {}", sql);
					return false;
				}

				if (!_sqlite->BindBlob(stmt, 1, peerEntity.address.begin(), peerEntity.address.size(), nullptr) ||
					!_sqlite->BindInt(stmt, 2, peerEntity.port)) {
					Log::error("bind args");
				}

				if (SQLITE_DONE != _sqlite->Step(stmt)) {
					Log::error("step");
				}

				if (!_sqlite->Finalize(stmt)) {
					Log::error("Peer delete finalize");
					return false;
				}

				return true;
			});
		}

		bool PeerDataSource::DeleteAllPeers() {
			return DoTransaction([this]() {

				std::string sql = "DELETE FROM " + PEER_TABLE_NAME + ";";

				if (!_sqlite->exec(sql, nullptr, nullptr)) {
					Log::error("exec sql: {}", sql);
					return false;
				}

				return true;
			});
		}

		bool PeerDataSource::Contain(const PeerEntity &entity) const {
			bool contain = false;

			std::string sql;

			sql = "SELECT " +
				  PEER_ADDRESS + "," +
				  PEER_PORT +
				  " FROM " + PEER_TABLE_NAME +
				  " WHERE " + PEER_ADDRESS + " = ? AND " + PEER_PORT + " = ?;";

			sqlite3_stmt *stmt;
			if (!_sqlite->Prepare(sql, &stmt, nullptr)) {
				Log::error("prepare sql: {}" + sql);
				return false;
			}

			if (!_sqlite->BindBlob(stmt, 1, entity.address.begin(), entity.address.size(), nullptr) ||
				!_sqlite->BindInt(stmt, 2, entity.port)) {
				Log::error("bind args");
			}

			if (SQLITE_ROW == _sqlite->Step(stmt)) {
				contain = true;
			}

			if (!_sqlite->Finalize(stmt)) {
				Log::error("Peer contain finalize");
				return false;
			}

			return contain;
		}

		std::vector<PeerEntity> PeerDataSource::GetAllPeers() const {
			std::vector<PeerEntity> peers;

			PeerEntity peer;
			std::string sql;

			sql = "SELECT " + PEER_COLUMN_ID + ", " + PEER_ADDRESS + ", " + PEER_PORT + ", " +
				  PEER_TIMESTAMP + " FROM " + PEER_TABLE_NAME + ";";

			sqlite3_stmt *stmt;
			if (!_sqlite->Prepare(sql, &stmt, nullptr)) {
				Log::error("prepare sql: {}", sql);
				return {};
			}

			while (SQLITE_ROW == _sqlite->Step(stmt)) {
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

				peers.push_back(peer);
			}

			if (!_sqlite->Finalize(stmt)) {
				Log::error("Peer get all finalize");
				return {};
			}

			return peers;
		}

		size_t PeerDataSource::GetAllPeersCount() const {
			size_t count = 0;

			std::string sql;

			sql = "SELECT COUNT(" + PEER_COLUMN_ID + ") AS nums FROM " + PEER_TABLE_NAME + ";";

			sqlite3_stmt *stmt;
			if (!_sqlite->Prepare(sql, &stmt, nullptr)) {
				Log::error("prepare sql: {}", sql);
				return 0;
			}

			if (SQLITE_ROW == _sqlite->Step(stmt)) {
				count = (uint32_t) _sqlite->ColumnInt(stmt, 0);
			}

			if (!_sqlite->Finalize(stmt)) {
				Log::error("Peer get all count finalize");
				return 0;
			}

			return count;
		}

	}
}

