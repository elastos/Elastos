// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef __ELASTOS_SDK_ASSETDATASTORE_H__
#define __ELASTOS_SDK_ASSETDATASTORE_H__

#include "TableBase.h"
#include "ELACoreExt/Payload/Asset.h"

namespace Elastos {
	namespace ElaWallet {

		struct AssetEntity {
			AssetEntity(const Asset &Asset, uint64_t amount, const UInt256 &txHash) :
					Amount(amount),
					TxHash(txHash),
					Asset(Asset) {
			}

			uint64_t Amount;
			Asset Asset;
			UInt256 TxHash;
		};

		class AssetDataStore : public TableBase {
		public:
			AssetDataStore(Sqlite *sqlite);

			AssetDataStore(SqliteTransactionType type, Sqlite *sqlite);

			~AssetDataStore();

			bool PutAsset(const std::string &iso, const AssetEntity &asset);

			bool PutAssets(const std::string &iso, const std::vector<AssetEntity> &assets);

			bool DeleteAsset(const std::string &iso, const UInt256 &assetID);

			bool DeleteAllAssets(const std::string &iso);

			AssetEntity GetAssetDetails(uint32_t assetTableID);

			std::vector<AssetEntity> GetAllMerkleBlocks(const std::string &iso) const;

		};

	}
}


#endif //__ELASTOS_SDK_ASSETDATASTORE_H__
