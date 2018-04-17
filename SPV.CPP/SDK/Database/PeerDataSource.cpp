// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "PreCompiled.h"
#include <sstream>

#include "ByteData.h"
#include "PeerDataSource.h"

namespace Elastos {
	namespace SDK {

		PeerDataSource::PeerDataSource(Sqlite *sqlite) :
			_sqlite(sqlite),
			_txType(IMMEDIATE) {
			_sqlite->transaction(_txType, PEER_DATABASE_CREATE, nullptr, nullptr);
		}

		PeerDataSource::PeerDataSource(SqliteTransactionType type, Sqlite *sqlite) :
			_sqlite(sqlite),
			_txType(type) {
			_sqlite->transaction(_txType, PEER_DATABASE_CREATE, nullptr, nullptr);
		}

		PeerDataSource::~PeerDataSource() {
		}

		bool PeerDataSource::putPeer(const std::string &iso, const PeerEntity &peerEntity) {
			std::stringstream ss;

			ss << "INSERT INTO " << PEER_TABLE_NAME << " (" <<
				PEER_ADDRESS   << "," <<
				PEER_PORT      << "," <<
				PEER_TIMESTAMP << "," <<
				PEER_ISO       <<
				") VALUES (?, ?, ?, ?);";

			_sqlite->exec("BEGIN " + _sqlite->getTxTypeString(_txType) + ";", nullptr, nullptr);

			sqlite3_stmt *stmt;
			if (true != _sqlite->prepare(ss.str(), &stmt, nullptr)) {
				return false;
			}

			ByteData addr((uint8_t*)&peerEntity.address.u8[0], sizeof(peerEntity.address.u8));
			_sqlite->bindBlob(stmt, 1, addr, nullptr);
			_sqlite->bindInt(stmt, 2, peerEntity.port);
			_sqlite->bindInt64(stmt, 3, peerEntity.timeStamp);
			_sqlite->bindText(stmt, 4, iso, nullptr);

			_sqlite->step(stmt);

			_sqlite->finalize(stmt);

			return _sqlite->exec("COMMIT;", nullptr, nullptr);
		}

		bool PeerDataSource::putPeers(const std::string &iso, const std::vector<PeerEntity> &peerEntities) {
			for (size_t i = 0; i < peerEntities.size(); ++i) {
				if (true != putPeer(iso, peerEntities[i])) {
					return false;
				}
			}
			return true;
		}

		bool PeerDataSource::deletePeer(const std::string &iso, const PeerEntity &peerEntity) {
			std::stringstream ss;

			ss << "DELETE FROM " << PEER_TABLE_NAME <<
				" WHERE " << PEER_COLUMN_ID << " = " << peerEntity.id <<
				" AND " << PEER_ISO << " = '" << iso << "';";

			_sqlite->transaction(_txType, ss.str(), nullptr, nullptr);

			return true;
		}

		bool PeerDataSource::deleteAllPeers(const std::string &iso) {
			std::stringstream ss;

			ss << "DELETE FROM " << PEER_TABLE_NAME <<
				" WHERE " << PEER_ISO << " = '" << iso << "';";

			_sqlite->transaction(_txType, ss.str(), nullptr, nullptr);

			return true;
		}

		std::vector<PeerEntity> PeerDataSource::getAllPeers(const std::string &iso) const {
			PeerEntity peer;
			std::vector<PeerEntity> peers;
			std::stringstream ss;

			ss << "SELECT " <<
				PEER_COLUMN_ID << ", " <<
				PEER_ADDRESS   << ", " <<
				PEER_PORT      << ", " <<
				PEER_TIMESTAMP <<
				" FROM " << PEER_TABLE_NAME <<
				" WHERE " << PEER_ISO << " = '" << iso << "';";

			_sqlite->exec("BEGIN " + _sqlite->getTxTypeString(_txType) + ";", nullptr, nullptr);

			sqlite3_stmt *stmt;
			if (true != _sqlite->prepare(ss.str(), &stmt, nullptr)) {
				return peers;
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

			_sqlite->exec("COMMIT;", nullptr, nullptr);

			return peers;
		}

	}
}

