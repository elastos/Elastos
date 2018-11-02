// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "PayloadRegisterProducer.h"

namespace Elastos {
	namespace ElaWallet {

		PayloadRegisterProducer::PayloadRegisterProducer() {

		}

		PayloadRegisterProducer::~PayloadRegisterProducer() {

		}

		const std::string &PayloadRegisterProducer::GetPublicKey() const {
			return _publicKey;
		}

		void PayloadRegisterProducer::SetPublicKey(const std::string &key) {
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

		void PayloadRegisterProducer::Serialize(ByteStream &ostream) const {
			ostream.writeVarString(_publicKey);
			ostream.writeVarString(_nickName);
			ostream.writeVarString(_url);
			ostream.writeUint64(_location);
		}

		bool PayloadRegisterProducer::Deserialize(ByteStream &istream) {
			return !(!istream.readVarString(_publicKey) ||
					 !istream.readVarString(_nickName) ||
					 !istream.readVarString(_url) ||
					 !istream.readUint64(_location));
		}

		nlohmann::json PayloadRegisterProducer::toJson() const {
			nlohmann::json j;
			j["PublicKey"] = _publicKey;
			j["NickName"] = _nickName;
			j["Url"] = _url;
			j["Location"] = _location;
			return j;
		}

		void PayloadRegisterProducer::fromJson(const nlohmann::json &j) {
			_publicKey = j["PublicKey"].get<std::string>();
			_nickName = j["NickName"].get<std::string>();
			_url = j["Url"].get<std::string>();
			_location = j["Location"].get<std::string>();
		}
	}
}
