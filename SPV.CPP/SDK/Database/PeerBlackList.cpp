// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "PeerBlackList.h"

#include <Common/Log.h>
#include <Common/ErrorChecker.h>

namespace Elastos {
	namespace ElaWallet {

		PeerBlackList::PeerBlackList(Sqlite *sqlite) :
			TableBase(sqlite) {
			InitializeTable(_tableCreateSql);
		}

		PeerBlackList::PeerBlackList(SqliteTransactionType type, Sqlite *sqlite) :
			TableBase(type, sqlite) {
			InitializeTable(_tableCreateSql);
		}

		PeerBlackList::~PeerBlackList() {
		}

		bool PeerBlackList::PutPeer(const PeerEntity &peerEntity) {
			if (Contain(peerEntity))
				return true;

			return DoTransaction([&peerEntity, this]() {
				return this->PutPeerInternal(peerEntity);
			});
		}

		bool PeerBlackList::PutPeers(const std::vector<PeerEntity> &peerEntities) {
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

		bool PeerBlackList::PutPeerInternal(const PeerEntity &peerEntity) {
			std::string sql;

			sql = "INSERT INTO " + _peerBlackListTable + " (" + _address + "," + _port + "," +
				  _timestamp + ") VALUES (?, ?, ?);";

			sqlite3_stmt *stmt;
			if (!_sqlite->Prepare(sql, &stmt, nullptr)) {
				Log::error("prepare sql: {}" + sql);
				return false;
			}

			if (!_sqlite->BindBlob(stmt, 1, peerEntity.address.begin(), peerEntity.address.size(), nullptr) ||
				!_sqlite->BindInt(stmt, 2, peerEntity.port) ||
				!_sqlite->BindInt64(stmt, 3, peerEntity.timeStamp)) {
				Log::error("bind args");
			}

			if (SQLITE_DONE != _sqlite->Step(stmt)) {
				Log::error("step");
			}

			if (!_sqlite->Finalize(stmt)) {
				Log::error("Peer bl put finalize");
				return false;
			}

			return true;
		}

		bool PeerBlackList::DeletePeer(const PeerEntity &peerEntity) {
			return DoTransaction([&peerEntity, this]() {
				std::string sql;

				sql = "DELETE FROM " + _peerBlackListTable + " WHERE " + _address + " = ? AND " + _port + " = ?;";

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
					Log::error("Peer bl delete finalize");
					return false;
				}

				return true;
			});
		}

		bool PeerBlackList::DeleteAllPeers() {
			return DoTransaction([this]() {

				std::string sql = "DELETE FROM " + _peerBlackListTable + ";";

				if (!_sqlite->exec(sql, nullptr, nullptr)) {
					Log::error("exec sql: {}", sql);
					return false;
				}

				return true;
			});
		}

		bool PeerBlackList::Contain(const PeerEntity &entity) const {
			bool contain = false;

			std::string sql;

			sql = "SELECT " +
				  _address + "," +
				  _port +
				  " FROM " + _peerBlackListTable +
				  " WHERE " + _address + " = ? AND " + _port + " = ?;";

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
				Log::error("Peer bl contain finalize");
				return false;
			}

			return contain;
		}

		std::vector<PeerEntity> PeerBlackList::GetAllPeers() const {
			std::vector<PeerEntity> peers;

			PeerEntity peer;
			std::string sql;

			sql = "SELECT " + _columnID + ", " + _address + ", " + _port + ", " +
				  _timestamp + " FROM " + _peerBlackListTable + ";";

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
				Log::error("Peer bl get all finalize");
				return {};
			}

			return peers;
		}

		void PeerBlackList::flush() {
			_sqlite->flush();
		}

		size_t PeerBlackList::GetAllPeersCount() const {
			size_t count = 0;

			std::string sql;

			sql = "SELECT COUNT(" + _columnID + ") AS nums FROM " + _peerBlackListTable + ";";

			sqlite3_stmt *stmt;
			if (!_sqlite->Prepare(sql, &stmt, nullptr)) {
				Log::error("prepare sql: {}", sql);
				return 0;
			}

			if (SQLITE_ROW == _sqlite->Step(stmt)) {
				count = (uint32_t) _sqlite->ColumnInt(stmt, 0);
			}

			if (!_sqlite->Finalize(stmt)) {
				Log::error("Peer bl get count finalize");
				return 0;
			}

			return count;
		}

	}
}

