// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

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
			DIDDataStore(Sqlite *sqlite);

			DIDDataStore(SqliteTransactionType type, Sqlite *sqlite);

			~DIDDataStore();

			bool PutDID(const std::string &iso, const DIDEntity &didEntity);

			bool UpdateDID(const std::vector<uint256> &txHashes, uint32_t blockHeight, time_t timestamp);

			bool DeleteDID(const std::string &did);

			bool DeleteDIDByTxHash(const std::string &txHash);

			bool DeleteAllDID();

			std::vector<DIDEntity> GetAllDID() const;

			std::string GetDIDByTxHash(const std::string &txHash) const;

			bool GetDIDDetails(const std::string &did, DIDEntity &didEntity) const;

			void flush();

		private:
			bool SelectDID(const std::string &did, DIDEntity &didEntity) const;

			bool InsertDID(const std::string &iso, const DIDEntity &didEntity);

			bool UpdateDID(const std::string &iso, const DIDEntity &didEntity);

			bool ContainTxHash(const std::string &txHash) const;

		private:
			const std::string DID_TABLE_NAME = "didTable";
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
