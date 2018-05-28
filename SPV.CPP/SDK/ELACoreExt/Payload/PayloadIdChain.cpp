// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "PayloadIdChain.h"

namespace Elastos {
	namespace SDK {

		PayloadIdChain::PayloadIdChain() :
				_id(""),
				_path(""),
				_dataHash(""),
				_proof(""),
				_sign("") {

		}

		PayloadIdChain::~PayloadIdChain() {

		}

		const std::string &PayloadIdChain::getId() const {
			return _id;
		}

		void PayloadIdChain::setId(const std::string &id) {
			_id = id;
		}

		const std::string &PayloadIdChain::getPath() const {
			return _path;
		}

		void PayloadIdChain::setPath(const std::string &path) {
			_path = path;
		}

		const std::string &PayloadIdChain::getDataHash() const {
			return _dataHash;
		}

		void PayloadIdChain::setDataHash(const std::string &dataHash) {
			_dataHash = dataHash;
		}

		const std::string &PayloadIdChain::getProof() const {
			return _proof;
		}

		void PayloadIdChain::setProof(const std::string &proof) {
			_proof = proof;
		}

		const std::string &PayloadIdChain::getSign() const {
			return _sign;
		}

		void PayloadIdChain::setSign(const std::string &sign) {
			_sign = sign;
		}

		void PayloadIdChain::Serialize(ByteStream &ostream) const {

			assert(!_id.empty());
			assert(!_path.empty());
			assert(!_dataHash.empty());
			assert(!_sign.empty());

			ostream.putVarUint(_id.size());
			ostream.putBytes((const uint8_t *) _id.c_str(), _id.size());

			ostream.putVarUint(_path.size());
			ostream.putBytes((const uint8_t *) _path.c_str(), _path.size());

			ostream.putVarUint(_dataHash.size());
			ostream.putBytes((const uint8_t *) _dataHash.c_str(), _dataHash.size());

			ostream.putVarUint(_proof.size());
			if (!_proof.empty())
				ostream.putBytes((const uint8_t *) _proof.c_str(), _proof.size());

			ostream.putVarUint(_sign.size());
			ostream.putBytes((const uint8_t *) _sign.c_str(), _sign.size());
		}

		void PayloadIdChain::Deserialize(ByteStream &istream) {
			uint64_t idLen = istream.getVarUint();
			assert(idLen > 0);
			char *idData = new char[idLen];
			istream.getBytes((uint8_t *) idData, idLen);
			_id = std::string(idData, idLen);

			uint64_t pathLen = istream.getVarUint();
			assert(pathLen > 0);
			char *pathData = new char[pathLen];
			istream.getBytes((uint8_t *) pathData, pathLen);
			_id = std::string(pathData, pathLen);

			uint64_t dataHashLen = istream.getVarUint();
			assert(dataHashLen > 0);
			char *dataHashData = new char[dataHashLen];
			istream.getBytes((uint8_t *) dataHashData, dataHashLen);
			_id = std::string(dataHashData, dataHashLen);

			uint64_t proofLen = istream.getVarUint();
			if (proofLen > 0) {
				char *proofData = new char[proofLen];
				istream.getBytes((uint8_t *) proofData, proofLen);
				_id = std::string(proofData, proofLen);
			}

			uint64_t signLen = istream.getVarUint();
			assert(signLen > 0);
			char *signData = new char[signLen];
			istream.getBytes((uint8_t *) signData, signLen);
			_id = std::string(signData, signLen);
		}

		nlohmann::json PayloadIdChain::toJson() {
			nlohmann::json j;
			j["Id"] = _id;
			j["Path"] = _path;
			j["DataHash"] = _dataHash;
			j["Proof"] = _proof;
			j["Sign"] = _sign;
			return j;
		}

		void PayloadIdChain::fromJson(const nlohmann::json &j) {
			_id = j["Id"].get<std::string>();
			_path = j["Path"].get<std::string>();
			_dataHash = j["DataHash"].get<std::string>();
			_proof = j["Proof"].get<std::string>();
			_sign = j["Sign"].get<std::string>();
		}
	}
}