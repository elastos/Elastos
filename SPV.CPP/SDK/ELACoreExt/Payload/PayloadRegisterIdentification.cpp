// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

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

			ostream.putVarUint(_id.size());
			ostream.putBytes((const uint8_t *) _id.c_str(), _id.size());

			ostream.putVarUint(_path.size());
			ostream.putBytes((const uint8_t *) _path.c_str(), _path.size());

			uint8_t dataHashData[256 / 8];
			UInt256Set(dataHashData, _dataHash);
			ostream.putBytes(dataHashData, 256 / 8);

			ostream.putVarUint(_proof.GetSize());
			if (_proof.GetSize() != 0)
				ostream.putBytes((uint8_t *)_proof, _proof.GetSize());

			ostream.putVarUint(_sign.GetSize());
			ostream.putBytes((uint8_t *)_sign, _sign.GetSize());
		}

		bool PayloadRegisterIdentification::Deserialize(ByteStream &istream) {
			uint64_t idLen = istream.getVarUint();
			assert(idLen > 0);
			char *idData = new char[idLen];
			istream.getBytes((uint8_t *) idData, idLen);
			_id = std::string(idData, idLen);

			uint64_t pathLen = istream.getVarUint();
			assert(pathLen > 0);
			char *pathData = new char[pathLen];
			istream.getBytes((uint8_t *) pathData, pathLen);
			_path = std::string(pathData, pathLen);

			uint8_t dataHashData[256 / 8];
			istream.getBytes(dataHashData, 256 / 8);
			UInt256Get(&_dataHash, dataHashData);

			uint64_t proofLen = istream.getVarUint();
			if (proofLen > 0) {
				_proof = CMBlock(proofLen);
				istream.getBytes(_proof, proofLen);
			}

			uint64_t signLen = istream.getVarUint();
			assert(signLen > 0);
			_sign = CMBlock(signLen);
			istream.getBytes(_sign, signLen);
		}

		nlohmann::json PayloadRegisterIdentification::toJson() {
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