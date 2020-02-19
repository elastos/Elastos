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

#ifndef __ELASTOS_SDK_DIDDATASTORE_H__
#define __ELASTOS_SDK_DIDDATASTORE_H__

#include "Sqlite.h"
#include "TableBase.h"

#include <Common/uint256.h>

namespace Elastos {
	namespace ElaWallet {

		struct DIDEntity {
			DIDEntity() {}

			DIDEntity(const std::string &did, const bytes_t &payloadInfo, time_t timestamp, uint32_t blockHeight,
			          const std::string &txHash, time_t createTime) :
					DID(did),
					PayloadInfo(payloadInfo),
					TimeStamp(timestamp),
					BlockHeight(blockHeight),
					TxHash(txHash),
					CreateTime(createTime) {
			}

			std::string DID;
			bytes_t PayloadInfo;
			time_t TimeStamp;
			uint32_t BlockHeight;
			std::string TxHash;
			time_t CreateTime;
		};

		class DIDDataStore : public TableBase {
		public:
			DIDDataStore(Sqlite *sqlite, SqliteTransactionType type = IMMEDIATE);

			~DIDDataStore();

			virtual void InitializeTable();

			bool PutDID(const DIDEntity &didEntity);

			bool UpdateDID(const std::vector<uint256> &txHashes, uint32_t blockHeight, time_t timestamp);

			bool DeleteDID(const std::string &did);

			bool DeleteDIDByTxHash(const std::string &txHash);

			bool DeleteAllDID();

			std::vector<DIDEntity> GetAllDID() const;

			std::string GetDIDByTxHash(const std::string &txHash) const;

			bool GetDIDDetails(const std::string &did, DIDEntity &didEntity) const;

		private:
			bool SelectDID(const std::string &did, DIDEntity &didEntity) const;

			bool InsertDID(const DIDEntity &didEntity);

			bool UpdateDID(const DIDEntity &didEntity);

			bool ContainTxHash(const std::string &txHash) const;

		private:
			const std::string DID_OLD_TABLE_NAME = "didTable";
			const std::string DID_TABLE_NAME = "didNewdTable";
			const std::string DID_COLUMN_ID = "_id";
			const std::string DID_PAYLOAD_BUFF = "didBuff";
			const std::string DID_CREATE_TIME = "createTime";
			const std::string BLOCK_HEIGHT = "blockHeight";
			const std::string TIME_STAMP = "timeStamp";
			const std::string TX_HASH = "txHash";
			const std::string DID_RESERVE = "reserve";
			const std::string DID_DATABASE_CREATE = "create table if not exists " + DID_TABLE_NAME + " (" +
			                                        DID_COLUMN_ID + " text not null, " +
			                                        DID_PAYLOAD_BUFF + " blob, " +
			                                        DID_CREATE_TIME + " integer, " +
			                                        BLOCK_HEIGHT + " integer, " +
			                                        TIME_STAMP + " integer, " +
			                                        TX_HASH + " text DEFAULT '', " +
			                                        DID_RESERVE + " text DEFAULT 'ELA');";
		};
	}
}

#endif //__ELASTOS_SDK_DIDDATASTORE_H__
