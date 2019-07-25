// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "UnregisterCR.h"
#include <SDK/Common/Log.h>

namespace Elastos {
	namespace ElaWallet {
		UnregisterCR::UnregisterCR() {

		}

		UnregisterCR::~UnregisterCR() {

		}

		void UnregisterCR::SetCode(const bytes_t &code) {
			_code = code;
		}

		const bytes_t &UnregisterCR::GetCode() const {
			return _code;
		}

		void UnregisterCR::SetSignature(const bytes_t &signature) {
			_signature = signature;
		}

		const bytes_t &UnregisterCR::GetSignature() const {
			return _signature;
		}

		size_t UnregisterCR::EstimateSize(uint8_t version) const {
			size_t size = 0;
			ByteStream stream;

			size += stream.WriteVarUint(_code.size());
			size += _code.size();
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
			ostream.WriteVarBytes(_code);
		}

		bool UnregisterCR::DeserializeUnsigned(const ByteStream &istream, uint8_t version) {
			if (!istream.ReadVarBytes(_code)) {
				Log::error("UnregisterCR Deserialize: read _code");
				return false;
			}
			return true;
		}

		nlohmann::json UnregisterCR::ToJson(uint8_t version) const {
			nlohmann::json j;
			j["Code"] = _code.getHex();
			j["Signature"] = _signature.getHex();
			return j;
		}

		void UnregisterCR::FromJson(const nlohmann::json &j, uint8_t version) {
			std::string code = j["Code"].get<std::string>();
			_code.setHex(code);

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
			_code = payload._code;
			_signature = payload._signature;
			return *this;
		}
	}
}