// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef __ELASTOS_SDK_SPVSDK_ASSET_H
#define __ELASTOS_SDK_SPVSDK_ASSET_H

#include <SDK/Plugin/Interface/ELAMessageSerializable.h>
#include <Core/BRInt.h>

namespace Elastos {
	namespace ElaWallet {

		class Asset :
				public ELAMessageSerializable {
		public:
			enum AssetType {
				Token = 0x00,
				Share = 0x01,
			};
			enum AssetRecordType {
				Unspent = 0x00,
				Balance = 0x01,
			};
			const static short maxPrecision = 8;
			const static short MinPrecidion = 0;
		public:
			Asset();

			~Asset();

			void SetName(const std::string &name);

			std::string GetName() const;

			void SetDescription(const std::string &desc);

			std::string GetDescription() const;

			void SetAssetType(Asset::AssetType type);

			Asset::AssetType GetAssetType() const;

			void SetAssetRecordType(Asset::AssetRecordType type);

			Asset::AssetRecordType GetAssetRecordType() const;

			void SetPrecision(uint8_t precision);

			uint8_t GetPrecision() const;

			virtual void Serialize(ByteStream &ostream) const;

			virtual bool Deserialize(ByteStream &istream);

			virtual nlohmann::json ToJson() const;

			virtual void FromJson(const nlohmann::json &jsonData);

			virtual UInt256 &GetHash() const;

			void SetHash(const UInt256 &hash);

		public:
			static const UInt256 &GetELAAssetID();

		private:
			std::string _name;
			std::string _description;
			uint8_t _precision;
			AssetType _assetType;
			AssetRecordType _recordType;
			mutable UInt256 _hash;

			static UInt256 _elaAsset;
		};
	}
}

#endif //__ELASTOS_SDK_SPVSDK_ASSET_H
