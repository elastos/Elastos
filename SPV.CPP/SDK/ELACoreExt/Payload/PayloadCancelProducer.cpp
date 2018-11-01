// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "PayloadCancelProducer.h"

namespace Elastos {
	namespace ElaWallet {

		PayloadCancelProducer::PayloadCancelProducer() {

		}

		PayloadCancelProducer::~PayloadCancelProducer() {

		}

		const std::string &PayloadCancelProducer::GetPublicKey() const {
			return _publicKey;
		}

		void PayloadCancelProducer::SetPublicKey(const std::string &key) {
			_publicKey = key;
		}

		void PayloadCancelProducer::Serialize(ByteStream &ostream) const {
			ostream.writeVarString(_publicKey);
		}

		bool PayloadCancelProducer::Deserialize(ByteStream &istream) {
			return istream.readVarString(_publicKey);
		}

		nlohmann::json PayloadCancelProducer::toJson() const {
			nlohmann::json j;
			j["PublicKey"] = _publicKey;
			return j;
		}

		void PayloadCancelProducer::fromJson(const nlohmann::json &j) {
			_publicKey = j["PublicKey"].get<std::string>();
		}
	}
}
