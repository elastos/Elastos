// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.


#include "BRInt.h"

#include "Program.h"
#include "Utils.h"

namespace Elastos {
	namespace ElaWallet {

		Program::Program() {
			_code.Resize(0);
			_parameter.Resize(0);
		}

		Program::Program(const Program &program) {
			ByteStream stream;
			program.Serialize(stream);
			stream.setPosition(0);
			this->Deserialize(stream);
		}

		Program::Program(const CMBlock &code, const CMBlock &parameter) :
				_parameter(parameter),
				_code(code) {

		}

		Program::~Program() {
		}

		bool Program::isValid() {
			if (!_parameter || _parameter.GetSize() <= 0) {
				return false;
			}

			if (!_code || _code.GetSize() <= 0) {
				return false;
			}
			return true;
		}

		const CMBlock &Program::getCode() {
			return _code;
		}

		const CMBlock &Program::getParameter()
		{
			return _parameter;
		}

		void Program::setCode(const CMBlock &code) {
			_code = code;
		}

		void Program::setParameter(const CMBlock &parameter) {
			_parameter = parameter;
		}

		void Program::Serialize(ByteStream &ostream) const {
			ostream.putVarUint(_parameter.GetSize());
			ostream.putBytes(_parameter, _parameter.GetSize());

			ostream.putVarUint(_code.GetSize());
			ostream.putBytes(_code, _code.GetSize());
		}

		bool Program::Deserialize(ByteStream &istream) {
			_parameter.Resize(size_t(istream.getVarUint()));
			istream.getBytes(_parameter, _parameter.GetSize());

			_code.Resize(size_t(istream.getVarUint()));
			istream.getBytes(_code, _code.GetSize());

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