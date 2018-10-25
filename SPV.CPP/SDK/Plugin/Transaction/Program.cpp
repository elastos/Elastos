// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.


#include <Core/BRAddress.h>
#include <SDK/Common/ParamChecker.h>
#include <SDK/Common/Log.h>
#include "BRInt.h"

#include "Program.h"
#include "Utils.h"
#include "Transaction.h"

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

		bool Program::isValid(const Transaction *transaction) const {
			if (!_parameter || _parameter.GetSize() <= 0) {
				return false;
			}

			if (!_code || _code.GetSize() <= 0) {
				return false;
			}

			//multi-sign check
			if (_code[_code.GetSize() - 1] == ELA_MULTISIG) {
				uint8_t m, n;
				std::vector<std::string> signers;
				ParseMultiSignRedeemScript(_code, m, n, signers);

				ParamChecker::checkCondition(_parameter.GetSize() % SignatureScriptLength != 0,
											 Error::MultiSign, "Invalid multi sign signatures, length not match");
				ParamChecker::checkCondition(_parameter.GetSize() / SignatureScriptLength < m,
											 Error::MultiSign, "Invalid signatures, not enough signatures");
				ParamChecker::checkCondition(_parameter.GetSize() / SignatureScriptLength > n,
											 Error::MultiSign, "Invalid signatures, too many signatures");

				CMBlock hashData = transaction->GetShaData();
				UInt256 md;
				memcpy(md.u8, hashData, sizeof(UInt256));

				for (int i = 0; i < _parameter.GetSize(); i += SignatureScriptLength) {
					bool verified = false;
					CMBlock signature(SignatureScriptLength);
					memcpy(signature, &_parameter[i], SignatureScriptLength);

					for (std::vector<std::string>::iterator signerIt = signers.begin();
						 signerIt != signers.end(); ++signerIt) {

						verified |= Key::verifyByPublicKey(*signerIt, md, signature);
						if (verified) break;
					}

					ParamChecker::checkCondition(!verified, Error::MultiSign, "Not matched signers.");
				}
			}

			return true;
		}

		bool Program::ParseMultiSignRedeemScript(const CMBlock &code, uint8_t &m, uint8_t &n,
												 std::vector<std::string> &signers) {
			m = code[0] - OP_1 + 1;
			n = code[code.GetSize() - 2] - OP_1 + 1;

			signers.clear();
			for (int i = 1; i < code.GetSize() - 2;) {
				uint8_t size = code[i];
				signers.push_back(Utils::encodeHex(&code[i + 1], size));
				i += size + 1;
			}

			return false;
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