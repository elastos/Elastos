// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <SDK/Common/Log.h>
#include "PayloadRegisterIdentification.h"
#include "Utils.h"

namespace Elastos {
	namespace ElaWallet {

		PayloadRegisterIdentification::PayloadRegisterIdentification() :
				_id(""),
				_path("") {

		}

		PayloadRegisterIdentification::~PayloadRegisterIdentification() {

		}

		const std::string &PayloadRegisterIdentification::getId() const {
			return _id;
		}

		void PayloadRegisterIdentification::setId(const std::string &id) {
			_id = id;
		}

		const std::string &PayloadRegisterIdentification::getPath() const {
			return _path;
		}

		void PayloadRegisterIdentification::setPath(const std::string &path) {
			_path = path;
		}

		const UInt256 &PayloadRegisterIdentification::getDataHash() const {
			return _dataHash;
		}

		void PayloadRegisterIdentification::setDataHash(const UInt256 &dataHash) {
			_dataHash = dataHash;
		}

		const CMBlock &PayloadRegisterIdentification::getProof() const {
			return _proof;
		}

		void PayloadRegisterIdentification::setProof(const CMBlock &proof) {
			_proof = proof;
		}

		const CMBlock &PayloadRegisterIdentification::getSign() const {
			return _sign;
		}

		void PayloadRegisterIdentification::setSign(const CMBlock &sign) {
			_sign = sign;
		}

		bool PayloadRegisterIdentification::isValid() const {
			if (_id.empty() || _path.empty() || _sign.GetSize() <= 0) {
				return false;
			}
			return true;
		}

		void PayloadRegisterIdentification::Serialize(ByteStream &ostream) const {

			assert(!_id.empty());
			assert(!_path.empty());
			assert(_sign.GetSize() > 0);

			ostream.writeVarString(_id);
			ostream.writeVarString(_path);
			ostream.writeBytes(_dataHash.u8, sizeof(_dataHash));
			ostream.writeVarBytes(_proof);
			ostream.writeVarBytes(_sign);
		}

		bool PayloadRegisterIdentification::Deserialize(ByteStream &istream) {
			if (!istream.readVarString(_id)) {
				Log::error("Payload register identification deserialize id fail");
				return false;
			}

			if (!istream.readVarString(_path)) {
				Log::error("Payload register identification deserialize path fail");
				return false;
			}

			if (!istream.readBytes(_dataHash.u8, sizeof(_dataHash))) {
				Log::error("Payload register identification deserialize data hash fail");
				return false;
			}

			if (!istream.readVarBytes(_proof)) {
				Log::error("Payload register identification deserialize proof fail");
				return false;
			}

			if (!istream.readVarBytes(_sign)) {
				Log::error("Payload register identification deserialize sign fail");
				return false;
			}

			return true;
		}

		nlohmann::json PayloadRegisterIdentification::toJson() const {
			nlohmann::json j;
			j["Id"] = _id;
			j["Path"] = _path;
			j["DataHash"] = Utils::UInt256ToString(_dataHash);
			j["Proof"] = Utils::encodeHex(_proof);
			j["Sign"] = Utils::encodeHex(_sign);
			return j;
		}

		void PayloadRegisterIdentification::fromJson(const nlohmann::json &j) {
			_id = j["Id"].get<std::string>();
			_path = j["Path"].get<std::string>();
			_dataHash = Utils::UInt256FromString(j["DataHash"].get<std::string>());
			_proof = Utils::decodeHex(j["Proof"].get<std::string>());
			_sign = Utils::decodeHex(j["Sign"].get<std::string>());
		}
	}
}