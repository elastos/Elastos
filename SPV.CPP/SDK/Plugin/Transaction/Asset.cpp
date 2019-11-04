// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.
#include "Asset.h"

#include <Common/Log.h>
#include <Common/hash.h>

#include <cstring>

namespace Elastos {
	namespace ElaWallet {

		uint256 Asset::_elaAsset = 0;

		Asset::Asset() :
			Asset("ELA", "", 8, AssetType::Token, AssetRecordType::Unspent) {
			_hash = GetELAAssetID();
		}

		Asset::Asset(const std::string &name, const std::string &desc, uint8_t precision,
					 AssetType assetType, AssetRecordType recordType) :
				_name(name),
				_description(desc),
				_precision(precision),
				_assetType(assetType),
				_recordType(recordType) {

		}

		Asset::Asset(const Asset &asset) {
			this->operator=(asset);
		}

		Asset::~Asset() {

		}

		Asset &Asset::operator=(const Asset &asset) {
			this->_name = asset._name;
			this->_description = asset._description;
			this->_precision = asset._precision;
			this->_assetType = asset._assetType;
			this->_recordType = asset._recordType;
			this->_hash = asset._hash;
			return *this;
		}

		size_t Asset::EstimateSize() const {
			size_t size = 0;
			ByteStream stream;

			size += stream.WriteVarUint(_name.size());
			size += _name.size();
			size += stream.WriteVarUint(_description.size());
			size += _description.size();
			size += 3;

			return size;
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

			if (_name == "ELA") {
				_hash = Asset::GetELAAssetID();
			} else {
				_hash = 0;
				GetHash();
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

			if (_name == "ELA") {
				_hash = Asset::GetELAAssetID();
			} else {
				_hash = 0;
				GetHash();
			}
		}

		const uint256 &Asset::GetELAAssetID() {
			if (_elaAsset == 0) {
				_elaAsset = uint256("a3d0eaa466df74983b5d7c543de6904f4c9418ead5ffd6d25814234a96db37b0");
			}
			return _elaAsset;
		}

		const uint256 &Asset::GetHash() const {
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