// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <SDK/Common/Log.h>
#include "PayloadRegisterIdentification.h"
#include "Utils.h"

namespace Elastos {
	namespace ElaWallet {

		PayloadRegisterIdentification::PayloadRegisterIdentification() :
				_id("") {

		}

		PayloadRegisterIdentification::~PayloadRegisterIdentification() {

		}

		const std::string &PayloadRegisterIdentification::getId() const {
			return _id;
		}

		void PayloadRegisterIdentification::setId(const std::string &id) {
			_id = id;
		}

		const CMBlock &PayloadRegisterIdentification::getSign() const {
			return _sign;
		}

		void PayloadRegisterIdentification::setSign(const CMBlock &sign) {
			_sign = sign;
		}

		bool PayloadRegisterIdentification::isValid() const {
			if (_id.empty() || _contents.empty() || _sign.GetSize() <= 0) {
				return false;
			}
			return true;
		}

		void PayloadRegisterIdentification::Serialize(ByteStream &ostream) const {

			assert(!_id.empty());
			assert(!_contents.empty());
//			assert(_sign.GetSize() > 0);

			ostream.writeVarString(_id);
			ostream.writeVarBytes(_sign);

			ostream.writeVarUint(_contents.size());
			for (size_t i = 0; i < _contents.size(); ++i) {
				ostream.writeVarString(_contents[i].Path);

				ostream.writeVarUint(_contents[i].Values.size());
				for (size_t j = 0; j < _contents[i].Values.size(); ++j) {
					ostream.writeBytes(_contents[i].Values[j].DataHash.u8, sizeof(_contents[i].Values[j].DataHash));
					ostream.writeVarString(_contents[i].Values[j].Proof);
				}
			}
		}

		bool PayloadRegisterIdentification::Deserialize(ByteStream &istream) {
			if (!istream.readVarString(_id)) {
				Log::error("Payload register identification deserialize id fail");
				return false;
			}

			if (!istream.readVarBytes(_sign)) {
				Log::error("Payload register identification deserialize sign fail");
				return false;
			}

			uint64_t size;
			if (!istream.readVarUint(size)) {
				Log::error("Payload register identification deserialize content size fail");
				return false;
			}

			_contents.clear();
			for (uint64_t i = 0; i < size; ++i) {
				SignContent content;

				if (!istream.readVarString(content.Path)) {
					Log::error("Payload register identification deserialize path fail");
					return false;
				}

				uint64_t valueSize;
				if (!istream.readVarUint(valueSize)) {
					Log::error("Payload register identification deserialize value size fail");
					return false;
				}

				for (size_t j = 0; j < valueSize; ++j) {
					ValueItem value;

					if (!istream.readBytes(value.DataHash.u8, sizeof(value.DataHash))) {
						Log::error("Payload register identification deserialize data hash fail");
						return false;
					}

					if (!istream.readVarString(value.Proof)) {
						Log::error("Payload register identification deserialize proof fail");
						return false;
					}
					content.Values.push_back(value);
				}

				_contents.push_back(content);
			}

			return true;
		}

		nlohmann::json PayloadRegisterIdentification::toJson() const {
			nlohmann::json j;
			j["Id"] = _id;
			j["Sign"] = Utils::encodeHex(_sign);

			std::vector<nlohmann::json> contents;
			for (int i = 0; i < _contents.size(); ++i) {
				nlohmann::json content;
				content["Path"] = _contents[i].Path;

				std::vector<nlohmann::json> values;
				for (int k = 0; k < _contents[i].Values.size(); ++k) {
					nlohmann::json value;
					value["DataHash"] = Utils::UInt256ToString(_contents[i].Values[k].DataHash, true);
					value["Proof"] = _contents[i].Values[k].Proof;
					values.push_back(value);
				}
				content["Values"] = values;

				contents.push_back(content);
			}
			j["Contents"] = contents;
			return j;
		}

		void PayloadRegisterIdentification::fromJson(const nlohmann::json &j) {
			_id = j["Id"].get<std::string>();
			if (j.find("Sign") != j.end())
				_sign = Utils::decodeHex(j["Sign"].get<std::string>());

			std::vector<nlohmann::json> contents = j["Contents"];
			_contents.clear();
			for (int i = 0; i < contents.size(); ++i) {

				SignContent content;
				content.Path = contents[i]["Path"].get<std::string>();

				std::vector<nlohmann::json> values = contents[i]["Values"];
				for (int k = 0; k < values.size(); ++k) {
					ValueItem value;
					value.DataHash = Utils::UInt256FromString(values[k]["DataHash"].get<std::string>(), true);
					value.Proof = values[k]["Proof"].get<std::string>();
					content.Values.push_back(value);
				}

				_contents.push_back(content);
			}
		}

		const std::string &PayloadRegisterIdentification::getPath(size_t index) const {
			if (index >= _contents.size())
				throw std::logic_error("Index greater than exiting contents.");

			return _contents[index].Path;
		}

		void PayloadRegisterIdentification::setPath(const std::string &path, size_t index) {
			if (index >= _contents.size())
				throw std::logic_error("Index greater than exiting contents.");

			_contents[index].Path = path;
		}

		const UInt256 &PayloadRegisterIdentification::getDataHash(size_t index, size_t valueIndex) const {
			if (index >= _contents.size())
				throw std::logic_error("Index greater than exiting contents.");

			return _contents[index].Values[valueIndex].DataHash;
		}

		void PayloadRegisterIdentification::setDataHash(const UInt256 &dataHash, size_t index, size_t valueIndex) {
			if (index >= _contents.size())
				throw std::logic_error("Index greater than exiting contents.");

			_contents[index].Values[valueIndex].DataHash = dataHash;
		}

		const std::string &PayloadRegisterIdentification::getProof(size_t index, size_t valueIndex) const {
			if (index >= _contents.size())
				throw std::logic_error("Index greater than exiting contents.");

			return _contents[index].Values[valueIndex].Proof;
		}

		void PayloadRegisterIdentification::setProof(const std::string &proof, size_t index, size_t valueIndex) {
			if (index >= _contents.size())
				throw std::logic_error("Index greater than exiting contents.");

			_contents[index].Values[valueIndex].Proof = proof;
		}

		size_t PayloadRegisterIdentification::getContentCount() const {
			return _contents.size();
		}

		void PayloadRegisterIdentification::addContent(const PayloadRegisterIdentification::SignContent &content) {
			_contents.push_back(content);
		}

		void PayloadRegisterIdentification::removeContent(size_t index) {
			if (index >= _contents.size())
				throw std::logic_error("Index greater than exiting contents.");

			_contents.erase(_contents.begin() + index);
		}
	}
}
