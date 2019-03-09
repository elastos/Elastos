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

		void PayloadCancelProducer::SetSignature(const CMBlock &signature) {
			_signature = signature;
		}

		void PayloadCancelProducer::SerializeUnsigned(ByteStream &ostream, uint8_t version) const {
			ostream.WriteVarBytes(_publicKey);
		}

		bool PayloadCancelProducer::DeserializeUnsigned(ByteStream &istream, uint8_t version) {
			return istream.ReadVarBytes(_publicKey);
		}

		void PayloadCancelProducer::Serialize(ByteStream &ostream, uint8_t version) const {
			SerializeUnsigned(ostream, version);
			ostream.WriteVarBytes(_signature);
		}

		bool PayloadCancelProducer::Deserialize(ByteStream &istream, uint8_t version) {
			if (!DeserializeUnsigned(istream, version)) {
				Log::error("Deserialize: cancel producer payload read unsigned");
				return false;
			}

			if (!istream.ReadVarBytes(_signature)) {
				Log::error("Deserialize: cancel producer payload read signature");
				return false;
			}

			return true;
		}

		nlohmann::json PayloadCancelProducer::ToJson(uint8_t version) const {
			nlohmann::json j;
			j["PublicKey"] = Utils::EncodeHex(_publicKey);
			j["Signature"] = Utils::EncodeHex(_signature);
			return j;
		}

		void PayloadCancelProducer::FromJson(const nlohmann::json &j, uint8_t version) {
			_publicKey = Utils::DecodeHex(j["PublicKey"].get<std::string>());
			_signature = Utils::DecodeHex(j["Signature"].get<std::string>());
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
			_signature.Memcpy(payload._signature);
			return *this;
		}

	}
}
