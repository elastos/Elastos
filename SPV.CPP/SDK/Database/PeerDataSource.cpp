// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <sstream>
#include <SDK/Common/Utils.h>
#include <SDK/Common/ParamChecker.h>

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
			   PEER_ADDRESS << "," <<
			   PEER_PORT << "," <<
			   PEER_TIMESTAMP << "," <<
			   PEER_ISO <<
			   ") VALUES (?, ?, ?, ?);";

			sqlite3_stmt *stmt;
			ParamChecker::checkCondition(!_sqlite->prepare(ss.str(), &stmt, nullptr), Error::SqliteError,
										 "Prepare sql " + ss.str());

			CMBlock addr;
			addr.SetMemFixed(&peerEntity.address.u8[0], sizeof(peerEntity.address.u8));
#ifdef NDEBUG
			_sqlite->bindBlob(stmt, 1, addr, nullptr);
#else
			std::string str = Utils::encodeHex(addr);
			addr.SetMemFixed((const uint8_t *) str.c_str(), str.length() + 1);
			_sqlite->bindBlob(stmt, 1, addr, nullptr);
#endif
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

				ParamChecker::checkCondition(!_sqlite->exec(ss.str(), nullptr, nullptr), Error::SqliteError,
											 "Exec sql " + ss.str());
			});
		}

		bool PeerDataSource::deleteAllPeers(const std::string &iso) {
			return doTransaction([&iso, this]() {
				std::stringstream ss;

				ss << "DELETE FROM " << PEER_TABLE_NAME <<
				   " WHERE " << PEER_ISO << " = '" << iso << "';";

				ParamChecker::checkCondition(!_sqlite->exec(ss.str(), nullptr, nullptr), Error::SqliteError,
											 "Exec sql " + ss.str());
			});
		}

		std::vector<PeerEntity> PeerDataSource::getAllPeers(const std::string &iso) const {
			std::vector<PeerEntity> peers;

			doTransaction([&iso, &peers, this]() {
				PeerEntity peer;
				std::stringstream ss;

				ss << "SELECT " <<
				   PEER_COLUMN_ID << ", " <<
				   PEER_ADDRESS << ", " <<
				   PEER_PORT << ", " <<
				   PEER_TIMESTAMP <<
				   " FROM " << PEER_TABLE_NAME <<
				   " WHERE " << PEER_ISO << " = '" << iso << "';";

				sqlite3_stmt *stmt;
				ParamChecker::checkCondition(!_sqlite->prepare(ss.str(), &stmt, nullptr), Error::SqliteError,
											 "Prepare sql " + ss.str());

				while (SQLITE_ROW == _sqlite->step(stmt)) {
					// id
					peer.id = _sqlite->columnInt(stmt, 0);

					// address
					const uint8_t *paddr = (const uint8_t *) _sqlite->columnBlob(stmt, 1);
					size_t len = _sqlite->columnBytes(stmt, 1);
#ifdef NDEBUG
					len = len <= sizeof(peer.address) ? len : sizeof(peer.address);
					memcpy(peer.address.u8, paddr, len);
#else
					std::string str((const char *) paddr);
					CMBlock addr = Utils::decodeHex(str);
					len = len <= sizeof(peer.address) ? len : sizeof(peer.address);
					memcpy(peer.address.u8, addr, len);
#endif

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

