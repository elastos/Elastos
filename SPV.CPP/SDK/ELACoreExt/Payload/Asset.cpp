// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <cstring>
#include <SDK/Common/Log.h>

#include "Asset.h"
#include "BRInt.h"

namespace Elastos {
	namespace ElaWallet {
		Asset::Asset() :
				_name(""),
				_description(""),
				_precision(0),
				_assetType(AssetType::Share),
				_recordType(AssetRecordType::Unspent) {

		}

		Asset::~Asset() {

		}

		void Asset::setName(const std::string &name) {
			_name = name;
		}

		std::string Asset::getName() const {
			return _name;
		}

		void Asset::setDescription(const std::string &desc) {
			_description = desc;
		}

		std::string Asset::getDescription() const {
			return _description;
		}

		void Asset::setAssetType(Asset::AssetType type) {
			_assetType = type;
		}

		Asset::AssetType Asset::getAssetType() const {
			return _assetType;
		}

		void Asset::setAssetRecordType(Asset::AssetRecordType type) {
			_recordType = type;
		}

		Asset::AssetRecordType Asset::getAssetRecordType() const {
			return _recordType;
		}

		void Asset::setPrecision(uint8_t precision) {
			_precision = precision;
		}

		uint8_t Asset::getPrecision() const {
			return _precision;
		}

		void Asset::Serialize(ByteStream &ostream) const {
			ostream.writeVarString(_name);
			ostream.writeVarString(_description);
			ostream.writeBytes(&_precision, 1);
			ostream.writeBytes(&_assetType, 1);
			ostream.writeBytes(&_recordType, 1);
		}

		bool Asset::Deserialize(ByteStream &istream) {
			if (!istream.readVarString(_name)) {
				Log::getLogger()->error("Asset payload deserialize name fail");
				return false;
			}

			if (!istream.readVarString(_description)) {
				Log::getLogger()->error("Asset payload deserialize description fail");
				return false;
			}

			if (!istream.readBytes(&_precision, 1)) {
				Log::getLogger()->error("Asset payload deserialize precision fail");
				return false;
			}

			if (!istream.readBytes(&_assetType, 1)) {
				Log::getLogger()->error("Asset payload deserialize asset type fail");
				return false;
			}

			if (!istream.readBytes(&_recordType, 1)) {
				Log::getLogger()->error("Asset payload deserialize record type fail");
				return false;
			}

			return true;
		}

		nlohmann::json Asset::toJson() const {
			nlohmann::json j;

			j["Name"] = _name;
			j["Description"] = _description;
			j["Precision"] = _precision;
			j["AssetType"] = _assetType;
			j["RecordType"] = _recordType;

			return j;
		}

		void Asset::fromJson(const nlohmann::json &j) {
			_name = j["Name"].get<std::string>();
			_description = j["Description"].get<std::string>();
			_precision = j["Precision"].get<uint8_t>();
			_assetType = j["AssetType"].get<AssetType>();
			_recordType = j["RecordType"].get<AssetRecordType>();
		}
	}
}