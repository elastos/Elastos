// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "PreCompiled.h"
#include <sstream>

#include "PeerDataSource.h"

namespace Elastos {
	namespace SDK {

		PeerDataSource::PeerDataSource(Sqlite *sqlite) :
			_sqlite(sqlite) {
			std::string sqlUpper = PEER_DATABASE_CREATE;
			std::transform(sqlUpper.begin(), sqlUpper.end(), sqlUpper.begin(), ::toupper);
			_sqlite->execSql(sqlUpper, nullptr, nullptr);
		}

		PeerDataSource::~PeerDataSource() {
		}

		bool PeerDataSource::putPeers(const std::string &iso, const std::vector<PeerEntity> &peerEntities) {
			return true;
		}

		bool PeerDataSource::deletePeer(const std::string &iso, const PeerEntity &peerEntity) {
			return true;
		}

		bool deleteAllPeers(const std::string &iso) {
			return true;
		}

		std::vector<PeerEntity> PeerDataSource::getAllPeers(const std::string &iso) const {
			std::vector<PeerEntity> peers;
			std::stringstream ss;

			std::string isoUpper = iso;
			std::transform(isoUpper.begin(), isoUpper.end(), isoUpper.begin(), ::toupper);
			ss << isoUpper;

			_sqlite->execSql(ss.str(), nullptr, nullptr);

			return peers;
		}

	}
}
