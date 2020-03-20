// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "Transaction.h"
#include "Program.h"

#include <Common/ErrorChecker.h>
#include <Common/Log.h>
#include <Common/Utils.h>
#include <WalletCore/Address.h>
#include <WalletCore/Key.h>

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

			std::vector<bytes_t> publicKeys;
			SignType type = DecodePublicKey(publicKeys);
			if (type == SignTypeInvalid) {
				Log::error("Invalid Redeem script");
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
					Log::info("Signature not enough for multi sign tx");
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
			std::vector<bytes_t> publicKeys;
			SignType type = DecodePublicKey(publicKeys);
			if (type == SignTypeInvalid) {
				Log::warn("Can not decode pubkey from program");
				return info;
			}

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

		SignType Program::DecodePublicKey(std::vector<bytes_t> &pubkeys) const {
			if (_code.size() < 33 + 2)
				return SignTypeInvalid;

			SignType signType = SignType(_code[_code.size() - 1]);
			bytes_t pubKey;

			ByteStream stream(_code);

			if (signType == SignTypeMultiSign || signType == SignTypeCrossChain) {
				stream.Skip(1);
			} else if (signType != SignTypeStandard) {
				Log::error("unsupport sign type");
				return SignTypeInvalid;
			}

			while (stream.ReadVarBytes(pubKey)) {
				pubkeys.push_back(pubKey);
			}

			return signType;
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

			if (_parameter.empty()) {
				if (SignType(_code.back()) == SignTypeMultiSign) {
					uint8_t m = (uint8_t)(_code[0] - OP_1 + 1);
					uint64_t signLen = m * 64ul;
					size += stream.WriteVarUint(signLen);
					size += signLen;
				} else if (SignType(_code.back()) == SignTypeStandard) {
					size += 65;
				}
			} else {
				size += stream.WriteVarUint(_parameter.size());
				size += _parameter.size();
			}

			size += stream.WriteVarUint(_code.size());
			size += _code.size();

			return size;
		}

		void Program::Serialize(ByteStream &ostream, bool extend) const {
			ostream.WriteVarBytes(_parameter);
			ostream.WriteVarBytes(_code);
			if (extend) {
				ostream.WriteVarString(_path);
			}
		}

		bool Program::Deserialize(const ByteStream &istream, bool extend) {
			if (!istream.ReadVarBytes(_parameter)) {
				Log::error("Program deserialize parameter fail");
				return false;
			}

			if (!istream.ReadVarBytes(_code)) {
				Log::error("Program deserialize code fail");
				return false;
			}

			if (extend) {
				if (!istream.ReadVarString(_path)) {
					Log::error("Program deserialize path fail");
					return false;
				}
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

		void Program::FromJson(const nlohmann::json &j) {

			_path = j.find("Path") != j.end() ? j["Path"].get<std::string>() : "";

			_parameter.setHex(j["Parameter"].get<std::string>());
			_code.setHex(j["Code"].get<std::string>());
		}

	}
}