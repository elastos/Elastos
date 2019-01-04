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

		void PayloadRegisterProducer::Serialize(ByteStream &ostream, uint8_t version) const {
			ostream.writeVarBytes(_publicKey);
			ostream.writeVarString(_nickName);
			ostream.writeVarString(_url);
			ostream.writeUint64(_location);
			ostream.writeVarString(_address);
		}

		bool PayloadRegisterProducer::Deserialize(ByteStream &istream, uint8_t version) {
			return !(!istream.readVarBytes(_publicKey) ||
					 !istream.readVarString(_nickName) ||
					 !istream.readVarString(_url) ||
					 !istream.readUint64(_location) ||
					 !istream.readVarString(_address));
		}

		nlohmann::json PayloadRegisterProducer::toJson() const {
			nlohmann::json j;
			j["PublicKey"] = Utils::encodeHex(_publicKey);
			j["NickName"] = _nickName;
			j["Url"] = _url;
			j["Location"] = _location;
			j["Address"] = _address;
			return j;
		}

		void PayloadRegisterProducer::fromJson(const nlohmann::json &j) {
			_publicKey = Utils::decodeHex(j["PublicKey"].get<std::string>());
			_nickName = j["NickName"].get<std::string>();
			_url = j["Url"].get<std::string>();
			_location = j["Location"].get<uint64_t>();
			_address = j["Address"].get<std::string>();
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
			return *this;
		}

	}
}
