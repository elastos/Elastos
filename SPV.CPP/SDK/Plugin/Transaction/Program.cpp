// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "Transaction.h"
#include "Program.h"

#include <SDK/Common/ParamChecker.h>
#include <SDK/Common/Log.h>
#include <SDK/Common/Utils.h>

#include <Core/BRAddress.h>
#include <Core/BRInt.h>

#define SignatureScriptLength 65

namespace Elastos {
	namespace ElaWallet {

		Program::Program() {
			_code.Resize(0);
			_parameter.Resize(0);
		}

		Program::Program(const Program &program) {
			this->_code.Resize(program._code.GetSize());
			memcpy(this->_code, program._code, program._code.GetSize());

			this->_parameter.Resize(program._parameter.GetSize());
			memcpy(this->_parameter, program._parameter, program._parameter.GetSize());
		}

		Program::Program(const CMBlock &code, const CMBlock &parameter) :
				_parameter(parameter),
				_code(code) {

		}

		Program::~Program() {
		}

		bool Program::VerifySignature(const UInt256 &md) const {
			Key key;
			uint8_t signatureCount = 0;

			std::vector<CMBlock> publicKeys = DecodePublicKey();
			if (publicKeys.empty()) {
				Log::error("Redeem script without public key");
				return false;
			}

			ByteStream stream(_parameter);
			CMBlock signature;
			while (stream.readVarBytes(signature)) {
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

			if (SignType(_code[_code.GetSize() - 1]) == SignTypeMultiSign) {
				uint8_t m = (uint8_t)(_code[0] - OP_1 + 1);
				uint8_t n = (uint8_t)(_code[_code.GetSize() - 2] - OP_1 + 1);

				if (signatureCount < m) {
					Log::error("Signature not enough for multi sign");
					return false;
				}

				if (publicKeys.size() > n) {
					Log::error("Too many signers");
					return false;
				}
			} else if (SignType(_code[_code.GetSize() - 1]) == SignTypeStandard) {
				if (publicKeys.size() != signatureCount) {
					return false;
				}
			}

			return true;
		}

		nlohmann::json Program::GetSignedInfo(const UInt256 &md) const {
			nlohmann::json info;
			std::vector<CMBlock> publicKeys = DecodePublicKey();
			if (publicKeys.empty())
				return info;

			Key key;
			ByteStream stream(_parameter);
			CMBlock signature;
			nlohmann::json signers;
			while (stream.readVarBytes(signature)) {
				for (size_t i = 0; i < publicKeys.size(); ++i) {
					key.SetPubKey(publicKeys[i]);
					if (key.Verify(md, signature)) {
						signers.push_back(Utils::encodeHex(publicKeys[i]));
						break;
					}
				}
			}

			if (SignType(_code[_code.GetSize() - 1]) == SignTypeMultiSign) {
				uint8_t m = (uint8_t)(_code[0] - OP_1 + 1);
				uint8_t n = (uint8_t)(_code[_code.GetSize() - 2] - OP_1 + 1);
				info["SignType"] = "MultiSign";
				info["M"] = m;
				info["N"] = n;
				info["Signers"] = signers;
			} else if (SignType(_code[_code.GetSize() - 1]) == SignTypeStandard) {
				info["SignType"] = "Standard";
				info["Signers"] = signers;
			}

			return info;
		}

		std::vector<CMBlock> Program::DecodePublicKey() const {
			std::vector<CMBlock> publicKeys;
			if (_code.GetSize() < 33 + 2)
				return publicKeys;

			SignType signType = SignType(_code[_code.GetSize() - 1]);
			CMBlock pubKey;

			ByteStream stream(_code);

			if (signType == SignTypeMultiSign) {
				stream.drop(1);
			} else if (signType != SignTypeStandard) {
				Log::error("unsupport sign type");
				return publicKeys;
			}

			while (stream.readVarBytes(pubKey)) {
				publicKeys.push_back(pubKey);
			}

			return publicKeys;
		}

		const CMBlock &Program::getCode() const {
			return _code;
		}

		const CMBlock &Program::getParameter() const {
			return _parameter;
		}

		void Program::setCode(const CMBlock &code) {
			_code = code;
		}

		void Program::setParameter(const CMBlock &parameter) {
			_parameter = parameter;
		}

		void Program::Serialize(ByteStream &ostream) const {
			ostream.writeVarBytes(_parameter);
			ostream.writeVarBytes(_code);
		}

		bool Program::Deserialize(ByteStream &istream) {
			if (!istream.readVarBytes(_parameter)) {
				Log::error("Program deserialize parameter fail");
				return false;
			}

			if (!istream.readVarBytes(_code)) {
				Log::error("Program deserialize code fail");
				return false;
			}

			return true;
		}

		nlohmann::json Program::toJson() const {
			nlohmann::json jsonData;

			jsonData["Parameter"] = Utils::encodeHex(_parameter);
			jsonData["Code"] = Utils::encodeHex(_code);

			return jsonData;
		}

		void Program::fromJson(const nlohmann::json &jsonData) {
			_parameter = Utils::decodeHex(jsonData["Parameter"].get<std::string>());
			_code = Utils::decodeHex(jsonData["Code"].get<std::string>());
		}

	}
}