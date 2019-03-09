// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "PeerDataSource.h"

#include <SDK/Common/Utils.h>
#include <SDK/Common/ErrorChecker.h>
#include <SDK/Common/CMemBlock.h>

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

		bool PeerDataSource::PutPeer(const std::string &iso, const PeerEntity &peerEntity) {
			return DoTransaction([&iso, &peerEntity, this]() {
				this->PutPeerInternal(iso, peerEntity);
			});
		}

		bool PeerDataSource::PutPeers(const std::string &iso, const std::vector<PeerEntity> &peerEntities) {
			return DoTransaction([&iso, &peerEntities, this] {
				for (size_t i = 0; i < peerEntities.size(); ++i) {
					this->PutPeerInternal(iso, peerEntities[i]);
				}
			});
		}

		bool PeerDataSource::PutPeerInternal(const std::string &iso, const PeerEntity &peerEntity) {
			std::stringstream ss;

			ss << "INSERT INTO " << PEER_TABLE_NAME << " (" <<
			   PEER_ADDRESS << "," <<
			   PEER_PORT << "," <<
			   PEER_TIMESTAMP << "," <<
			   PEER_ISO <<
			   ") VALUES (?, ?, ?, ?);";

			sqlite3_stmt *stmt;
			ErrorChecker::CheckCondition(!_sqlite->Prepare(ss.str(), &stmt, nullptr), Error::SqliteError,
										 "Prepare sql " + ss.str());

			CMBlock addr;
			addr.SetMemFixed(&peerEntity.address.u8[0], sizeof(peerEntity.address.u8));
			_sqlite->BindBlob(stmt, 1, addr, nullptr);
			_sqlite->BindInt(stmt, 2, peerEntity.port);
			_sqlite->BindInt64(stmt, 3, peerEntity.timeStamp);
			_sqlite->BindText(stmt, 4, iso, nullptr);

			_sqlite->Step(stmt);

			_sqlite->Finalize(stmt);

			return true;
		}

		bool PeerDataSource::DeletePeer(const std::string &iso, const PeerEntity &peerEntity) {
			return DoTransaction([&iso, &peerEntity, this]() {
				std::stringstream ss;

				ss << "DELETE FROM " << PEER_TABLE_NAME <<
				   " WHERE " << PEER_COLUMN_ID << " = " << peerEntity.id <<
				   " AND " << PEER_ISO << " = '" << iso << "';";

				ErrorChecker::CheckCondition(!_sqlite->exec(ss.str(), nullptr, nullptr), Error::SqliteError,
											 "Exec sql " + ss.str());
			});
		}

		bool PeerDataSource::DeleteAllPeers(const std::string &iso) {
			return DoTransaction([&iso, this]() {
				std::stringstream ss;

				ss << "DELETE FROM " << PEER_TABLE_NAME <<
				   " WHERE " << PEER_ISO << " = '" << iso << "';";

				ErrorChecker::CheckCondition(!_sqlite->exec(ss.str(), nullptr, nullptr), Error::SqliteError,
											 "Exec sql " + ss.str());
			});
		}

		std::vector<PeerEntity> PeerDataSource::GetAllPeers(const std::string &iso) const {
			std::vector<PeerEntity> peers;

			DoTransaction([&iso, &peers, this]() {
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
				ErrorChecker::CheckCondition(!_sqlite->Prepare(ss.str(), &stmt, nullptr), Error::SqliteError,
											 "Prepare sql " + ss.str());

				while (SQLITE_ROW == _sqlite->Step(stmt)) {
					// id
					peer.id = _sqlite->ColumnInt(stmt, 0);

					// address
					const uint8_t *paddr = (const uint8_t *) _sqlite->ColumnBlob(stmt, 1);
					size_t len = _sqlite->ColumnBytes(stmt, 1);
					len = len <= sizeof(peer.address) ? len : sizeof(peer.address);
					memcpy(peer.address.u8, paddr, len);

					// port
					peer.port = _sqlite->ColumnInt(stmt, 2);

					// timestamp
					peer.timeStamp = _sqlite->ColumnInt64(stmt, 3);

					peers.push_back(peer);
				}

				_sqlite->Finalize(stmt);
			});

			return peers;
		}

		size_t PeerDataSource::GetAllPeersCount(const std::string &iso) const {
			size_t count = 0;

			DoTransaction([&iso, &count, this]() {
				std::stringstream ss;

				ss << "SELECT " <<
				   " COUNT(" << PEER_COLUMN_ID << ") AS nums " <<
				   " FROM " << PEER_TABLE_NAME << ";";

				sqlite3_stmt *stmt;
				ErrorChecker::CheckCondition(!_sqlite->Prepare(ss.str(), &stmt, nullptr), Error::SqliteError,
											 "Prepare sql " + ss.str());

				while (SQLITE_ROW == _sqlite->Step(stmt)) {
					count = (uint32_t) _sqlite->ColumnInt(stmt, 0);
				}

				_sqlite->Finalize(stmt);
			});

			return count;
		}

	}
}

