// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "RegisterIdentification.h"
#include <Common/Log.h>
#include <Common/ErrorChecker.h>
#include <Common/Utils.h>

namespace Elastos {
	namespace ElaWallet {

		RegisterIdentification::RegisterIdentification() :
				_id("") {

		}

		RegisterIdentification::RegisterIdentification(const RegisterIdentification &payload) {
			operator=(payload);
		}

		RegisterIdentification::~RegisterIdentification() {

		}

		const std::string &RegisterIdentification::GetID() const {
			return _id;
		}

		void RegisterIdentification::SetID(const std::string &id) {
			_id = id;
		}

		const bytes_t &RegisterIdentification::GetSign() const {
			return _sign;
		}

		void RegisterIdentification::SetSign(const bytes_t &sign) {
			_sign = sign;
		}

		bool RegisterIdentification::IsValid(uint8_t version) const {
			if (_id.empty() || _contents.empty() || _sign.size() <= 0) {
				return false;
			}
			return true;
		}

		size_t RegisterIdentification::EstimateSize(uint8_t version) const {
			size_t size = 0;
			ByteStream stream;

			size += stream.WriteVarUint(_id.size());
			size += _id.size();
			size += stream.WriteVarUint(_sign.size());
			size += _sign.size();

			size += stream.WriteVarUint(_contents.size());
			for (size_t i = 0; i < _contents.size(); ++i) {
				size += stream.WriteVarUint(_contents[i].Path.size());
				size += _contents[i].Path.size();

				size += stream.WriteVarUint(_contents[i].Values.size());
				for (size_t j = 0; j < _contents[i].Values.size(); ++j) {
					size += _contents[i].Values[j].DataHash.size();
					size += stream.WriteVarUint(_contents[i].Values[j].Proof.size());
					size += _contents[i].Values[j].Proof.size();
					size += stream.WriteVarUint(_contents[i].Values[j].Info.size());
					size += _contents[i].Values[j].Info.size();
				}
			}

			return size;
		}

		void RegisterIdentification::Serialize(ByteStream &ostream, uint8_t version) const {

			assert(!_id.empty());
			assert(!_contents.empty());

			ostream.WriteVarString(_id);
			ostream.WriteVarBytes(_sign);

			ostream.WriteVarUint(_contents.size());
			for (size_t i = 0; i < _contents.size(); ++i) {
				ostream.WriteVarString(_contents[i].Path);

				ostream.WriteVarUint(_contents[i].Values.size());
				for (size_t j = 0; j < _contents[i].Values.size(); ++j) {
					ostream.WriteBytes(_contents[i].Values[j].DataHash);
					ostream.WriteVarString(_contents[i].Values[j].Proof);
					ostream.WriteVarString(_contents[i].Values[j].Info);
				}
			}
		}

		bool RegisterIdentification::Deserialize(const ByteStream &istream, uint8_t version) {
			if (!istream.ReadVarString(_id)) {
				Log::error("Payload register identification deserialize id fail");
				return false;
			}

			if (!istream.ReadVarBytes(_sign)) {
				Log::error("Payload register identification deserialize sign fail");
				return false;
			}

			uint64_t size;
			if (!istream.ReadVarUint(size)) {
				Log::error("Payload register identification deserialize content size fail");
				return false;
			}

			_contents.clear();
			for (uint64_t i = 0; i < size; ++i) {
				SignContent content;

				if (!istream.ReadVarString(content.Path)) {
					Log::error("Payload register identification deserialize path fail");
					return false;
				}

				uint64_t valueSize;
				if (!istream.ReadVarUint(valueSize)) {
					Log::error("Payload register identification deserialize value size fail");
					return false;
				}

				for (size_t j = 0; j < valueSize; ++j) {
					ValueItem value;

					if (!istream.ReadBytes(value.DataHash)) {
						Log::error("Payload register identification deserialize data hash fail");
						return false;
					}

					if (!istream.ReadVarString(value.Proof)) {
						Log::error("Payload register identification deserialize proof fail");
						return false;
					}

					if (!istream.ReadVarString(value.Info)) {
						Log::error("Payload register identification deserialize info fail");
						return false;
					}
					content.Values.push_back(value);
				}

				_contents.push_back(content);
			}

			return true;
		}

