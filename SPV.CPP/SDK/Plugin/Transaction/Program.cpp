// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "Transaction.h"
#include "Program.h"

#include <SDK/Common/ErrorChecker.h>
#include <SDK/Common/Log.h>
#include <SDK/Common/Utils.h>

namespace Elastos {
	namespace ElaWallet {

		Program::Program() {
		}

		Program::Program(const Program &program) {
			operator=(program);
		}

		Program::Program(const std::string &path, const bytes_t &code, const bytes_t &parameter) :
				_path(path),
				_parameter(parameter),
				_code(code) {

		}

		Program::~Program() {
		}

		Program &Program::operator=(const Program &p) {
			_path = p._path;
			_code = p._code;
			_parameter = p._parameter;
			return *this;
		}

		bool Program::VerifySignature(const uint256 &md) const {
			Key key;
			uint8_t signatureCount = 0;

			std::vector<bytes_t> publicKeys = DecodePublicKey();
			if (publicKeys.empty()) {
				Log::error("Redeem script without public key");
				return false;
			}

			ByteStream stream(_parameter);
			bytes_t signature;
			while (stream.ReadVarBytes(signature)) {
				bool verified = false;
				for (size_t i = 0; i < publicKeys.size(); ++i) {
					key.SetPubKey(publicKeys[i]);
					if (key.Verify(md, signature)) {
						verified = true;
						break;
					}
				}

				signatureCount++;
				if (!verified) {
					Log::error("Transaction signature verify failed");
					return false;
				}
			}

			if (SignType(_code.back()) == SignTypeMultiSign) {
				uint8_t m = (uint8_t)(_code[0] - OP_1 + 1);
				uint8_t n = (uint8_t)(_code[_code.size() - 2] - OP_1 + 1);

				if (signatureCount < m) {
					Log::error("Signature not enough for multi sign");
					return false;
				}

				if (publicKeys.size() > n) {
					Log::error("Too many signers");
					return false;
				}
			} else if (SignType(_code.back()) == SignTypeStandard) {
				if (publicKeys.size() != signatureCount) {
					return false;
				}
			}

			return true;
		}

		nlohmann::json Program::GetSignedInfo(const uint256 &md) const {
			nlohmann::json info;
			std::vector<bytes_t> publicKeys = DecodePublicKey();
			if (publicKeys.empty())
				return info;

			Key key;
			ByteStream stream(_parameter);
			bytes_t signature;
			nlohmann::json signers;
			while (stream.ReadVarBytes(signature)) {
				for (size_t i = 0; i < publicKeys.size(); ++i) {
					key.SetPubKey(publicKeys[i]);
					if (key.Verify(md, signature)) {
						signers.push_back(publicKeys[i].getHex());
						break;
					}
				}
			}

			if (SignType(_code.back()) == SignTypeMultiSign) {
				uint8_t m = (uint8_t)(_code[0] - OP_1 + 1);
				uint8_t n = (uint8_t)(_code[_code.size() - 2] - OP_1 + 1);
				info["SignType"] = "MultiSign";
				info["M"] = m;
				info["N"] = n;
				info["Signers"] = signers;
			} else if (SignType(_code.back()) == SignTypeStandard) {
				info["SignType"] = "Standard";
				info["Signers"] = signers;
			}

			return info;
		}

		std::vector<bytes_t> Program::DecodePublicKey() const {
			std::vector<bytes_t> publicKeys;
			if (_code.size() < 33 + 2)
				return publicKeys;

			SignType signType = SignType(_code[_code.size() - 1]);
			bytes_t pubKey;

			ByteStream stream(_code);

			if (signType == SignTypeMultiSign || signType == SignTypeCrossChain) {
				stream.Skip(1);
			} else if (signType != SignTypeStandard) {
				Log::error("unsupport sign type");
				return publicKeys;
			}

			while (stream.ReadVarBytes(pubKey)) {
				publicKeys.push_back(pubKey);
			}

			return publicKeys;
		}

		const bytes_t &Program::GetCode() const {
			return _code;
		}

		const bytes_t &Program::GetParameter() const {
			return _parameter;
		}

		void Program::SetCode(const bytes_t &code) {
			_code = code;
		}

		void Program::SetParameter(const bytes_t &parameter) {
			_parameter = parameter;
		}

		void Program::SetPath(const std::string &path) {
			_path = path;
		}

		const std::string &Program::GetPath() const {
			return _path;
		}

		size_t Program::EstimateSize() const {
			size_t size = 0;
			ByteStream stream;

			size += stream.WriteVarUint(_parameter.size());
			size += _parameter.size();
			size += stream.WriteVarUint(_code.size());
			size += _code.size();

			return size;
		}

		void Program::Serialize(ByteStream &ostream) const {
			ostream.WriteVarBytes(_parameter);
			ostream.WriteVarBytes(_code);
		}

		bool Program::Deserialize(const ByteStream &istream) {
			if (!istream.ReadVarBytes(_parameter)) {
				Log::error("Program deserialize parameter fail");
				return false;
			}

			if (!istream.ReadVarBytes(_code)) {
				Log::error("Program deserialize code fail");
				return false;
			}

			return true;
		}

		nlohmann::json Program::ToJson() const {
			nlohmann::json jsonData;

			jsonData["Parameter"] = _parameter.getHex();
			jsonData["Code"] = _code.getHex();
			jsonData["Path"] = _path;

			return jsonData;
		}

		void Program::FromJson(const nlohmann::json &jsonData) {

			_path = jsonData.find("Path") != jsonData.end() ? jsonData["Path"].get<std::string>() : "";

			_parameter.setHex(jsonData["Parameter"].get<std::string>());
			_code.setHex(jsonData["Code"].get<std::string>());
		}

	}
}