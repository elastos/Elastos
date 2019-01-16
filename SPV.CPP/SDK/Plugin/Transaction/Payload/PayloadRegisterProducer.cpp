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

		const CMBlock &PayloadRegisterProducer::GetPublicKey() const {
			return _publicKey;
		}

		void PayloadRegisterProducer::SetPublicKey(const CMBlock &key) {
			_publicKey = key;
		}

		const CMBlock &PayloadRegisterProducer::GetNodePublicKey() const {
			return _nodePublicKey;
		}

		void PayloadRegisterProducer::SetNodePublicKey(const CMBlock &key) {
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

		const CMBlock &PayloadRegisterProducer::GetSignature() const {
			return _signature;
		}

		void PayloadRegisterProducer::SetSignature(const CMBlock &signature) {
			_signature = signature;
		}

		void PayloadRegisterProducer::SerializeUnsigned(ByteStream &ostream, uint8_t version) const {
			ostream.writeVarBytes(_publicKey);
			ostream.writeVarBytes(_nodePublicKey);
			ostream.writeVarString(_nickName);
			ostream.writeVarString(_url);
			ostream.writeUint64(_location);
			ostream.writeVarString(_address);
		}

		bool PayloadRegisterProducer::DeserializeUnsigned(ByteStream &istream, uint8_t version) {
			if (!istream.readVarBytes(_publicKey)) {
				Log::error("Deserialize: read public key");
				return false;
			}
			if (!istream.readVarBytes(_nodePublicKey)) {
				Log::error("Deserialize: read node public key");
				return false;
			}
			if (!istream.readVarString(_nickName)) {
				Log::error("Deserialize: read nick name");
				return false;
			}
			if (!istream.readVarString(_url)) {
				Log::error("Deserialize: read url");
				return false;
			}
			if (!istream.readUint64(_location)) {
				Log::error("Deserialize: read location");
				return false;
			}
			if (!istream.readVarString(_address)) {
				Log::error("Deserialize: read address");
				return false;
			}

			return true;
		}

		void PayloadRegisterProducer::Serialize(ByteStream &ostream, uint8_t version) const {
			SerializeUnsigned(ostream, version);
			ostream.writeVarBytes(_signature);
		}

		bool PayloadRegisterProducer::Deserialize(ByteStream &istream, uint8_t version) {
			if (!DeserializeUnsigned(istream, version)) {
				Log::error("Deserialize: register producer payload unsigned");
				return false;
			}

			if (!istream.readVarBytes(_signature)) {
				Log::error("Deserialize: register producer payload read signature");
				return false;
			}

			return true;
		}

		nlohmann::json PayloadRegisterProducer::toJson(uint8_t version) const {
			nlohmann::json j;
			j["PublicKey"] = Utils::encodeHex(_publicKey);
			j["NickName"] = _nickName;
			j["Url"] = _url;
			j["Location"] = _location;
			j["Address"] = _address;
			j["Signature"] = Utils::encodeHex(_signature);
			return j;
		}

		void PayloadRegisterProducer::fromJson(const nlohmann::json &j, uint8_t version) {
			_publicKey = Utils::decodeHex(j["PublicKey"].get<std::string>());
			_nickName = j["NickName"].get<std::string>();
			_url = j["Url"].get<std::string>();
			_location = j["Location"].get<uint64_t>();
			_address = j["Address"].get<std::string>();
			_signature = Utils::decodeHex(j["Signature"].get<std::string>());
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
			_publicKey.Memcpy(payload._publicKey);
			_nickName = payload._nickName;
			_url = payload._url;
			_location = payload._location;
			_address = payload._address;
			_signature.Memcpy(payload._signature);
			return *this;
		}

	}
}
