// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef __ELASTOS_SDK_PEERBLACKLIST_H__
#define __ELASTOS_SDK_PEERBLACKLIST_H__

#include "Sqlite.h"
#include "TableBase.h"

#include "PeerDataSource.h"

namespace Elastos {
	namespace ElaWallet {

		class PeerBlackList: public TableBase {

		public:
			PeerBlackList(Sqlite *sqlite);

			PeerBlackList(SqliteTransactionType type, Sqlite *sqlite);

			~PeerBlackList();

			bool PutPeer(const PeerEntity &peerEntity);

			bool PutPeers(const std::vector<PeerEntity> &peerEntities);

			bool DeletePeer(const PeerEntity &peerEntity);

			bool DeleteAllPeers();

			size_t GetAllPeersCount() const;

			std::vector<PeerEntity> GetAllPeers() const;

			void flush();

		private:
			bool Contain(const PeerEntity &entity) const;

			bool PutPeerInternal(const PeerEntity &peerEntity);

		private:
			/*
			 * peer black list table
			 */
			const std::string _peerBlackListTable = "peerBlackList";
			const std::string _columnID = "_id";
			const std::string _address = "peerAddress";
			const std::string _port = "peerPort";
			const std::string _timestamp = "peerTimestamp";

			const std::string _tableCreateSql = "create table if not exists " + _peerBlackListTable + " (" +
													 _columnID + " integer primary key autoincrement, " +
													 _address + " blob," +
													 _port + " integer," +
													 _timestamp + " integer);";
		};

	}
}

#endif //__ELASTOS_SDK_PEERBLACKLIST_H__

