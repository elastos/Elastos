// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "PayloadCancelProducer.h"
#include <SDK/Common/Log.h>
#include <SDK/Common/Utils.h>

namespace Elastos {
	namespace ElaWallet {

		PayloadCancelProducer::PayloadCancelProducer() {

		}

		PayloadCancelProducer::PayloadCancelProducer(const PayloadCancelProducer &payload) {
			operator=(payload);
		}

		PayloadCancelProducer::~PayloadCancelProducer() {

		}

		const CMBlock &PayloadCancelProducer::GetPublicKey() const {
			return _publicKey;
		}

		void PayloadCancelProducer::SetPublicKey(const CMBlock &key) {
			_publicKey = key;
		}

		void PayloadCancelProducer::Serialize(ByteStream &ostream, uint8_t version) const {
			ostream.writeVarBytes(_publicKey);
		}

		bool PayloadCancelProducer::Deserialize(ByteStream &istream, uint8_t version) {
			return istream.readVarBytes(_publicKey);
		}

		nlohmann::json PayloadCancelProducer::toJson() const {
			nlohmann::json j;
			j["PublicKey"] = Utils::encodeHex(_publicKey);
			return j;
		}

		void PayloadCancelProducer::fromJson(const nlohmann::json &j) {
			_publicKey = Utils::decodeHex(j["PublicKey"].get<std::string>());
		}

		IPayload &PayloadCancelProducer::operator=(const IPayload &payload) {
			try {
				const PayloadCancelProducer &payloadCancelProducer = dynamic_cast<const PayloadCancelProducer &>(payload);
				operator=(payloadCancelProducer);
			} catch (const std::bad_cast &e) {
				Log::error("payload is not instance of PayloadCancelProducer");
			}

			return *this;
		}

		PayloadCancelProducer &PayloadCancelProducer::operator=(const PayloadCancelProducer &payload) {
			_publicKey.Memcpy(payload._publicKey);
			return *this;
		}

	}
}
