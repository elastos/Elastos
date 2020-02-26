// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "UnregisterCR.h"
#include <Common/Log.h>
#include <WalletCore/Address.h>

namespace Elastos {
	namespace ElaWallet {
		UnregisterCR::UnregisterCR() {

		}

		UnregisterCR::~UnregisterCR() {

		}

		void UnregisterCR::SetCID(const uint168 &cid) {
			_cid = cid;
		}

		const uint168 &UnregisterCR::GetCID() const {
			return _cid;
		}

		void UnregisterCR::SetSignature(const bytes_t &signature) {
			_signature = signature;
		}

		const bytes_t &UnregisterCR::GetSignature() const {
			return _signature;
		}

		size_t UnregisterCR::EstimateSize(uint8_t version) const {
			size_t size = 0;
			size += _cid.size();

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
			ostream.WriteBytes(_cid);
		}

		bool UnregisterCR::DeserializeUnsigned(const ByteStream &istream, uint8_t version) {
			if (!istream.ReadBytes(_cid)) {
				Log::error("UnregisterCR Deserialize: read _did");
				return false;
			}
			return true;
		}

		nlohmann::json UnregisterCR::ToJson(uint8_t version) const {
			nlohmann::json j;
			j["CID"] = Address(_cid).String();
			j["Signature"] = _signature.getHex();
			return j;
		}

		void UnregisterCR::FromJson(const nlohmann::json &j, uint8_t version) {
			std::string cid = j["CID"].get<std::string>();
			_cid = Address(cid).ProgramHash();

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
			_cid = payload._cid;
			_signature = payload._signature;
			return *this;
		}
	}
}