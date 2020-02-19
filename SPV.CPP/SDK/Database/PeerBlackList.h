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

#ifndef __ELASTOS_SDK_PEERBLACKLIST_H__
#define __ELASTOS_SDK_PEERBLACKLIST_H__

#include "Sqlite.h"
#include "TableBase.h"

#include "PeerDataSource.h"

namespace Elastos {
	namespace ElaWallet {

		class PeerBlackList: public TableBase {

		public:
			PeerBlackList(Sqlite *sqlite, SqliteTransactionType type = IMMEDIATE);

			~PeerBlackList();

			virtual void InitializeTable();

			bool PutPeer(const PeerEntity &peerEntity);

			bool PutPeers(const std::vector<PeerEntity> &peerEntities);

			bool DeletePeer(const PeerEntity &peerEntity);

			bool DeleteAllPeers();

			size_t GetAllPeersCount() const;

			std::vector<PeerEntity> GetAllPeers() const;

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

