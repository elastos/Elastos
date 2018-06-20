// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "PreCompiled.h"
#include <sstream>

#include "CMemBlock.h"
#include "PeerDataSource.h"

namespace Elastos {
	namespace ElaWallet {

		PeerDataSource::PeerDataSource(Sqlite *sqlite) :
			TableBase(sqlite) {
			initializeTable(PEER_DATABASE_CREATE);
		}

		PeerDataSource::PeerDataSource(SqliteTransactionType type, Sqlite *sqlite) :
			TableBase(type, sqlite) {
			initializeTable(PEER_DATABASE_CREATE);
		}

		PeerDataSource::~PeerDataSource() {
		}

		bool PeerDataSource::putPeer(const std::string &iso, const PeerEntity &peerEntity) {
			return doTransaction([&iso, &peerEntity, this]() {
				this->putPeerInternal(iso, peerEntity);
			});
		}

		bool PeerDataSource::putPeers(const std::string &iso, const std::vector<PeerEntity> &peerEntities) {
			return doTransaction([&iso, &peerEntities, this] {
				for (size_t i = 0; i < peerEntities.size(); ++i) {
					this->putPeerInternal(iso, peerEntities[i]);
				}
			});
		}

		bool PeerDataSource::putPeerInternal(const std::string &iso, const PeerEntity &peerEntity) {
			std::stringstream ss;

			ss << "INSERT INTO " << PEER_TABLE_NAME << " (" <<
			   PEER_ADDRESS   << "," <<
			   PEER_PORT      << "," <<
			   PEER_TIMESTAMP << "," <<
			   PEER_ISO       <<
			   ") VALUES (?, ?, ?, ?);";

			sqlite3_stmt *stmt;
			if (!_sqlite->prepare(ss.str(), &stmt, nullptr)) {
				throw std::logic_error("put peer prepare error");
			}

			CMBlock addr((uint64_t)sizeof(peerEntity.address.u8));
			memcpy(addr, &peerEntity.address.u8[0], sizeof(peerEntity.address.u8));
			_sqlite->bindBlob(stmt, 1, addr, nullptr);
			_sqlite->bindInt(stmt, 2, peerEntity.port);
			_sqlite->bindInt64(stmt, 3, peerEntity.timeStamp);
			_sqlite->bindText(stmt, 4, iso, nullptr);

			_sqlite->step(stmt);

			_sqlite->finalize(stmt);

			return true;
		}

		bool PeerDataSource::deletePeer(const std::string &iso, const PeerEntity &peerEntity) {
			return doTransaction([&iso, &peerEntity, this]() {
				std::stringstream ss;

				ss << "DELETE FROM " << PEER_TABLE_NAME <<
				   " WHERE " << PEER_COLUMN_ID << " = " << peerEntity.id <<
				   " AND " << PEER_ISO << " = '" << iso << "';";

				if (!_sqlite->exec(ss.str(), nullptr, nullptr)) {
					std::stringstream ess;
					ess << "exec sql " << ss.str() << " fail";
					throw std::logic_error(ess.str());
				}
			});
		}

		bool PeerDataSource::deleteAllPeers(const std::string &iso) {
			return doTransaction([&iso, this]() {
				std::stringstream ss;

				ss << "DELETE FROM " << PEER_TABLE_NAME <<
				   " WHERE " << PEER_ISO << " = '" << iso << "';";

				if (!_sqlite->exec(ss.str(), nullptr, nullptr)) {
					std::stringstream ess;
					ess << "exec sql " << ss.str() << " fail";
					throw std::logic_error(ess.str());
				}
			});
		}

		std::vector<PeerEntity> PeerDataSource::getAllPeers(const std::string &iso) const {
			std::vector<PeerEntity> peers;

			doTransaction([&iso, &peers, this]() {
				PeerEntity peer;
				std::stringstream ss;

				ss << "SELECT " <<
				   PEER_COLUMN_ID << ", " <<
				   PEER_ADDRESS   << ", " <<
				   PEER_PORT      << ", " <<
				   PEER_TIMESTAMP <<
				   " FROM " << PEER_TABLE_NAME <<
				   " WHERE " << PEER_ISO << " = '" << iso << "';";

				sqlite3_stmt *stmt;
				if (!_sqlite->prepare(ss.str(), &stmt, nullptr)) {
					std::stringstream ess;
					ess << "prepare sql " << ss.str() << " fail";
					throw std::logic_error(ess.str());
				}

				while (SQLITE_ROW == _sqlite->step(stmt)) {
					// id
					peer.id = _sqlite->columnInt(stmt, 0);

					// address
					const uint8_t *paddr = (const uint8_t *)_sqlite->columnBlob(stmt, 1);
					size_t len = _sqlite->columnBytes(stmt, 1);
					len = len <= sizeof(peer.address) ? len : sizeof(peer.address);
					memcpy(peer.address.u8, paddr, len);

					// port
					peer.port = _sqlite->columnInt(stmt, 2);

					// timestamp
					peer.timeStamp = _sqlite->columnInt64(stmt, 3);

					peers.push_back(peer);
				}

				_sqlite->finalize(stmt);
			});

			return peers;
		}

	}
}

