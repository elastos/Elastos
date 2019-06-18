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

		class CoinBaseUTXOEntity {
		public:
			CoinBaseUTXOEntity();

			~CoinBaseUTXOEntity();

			bool Spent() const;

			void SetSpent(bool status);

			const uint16_t &Index() const;

			void SetIndex(uint16_t index);

			const uint168 &ProgramHash() const;

			void SetProgramHash(const uint168 &hash);

			const uint256 &AssetID() const;

			void SetAssetID(const uint256 &ID);

			const uint32_t &OutputLock() const;

			void SetOutputLock(uint32_t outputLock);

			const BigInt &Amount() const;

			void SetAmount(const BigInt &amount);

			const bytes_ptr &Payload() const;

			void SetPayload(const bytes_ptr &payload);

			const time_t &Timestamp() const;

			void SetTimestamp(time_t timestamp);

			const uint32_t &BlockHeight() const;

			void SetBlockHeight(uint32_t blockHeight);

			const std::string &TxHash() const;

			void SetTxHash(const std::string &txHash);

		private:
			bool _spent;

			// output
			uint16_t _index;
			uint168 _programHash;
			uint256 _assetID;
			uint32_t _outputLock;
			BigInt _amount;
			bytes_ptr _payload;

			time_t _timestamp;
			uint32_t _blockHeight;
			std::string _txHash;
		};

		typedef boost::shared_ptr<CoinBaseUTXOEntity> CoinBaseUTXOEntityPtr;

		class CoinBaseUTXODataStore : public TableBase {
		public:
			explicit CoinBaseUTXODataStore(Sqlite *sqlite);

			~CoinBaseUTXODataStore();

			bool Put(const std::vector<CoinBaseUTXOEntity> &entitys);

			bool Put(const CoinBaseUTXOEntity &entity);

			bool DeleteAll();

			size_t GetTotalCount() const;

			std::vector<CoinBaseUTXOEntityPtr> GetAll() const;

			bool Update(const std::vector<uint256> &txHashes, uint32_t blockHeight, time_t timestamp);

			bool UpdateSpent(const std::vector<uint256> &txHashes);

			bool Delete(const std::string &hash);

		private:
			bool PutInternal(const CoinBaseUTXOEntity &entity);

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
