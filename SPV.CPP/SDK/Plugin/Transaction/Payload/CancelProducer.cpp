// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "CancelProducer.h"
#include <Common/Log.h>
#include <Common/Utils.h>

namespace Elastos {
	namespace ElaWallet {

		CancelProducer::CancelProducer() {

		}

		CancelProducer::CancelProducer(const CancelProducer &payload) {
			operator=(payload);
		}

		CancelProducer::~CancelProducer() {

		}

		const bytes_t &CancelProducer::GetPublicKey() const {
			return _publicKey;
		}

		void CancelProducer::SetPublicKey(const bytes_t &key) {
			_publicKey = key;
		}

		void CancelProducer::SetSignature(const bytes_t &signature) {
			_signature = signature;
		}

		void CancelProducer::SerializeUnsigned(ByteStream &ostream, uint8_t version) const {
			ostream.WriteVarBytes(_publicKey);
		}

		bool CancelProducer::DeserializeUnsigned(const ByteStream &istream, uint8_t version) {
			return istream.ReadVarBytes(_publicKey);
		}

		size_t CancelProducer::EstimateSize(uint8_t version) const {
			size_t size = 0;
			ByteStream stream;

			size += stream.WriteVarUint(_publicKey.size());
			size += _publicKey.size();
			size += stream.WriteVarUint(_signature.size());
			size += _signature.size();

			return size;
		}

		void CancelProducer::Serialize(ByteStream &ostream, uint8_t version) const {
			SerializeUnsigned(ostream, version);
			ostream.WriteVarBytes(_signature);
		}

		bool CancelProducer::Deserialize(const ByteStream &istream, uint8_t version) {
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

		nlohmann::json CancelProducer::ToJson(uint8_t version) const {
			nlohmann::json j;
			j["PublicKey"] = _publicKey.getHex();
			j["Signature"] = _signature.getHex();
			return j;
		}

		void CancelProducer::FromJson(const nlohmann::json &j, uint8_t version) {
			_publicKey.setHex(j["PublicKey"].get<std::string>());
			_signature.setHex(j["Signature"].get<std::string>());
		}

		IPayload &CancelProducer::operator=(const IPayload &payload) {
			try {
				const CancelProducer &payloadCancelProducer = dynamic_cast<const CancelProducer &>(payload);
				operator=(payloadCancelProducer);
			} catch (const std::bad_cast &e) {
				Log::error("payload is not instance of CancelProducer");
			}

			return *this;
		}

		CancelProducer &CancelProducer::operator=(const CancelProducer &payload) {
			_publicKey = payload._publicKey;
			_signature = payload._signature;
			return *this;
		}

	}
}
