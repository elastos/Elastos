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

#ifndef __ELASTOS_SDK_PEERDATASOURCE_H__
#define __ELASTOS_SDK_PEERDATASOURCE_H__

#include "Sqlite.h"
#include "TableBase.h"

#include <Common/uint256.h>

namespace Elastos {
	namespace ElaWallet {

		struct PeerEntity {
			PeerEntity() :
				id(0),
				port(0),
				timeStamp(0),
				speed(0)
			{
			}

			PeerEntity(long i, const uint128 &addr, uint16_t p, uint64_t ts, int s) :
				id(i),
				address(addr),
				port(p),
				timeStamp(ts),
				speed(s)
			{
			}

			long id;
			uint128 address;
			uint16_t port;
			uint64_t timeStamp;
			int speed;
		};

		class PeerDataSource : public TableBase {

		public:
			PeerDataSource(Sqlite *sqlite, SqliteTransactionType type = IMMEDIATE);

			~PeerDataSource();

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
			 * peer table
			 */
			const std::string PEER_OLD_TABLE_NAME = "peerTable";
			const std::string PEER_TABLE_NAME = "peersTable";
			const std::string PEER_COLUMN_ID = "_id";
			const std::string PEER_ADDRESS = "address";
			const std::string PEER_PORT = "port";
			const std::string PEER_TIMESTAMP = "timestamp";
			const std::string PEER_SPEED = "speed";

			const std::string PEER_DATABASE_CREATE = "CREATE TABLE IF NOT EXISTS " + PEER_TABLE_NAME + " (" +
				PEER_COLUMN_ID + " INTEGER PRIMARY KEY AUTOINCREMENT, " +
				PEER_ADDRESS + " BLOB," +
				PEER_PORT + " INTEGER," +
				PEER_TIMESTAMP + " INTEGER," +
				PEER_SPEED + " INTEGER);";
		};

	}
}

#endif //__ELASTOS_SDK_PEERDATASOURCE_H__

