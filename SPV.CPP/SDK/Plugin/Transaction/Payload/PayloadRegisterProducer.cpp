// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "PayloadRegisterProducer.h"

#include <SDK/Common/Utils.h>
#include <SDK/Common/Log.h>

namespace Elastos {
	namespace ElaWallet {

		PayloadRegisterProducer::PayloadRegisterProducer() {

		}

		PayloadRegisterProducer::PayloadRegisterProducer(const PayloadRegisterProducer &payload) {
			operator=(payload);
		}

		PayloadRegisterProducer::~PayloadRegisterProducer() {

		}

		const bytes_t &PayloadRegisterProducer::GetPublicKey() const {
			return _ownerPublicKey;
		}

		void PayloadRegisterProducer::SetPublicKey(const bytes_t &key) {
			_ownerPublicKey = key;
		}

		const bytes_t &PayloadRegisterProducer::GetNodePublicKey() const {
			return _nodePublicKey;
		}

		void PayloadRegisterProducer::SetNodePublicKey(const bytes_t &key) {
			_nodePublicKey = key;
		}

		const std::string &PayloadRegisterProducer::GetNickName() const {
			return _nickName;
		}

		void PayloadRegisterProducer::SetNickName(const std::string &name) {
			_nickName = name;
		}

		const std::string &PayloadRegisterProducer::GetUrl() const {
			return _url;
		}

		void PayloadRegisterProducer::SetUrl(const std::string &url) {
			_url = url;
		}

		uint64_t PayloadRegisterProducer::GetLocation() const {
			return _location;
		}

		void PayloadRegisterProducer::SetLocation(uint64_t location) {
			_location = location;
		}

		const std::string &PayloadRegisterProducer::GetAddress() const {
			return _address;
		}

		void PayloadRegisterProducer::SetAddress(const std::string &address) {
			_address = address;
		}

		const bytes_t &PayloadRegisterProducer::GetSignature() const {
			return _signature;
		}

		void PayloadRegisterProducer::SetSignature(const bytes_t &signature) {
			_signature = signature;
		}

		void PayloadRegisterProducer::SerializeUnsigned(ByteStream &ostream, uint8_t version) const {
			ostream.WriteVarBytes(_ownerPublicKey);
			ostream.WriteVarBytes(_nodePublicKey);
			ostream.WriteVarString(_nickName);
			ostream.WriteVarString(_url);
			ostream.WriteUint64(_location);
			ostream.WriteVarString(_address);
		}

		bool PayloadRegisterProducer::DeserializeUnsigned(const ByteStream &istream, uint8_t version) {
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

		void PayloadRegisterProducer::Serialize(ByteStream &ostream, uint8_t version) const {
			SerializeUnsigned(ostream, version);
			ostream.WriteVarBytes(_signature);
		}

		bool PayloadRegisterProducer::Deserialize(const ByteStream &istream, uint8_t version) {
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

		nlohmann::json PayloadRegisterProducer::ToJson(uint8_t version) const {
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

		void PayloadRegisterProducer::FromJson(const nlohmann::json &j, uint8_t version) {
			_ownerPublicKey.setHex(j["OwnerPublicKey"].get<std::string>());
			_nodePublicKey.setHex(j["NodePublicKey"].get<std::string>());
			_nickName = j["NickName"].get<std::string>();
			_url = j["Url"].get<std::string>();
			_location = j["Location"].get<uint64_t>();
			_address = j["Address"].get<std::string>();
			_signature.setHex(j["Signature"].get<std::string>());
		}

		IPayload &PayloadRegisterProducer::operator=(const IPayload &payload) {
			try {
				const PayloadRegisterProducer &payloadRegisterProducer = dynamic_cast<const PayloadRegisterProducer &>(payload);
				operator=(payloadRegisterProducer);
			} catch (const std::bad_cast &e) {
				Log::error("payload is not instance of PayloadRegisterProducer");
			}

			return *this;
		}

		PayloadRegisterProducer &PayloadRegisterProducer::operator=(const PayloadRegisterProducer &payload) {
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
