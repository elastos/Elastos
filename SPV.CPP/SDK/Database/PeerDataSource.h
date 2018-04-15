// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef __ELASTOS_SDK_PEERDATASOURCE_H__
#define __ELASTOS_SDK_PEERDATASOURCE_H__

#include "BRInt.h"
#include "Sqlite.h"

namespace Elastos {
	namespace SDK {

		typedef struct {
			UInt128 address;
			uint16_t port;
			uint64_t timeStamp;
		} PeerEntity;

		class PeerDataSource {

		public:
			PeerDataSource(Sqlite *sqlite);
			~PeerDataSource();

			bool putPeers(const std::string &iso, const std::vector<PeerEntity> &peerEntities);
			bool deletePeer(const std::string &iso, const PeerEntity &peerEntity);
			bool deleteAllPeers(const std::string &iso);
			std::vector<PeerEntity> getAllPeers(const std::string &iso) const;

		private:
			Sqlite *_sqlite;


			const std::string PEER_TABLE_NAME_OLD = "peerTable";
			const std::string PEER_TABLE_NAME = "peerTable_v2";
			const std::string PEER_COLUMN_ID = "_id";
			const std::string PEER_ADDRESS = "peerAddress";
			const std::string PEER_PORT = "peerPort";
			const std::string PEER_TIMESTAMP = "peerTimestamp";
			const std::string PEER_ISO = "peerIso";
			const std::string PEER_DATABASE_CREATE = "create table if not exists " + PEER_TABLE_NAME + " (" +
				PEER_COLUMN_ID + " integer primary key autoincrement, " +
				PEER_ADDRESS + " blob," +
				PEER_PORT + " blob," +
				PEER_TIMESTAMP + " blob," +
				PEER_ISO + "  text default 'BTC');";
		};

	}
}


#endif //__ELASTOS_SDK_PEERDATASOURCE_H__
