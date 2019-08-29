// Copyright (c) 2012-2019 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef __ELASTOS_SDK_COINBASEUTXODATASTORE_H__
#define __ELASTOS_SDK_COINBASEUTXODATASTORE_H__

#include "Sqlite.h"
#include "TableBase.h"

#include <SDK/Common/uint256.h>
#include <SDK/Common/BigInt.h>

namespace Elastos {
	namespace ElaWallet {

		class UTXO;
		typedef boost::shared_ptr<UTXO> UTXOPtr;

		class CoinBaseUTXODataStore : public TableBase {
		public:
			explicit CoinBaseUTXODataStore(Sqlite *sqlite);

			~CoinBaseUTXODataStore();

			bool Put(const std::vector<UTXOPtr> &entitys);

			bool Put(const UTXOPtr &entity);

			bool DeleteAll();

			size_t GetTotalCount() const;

			std::vector<UTXOPtr> GetAll() const;

			bool Update(const std::vector<uint256> &txHashes, uint32_t blockHeight, time_t timestamp);

			bool UpdateSpent(const std::vector<uint256> &txHashes);

			bool Delete(const uint256 &hash);

			void flush();
		private:
			bool PutInternal(const UTXOPtr &entity);

		private:
			/*
			 * coin base utxo table
			 */
			const std::string _tableName = "coinBaseUTXOTable";

			const std::string _txHash = "txHash";
			const std::string _blockHeight = "blockHeight";
			const std::string _timestamp = "timestamp";

			const std::string _index = "outputIndex";
			const std::string _programHash = "programHash";
			const std::string _assetID = "assetID";
			const std::string _outputLock = "outputLock";
			const std::string _amount = "amount";
			const std::string _payload = "payload";

			const std::string _spent = "spent";

			const std::string _databaseCreate = "create table if not exists " + _tableName + " (" +
												_txHash + " text not null, " +
												_blockHeight + " INTEGER, " +
												_timestamp + " INTEGER, " +
												_index + " INTEGER, " +
												_programHash + " BLOB, " +
												_assetID + " BLOB, " +
												_outputLock + " INTEGER, " +
												_amount + " TEXT DEFAULT '0', " +
												_payload + " BLOB, " +
												_spent + " INTEGER);";
		};

	}
}


#endif //__ELASTOS_SDK_COINBASEUTXODATASTORE_H__
