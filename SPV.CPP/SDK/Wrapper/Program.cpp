// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.


#include "BRInt.h"

#include "Program.h"

namespace Elastos {
	namespace SDK {

		Program::Program() {

		}

		Program::Program(const CMBlock &code, const CMBlock &parameter) :
				_parameter(parameter),
				_code(code) {

		}

		Program::~Program() {
		}

		void Program::Serialize(ByteStream &ostream) const {
			ostream.putVarUint(_parameter.GetSize());
			ostream.putBytes(_parameter, _parameter.GetSize());

			ostream.putVarUint(_code.GetSize());
			ostream.putBytes(_code, _code.GetSize());
		}

		void Program::Deserialize(ByteStream &istream) {
			_parameter.Resize(istream.getVarUint());
			istream.getBytes(_parameter, _parameter.GetSize());

			_code.Resize(istream.getVarUint());
			istream.getBytes(_code, _code.GetSize());
		}

		nlohmann::json Program::toJson() {
			nlohmann::json jsonData;

			char *parameter = new char[_parameter.GetSize()];
			memcpy(parameter, _parameter, _parameter.GetSize());
			std::string parameterStr(parameter, _parameter.GetSize());
			jsonData["parameter"] = parameterStr;

			char *code = new char[_code.GetSize()];
			memcpy(code, _code, _code.GetSize());
			std::string codeStr(code, _code.GetSize());
			jsonData["code"] = codeStr;

			return jsonData;
		}

		void Program::fromJson(nlohmann::json jsonData) {
			std::string parameter = jsonData["parameter"].get<std::string>();
			_parameter.Resize(parameter.size());
			memcpy(_parameter, parameter.data(), parameter.size());

			std::string codeStr = jsonData["code"].get<std::string>();
			_code.Resize(codeStr.size());
			memcpy(_code, codeStr.data(), codeStr.size());
		}

	}
}