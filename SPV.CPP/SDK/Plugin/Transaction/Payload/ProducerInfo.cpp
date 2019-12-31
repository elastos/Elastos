// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "ProducerInfo.h"

#include <Common/Utils.h>
#include <Common/Log.h>

namespace Elastos {
	namespace ElaWallet {

		ProducerInfo::ProducerInfo() {

		}

		ProducerInfo::ProducerInfo(const ProducerInfo &payload) {
			operator=(payload);
		}

		ProducerInfo::~ProducerInfo() {

		}

		const bytes_t &ProducerInfo::GetPublicKey() const {
			return _ownerPublicKey;
		}

		void ProducerInfo::SetPublicKey(const bytes_t &key) {
			_ownerPublicKey = key;
		}

		const bytes_t &ProducerInfo::GetNodePublicKey() const {
			return _nodePublicKey;
		}

		void ProducerInfo::SetNodePublicKey(const bytes_t &key) {
			_nodePublicKey = key;
		}

		const std::string &ProducerInfo::GetNickName() const {
			return _nickName;
		}

		void ProducerInfo::SetNickName(const std::string &name) {
			_nickName = name;
		}

		const std::string &ProducerInfo::GetUrl() const {
			return _url;
		}

		void ProducerInfo::SetUrl(const std::string &url) {
			_url = url;
		}

		uint64_t ProducerInfo::GetLocation() const {
			return _location;
		}

		void ProducerInfo::SetLocation(uint64_t location) {
			_location = location;
		}

		const std::string &ProducerInfo::GetAddress() const {
			return _address;
		}

		void ProducerInfo::SetAddress(const std::string &address) {
			_address = address;
		}

		const bytes_t &ProducerInfo::GetSignature() const {
			return _signature;
		}

		void ProducerInfo::SetSignature(const bytes_t &signature) {
			_signature = signature;
		}

		void ProducerInfo::SerializeUnsigned(ByteStream &ostream, uint8_t version) const {
			ostream.WriteVarBytes(_ownerPublicKey);
			ostream.WriteVarBytes(_nodePublicKey);
			ostream.WriteVarString(_nickName);
			ostream.WriteVarString(_url);
			ostream.WriteUint64(_location);
			ostream.WriteVarString(_address);
		}

		bool ProducerInfo::DeserializeUnsigned(const ByteStream &istream, uint8_t version) {
			if (!istream.ReadVarBytes(_ownerPublicKey)) {
				Log::error("Deserialize: read public key");
				return false;
			}
			if (!istream.ReadVarBytes(_nodePublicKey)) {
				Log::error("Deserialize: read node public key");
				return false;
			}
			if (!istream.ReadVarString(_nickName)) {
				Log::error("Deserialize: read nick name");
				return false;
			}
			if (!istream.ReadVarString(_url)) {
				Log::error("Deserialize: read url");
				return false;
			}
			if (!istream.ReadUint64(_location)) {
				Log::error("Deserialize: read location");
				return false;
			}
			if (!istream.ReadVarString(_address)) {
				Log::error("Deserialize: read address");
				return false;
			}

			return true;
		}

		size_t ProducerInfo::EstimateSize(uint8_t version) const {
			size_t size = 0;
			ByteStream stream;

			size += stream.WriteVarUint(_ownerPublicKey.size());
			size += _ownerPublicKey.size();
			size += stream.WriteVarUint(_nodePublicKey.size());
			size += _nodePublicKey.size();
			size += stream.WriteVarUint(_nickName.size());
			size += _nickName.size();
			size += stream.WriteVarUint(_url.size());
			size += _url.size();
			size += sizeof(_location);
			size += stream.WriteVarUint(_address.size());
			size += _address.size();
			size += stream.WriteVarUint(_signature.size());
			size += _signature.size();

			return size;
		}

		void ProducerInfo::Serialize(ByteStream &ostream, uint8_t version) const {
			SerializeUnsigned(ostream, version);
			ostream.WriteVarBytes(_signature);
		}

		bool ProducerInfo::Deserialize(const ByteStream &istream, uint8_t version) {
			if (!DeserializeUnsigned(istream, version)) {
				Log::error("Deserialize: register producer payload unsigned");
				return false;
			}

			if (!istream.ReadVarBytes(_signature)) {
				Log::error("Deserialize: register producer payload read signature");
				return false;
			}

			return true;
		}

		nlohmann::json ProducerInfo::ToJson(uint8_t version) const {
			nlohmann::json j;
			j["OwnerPublicKey"] = _ownerPublicKey.getHex();
			j["NodePublicKey"] = _nodePublicKey.getHex();
			j["NickName"] = _nickName;
			j["Url"] = _url;
			j["Location"] = _location;
			j["Address"] = _address;
			j["Signature"] = _signature.getHex();
			return j;
		}

		void ProducerInfo::FromJson(const nlohmann::json &j, uint8_t version) {
			_ownerPublicKey.setHex(j["OwnerPublicKey"].get<std::string>());
			_nodePublicKey.setHex(j["NodePublicKey"].get<std::string>());
			_nickName = j["NickName"].get<std::string>();
			_url = j["Url"].get<std::string>();
			_location = j["Location"].get<uint64_t>();
			_address = j["Address"].get<std::string>();
			_signature.setHex(j["Signature"].get<std::string>());
		}

		IPayload &ProducerInfo::operator=(const IPayload &payload) {
			try {
				const ProducerInfo &payloadRegisterProducer = dynamic_cast<const ProducerInfo &>(payload);
				operator=(payloadRegisterProducer);
			} catch (const std::bad_cast &e) {
				Log::error("payload is not instance of ProducerInfo");
			}

			return *this;
		}

		ProducerInfo &ProducerInfo::operator=(const ProducerInfo &payload) {
			_ownerPublicKey = payload._ownerPublicKey;
			_nodePublicKey = payload._nodePublicKey;
			_nickName = payload._nickName;
			_url = payload._url;
			_location = payload._location;
			_address = payload._address;
			_signature = payload._signature;
			return *this;
		}

	}
}