		nlohmann::json RegisterIdentification::ToJson(uint8_t version) const {
			nlohmann::json j;
			j["Id"] = _id;
			j["Sign"] = _sign.getHex();

			std::vector<nlohmann::json> contents;
			for (int i = 0; i < _contents.size(); ++i) {
				nlohmann::json content;
				content["Path"] = _contents[i].Path;

				std::vector<nlohmann::json> values;
				for (int k = 0; k < _contents[i].Values.size(); ++k) {
					nlohmann::json value;
					value["DataHash"] = _contents[i].Values[k].DataHash.GetHex();
					value["Proof"] = _contents[i].Values[k].Proof;
					value["Info"] = _contents[i].Values[k].Info;
					values.push_back(value);
				}
				content["Values"] = values;

				contents.push_back(content);
			}
			j["Contents"] = contents;
			return j;
		}

		void RegisterIdentification::FromJson(const nlohmann::json &j, uint8_t version) {
			_id = j["Id"].get<std::string>();
			if (j.find("Sign") != j.end())
				_sign.setHex(j["Sign"].get<std::string>());

			std::vector<nlohmann::json> contents = j["Contents"];
			_contents.clear();
			for (int i = 0; i < contents.size(); ++i) {

				SignContent content;
				content.Path = contents[i]["Path"].get<std::string>();

				std::vector<nlohmann::json> values = contents[i]["Values"];
				for (int k = 0; k < values.size(); ++k) {
					ValueItem value;
					value.DataHash.SetHex(values[k]["DataHash"].get<std::string>());
					value.Proof = values[k]["Proof"].get<std::string>();
					if (values[k].find("Info") != values[k].end())
						value.Info = values[k]["Info"].get<std::string>();
					content.Values.push_back(value);
				}

				_contents.push_back(content);
			}
		}

		const std::string &RegisterIdentification::GetPath(size_t index) const {
			ErrorChecker::CheckCondition(index >= _contents.size(), Error::PayloadRegisterID, "Index too large");

			return _contents[index].Path;
		}

		void RegisterIdentification::SetPath(const std::string &path, size_t index) {
			ErrorChecker::CheckCondition(index >= _contents.size(), Error::PayloadRegisterID, "Index too large");

			_contents[index].Path = path;
		}

		const uint256 &RegisterIdentification::GetDataHash(size_t index, size_t valueIndex) const {
			ErrorChecker::CheckCondition(index >= _contents.size(), Error::PayloadRegisterID, "Index too large");

			return _contents[index].Values[valueIndex].DataHash;
		}

		void RegisterIdentification::SetDataHash(const uint256 &dataHash, size_t index, size_t valueIndex) {
			ErrorChecker::CheckCondition(index >= _contents.size(), Error::PayloadRegisterID, "Index too large");

			_contents[index].Values[valueIndex].DataHash = dataHash;
		}

		const std::string &RegisterIdentification::GetProof(size_t index, size_t valueIndex) const {
			ErrorChecker::CheckCondition(index >= _contents.size(), Error::PayloadRegisterID, "Index too large");

			return _contents[index].Values[valueIndex].Proof;
		}

		void RegisterIdentification::SetProof(const std::string &proof, size_t index, size_t valueIndex) {
			ErrorChecker::CheckCondition(index >= _contents.size(), Error::PayloadRegisterID, "Index too large");

			_contents[index].Values[valueIndex].Proof = proof;
		}

		size_t RegisterIdentification::GetContentCount() const {
			return _contents.size();
		}

		void RegisterIdentification::AddContent(const RegisterIdentification::SignContent &content) {
			_contents.push_back(content);
		}

		void RegisterIdentification::RemoveContent(size_t index) {
			ErrorChecker::CheckCondition(index >= _contents.size(), Error::PayloadRegisterID, "Index too large");

			_contents.erase(_contents.begin() + index);
		}

		IPayload &RegisterIdentification::operator=(const IPayload &payload) {
			try {
				const RegisterIdentification &payloadRegisterIdentification = dynamic_cast<const RegisterIdentification &>(payload);
				operator=(payloadRegisterIdentification);
			} catch (const std::bad_cast &e) {
				Log::error("payload is not instance of RegisterIdentification");
			}
			return *this;
		}

		RegisterIdentification &RegisterIdentification::operator=(const RegisterIdentification &payload) {
			_id = payload._id;
			_sign = payload._sign;
			_contents = payload._contents;

			return *this;
		}

	}
}
