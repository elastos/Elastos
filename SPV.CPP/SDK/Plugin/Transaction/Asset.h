// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef __ELASTOS_SDK_SPVSDK_ASSET_H
#define __ELASTOS_SDK_SPVSDK_ASSET_H

#include <Plugin/Interface/ELAMessageSerializable.h>
#include <Common/JsonSerializer.h>

namespace Elastos {
	namespace ElaWallet {

#define TOKEN_ASSET_PRECISION "1000000000000000000"

		class Asset :
				public ELAMessageSerializable, public JsonSerializer {
		public:
			enum AssetType {
				Token = 0x00,
				Share = 0x01,
			};
			enum AssetRecordType {
				Unspent = 0x00,
				Balance = 0x01,
			};
			const static uint8_t MaxPrecision = 18;
		public:
			Asset();

			Asset(const std::string &name, const std::string &desc, uint8_t precision,
				  AssetType assetType = Token, AssetRecordType recordType = Unspent);

			Asset(const Asset &asset);

			~Asset();

			Asset &operator=(const Asset &asset);

			void SetName(const std::string &name) {
				_name = name;
			}

			const std::string &GetName() const {
				return _name;
			}

			void SetDescription(const std::string &desc) {
				_description = desc;
			}

			const std::string &GetDescription() const {
				return _description;
			}

			void SetAssetType(Asset::AssetType type) {
				_assetType = type;
			}

			Asset::AssetType GetAssetType() const {
				return _assetType;
			}

			void SetAssetRecordType(Asset::AssetRecordType type) {
				_recordType = type;
			}

			Asset::AssetRecordType GetAssetRecordType() const {
				return _recordType;
			}

			void SetPrecision(uint8_t precision) {
				_precision = precision;
			}

			uint8_t GetPrecision() const {
				return _precision;
			}

			virtual size_t EstimateSize() const;

			virtual void Serialize(ByteStream &ostream) const;

			virtual bool Deserialize(const ByteStream &istream);

			virtual nlohmann::json ToJson() const;

			virtual void FromJson(const nlohmann::json &jsonData);

			const uint256 &GetHash() const;

			void SetHash(const uint256 &hash);

		public:
			static const uint256 &GetELAAssetID();

		private:
			std::string _name;
			std::string _description;
			uint8_t _precision;
			AssetType _assetType;
			AssetRecordType _recordType;
			mutable uint256 _hash;

			static uint256 _elaAsset;
		};

		typedef boost::shared_ptr<Asset> AssetPtr;
	}
}

#endif //__ELASTOS_SDK_SPVSDK_ASSET_H
