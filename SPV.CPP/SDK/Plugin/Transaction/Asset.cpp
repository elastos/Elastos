// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.
#include "Asset.h"

#include <SDK/Common/Log.h>
#include <SDK/Plugin/Transaction/Transaction.h>

#include <Core/BRCrypto.h>

#include <cstring>

namespace Elastos {
	namespace ElaWallet {

		uint256 Asset::_elaAsset = 0;

		Asset::Asset() :
				_name(""),
				_description(""),
				_precision(0),
				_assetType(AssetType::Share),
				_recordType(AssetRecordType::Unspent) {

		}

		Asset::~Asset() {

		}

		void Asset::SetName(const std::string &name) {
			_name = name;
		}

		std::string Asset::GetName() const {
			return _name;
		}

		void Asset::SetDescription(const std::string &desc) {
			_description = desc;
		}

		std::string Asset::GetDescription() const {
			return _description;
		}

		void Asset::SetAssetType(Asset::AssetType type) {
			_assetType = type;
		}

		Asset::AssetType Asset::GetAssetType() const {
			return _assetType;
		}

		void Asset::SetAssetRecordType(Asset::AssetRecordType type) {
			_recordType = type;
		}

		Asset::AssetRecordType Asset::GetAssetRecordType() const {
			return _recordType;
		}

		void Asset::SetPrecision(uint8_t precision) {
			_precision = precision;
		}

		uint8_t Asset::GetPrecision() const {
			return _precision;
		}

		void Asset::Serialize(ByteStream &ostream) const {
			ostream.WriteVarString(_name);
			ostream.WriteVarString(_description);
			ostream.WriteBytes(&_precision, 1);
			ostream.WriteBytes(&_assetType, 1);
			ostream.WriteBytes(&_recordType, 1);
		}

		bool Asset::Deserialize(const ByteStream &istream) {
			if (!istream.ReadVarString(_name)) {
				Log::error("Asset payload deserialize name fail");
				return false;
			}

			if (!istream.ReadVarString(_description)) {
				Log::error("Asset payload deserialize description fail");
				return false;
			}

			if (!istream.ReadBytes(&_precision, 1)) {
				Log::error("Asset payload deserialize precision fail");
				return false;
			}

			if (!istream.ReadBytes(&_assetType, 1)) {
				Log::error("Asset payload deserialize asset type fail");
				return false;
			}

			if (!istream.ReadBytes(&_recordType, 1)) {
				Log::error("Asset payload deserialize record type fail");
				return false;
			}

			return true;
		}

		nlohmann::json Asset::ToJson() const {
			nlohmann::json j;

			j["Name"] = _name;
			j["Description"] = _description;
			j["Precision"] = _precision;
			j["AssetType"] = _assetType;
			j["RecordType"] = _recordType;

			return j;
		}

		void Asset::FromJson(const nlohmann::json &j) {
			_name = j["Name"].get<std::string>();
			_description = j["Description"].get<std::string>();
			_precision = j["Precision"].get<uint8_t>();
			_assetType = j["AssetType"].get<AssetType>();
			_recordType = j["RecordType"].get<AssetRecordType>();
		}

		const uint256 &Asset::GetELAAssetID() {
			if (_elaAsset == 0) {
				Transaction elaCoin;
				elaCoin.SetTransactionType(Transaction::RegisterAsset);
				_elaAsset = elaCoin.GetHash();
			}
			return _elaAsset;
		}

		uint256 &Asset::GetHash() const {
			if (_hash == 0) {
				ByteStream stream;
				Serialize(stream);
				_hash = sha256_2(stream.GetBytes());
			}
			return _hash;
		}

		void Asset::SetHash(const uint256 &hash) {
			_hash = hash;
		}

	}
}