// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "UnregisterCR.h"
#include <Common/Log.h>

namespace Elastos {
	namespace ElaWallet {
		UnregisterCR::UnregisterCR() {

		}

		UnregisterCR::~UnregisterCR() {

		}

		void UnregisterCR::SetDID(const uint168 &did) {
			_did = did;
		}

		const uint168 &UnregisterCR::GetDID() const {
			return _did;
		}

		void UnregisterCR::SetSignature(const bytes_t &signature) {
			_signature = signature;
		}

		const bytes_t &UnregisterCR::GetSignature() const {
			return _signature;
		}

		size_t UnregisterCR::EstimateSize(uint8_t version) const {
			size_t size = 0;
			size += _did.size();

			ByteStream stream;
			size += stream.WriteVarUint(_signature.size());
			size += _signature.size();

			return size;
		}

		void UnregisterCR::Serialize(ByteStream &ostream, uint8_t version) const {
			SerializeUnsigned(ostream, version);
			ostream.WriteVarBytes(_signature);
		}

		bool UnregisterCR::Deserialize(const ByteStream &istream, uint8_t version) {
			if (!DeserializeUnsigned(istream, version)) {
				return false;
			}

			if (!istream.ReadVarBytes(_signature)) {
				Log::error("UnregisterCR Deserialize: read _signature");
				return false;
			}

			return true;
		}

		void UnregisterCR::SerializeUnsigned(ByteStream &ostream, uint8_t version) const {
			ostream.WriteBytes(_did);
		}

		bool UnregisterCR::DeserializeUnsigned(const ByteStream &istream, uint8_t version) {
			if (!istream.ReadBytes(_did)) {
				Log::error("UnregisterCR Deserialize: read _did");
				return false;
			}
			return true;
		}

		nlohmann::json UnregisterCR::ToJson(uint8_t version) const {
			nlohmann::json j;
			j["DID"] = _did.GetHex();
			j["Signature"] = _signature.getHex();
			return j;
		}

		void UnregisterCR::FromJson(const nlohmann::json &j, uint8_t version) {
			std::string did = j["DID"].get<std::string>();
			_did.SetHex(did);

			std::string signature = j["Signature"].get<std::string>();
			_signature.setHex(signature);
		}

		IPayload &UnregisterCR::operator=(const IPayload &payload) {
			try {
				const UnregisterCR &unregisterCR = dynamic_cast<const UnregisterCR &>(payload);
				operator=(unregisterCR);
			} catch (const std::bad_cast &e) {
				Log::error("payload is not instance of UnregisterCR");
			}

			return *this;
		}

		UnregisterCR &UnregisterCR::operator=(const UnregisterCR &payload) {
			_did = payload._did;
			_signature = payload._signature;
			return *this;
		}
	}
}