// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.


#include "BRInt.h"

#include "Program.h"

namespace Elastos {
	namespace SDK {

		Program::Program() {

		}

		Program::Program(const ByteData &code, const ByteData &parameter) :
			_parameter(parameter),
			_code(code) {

		}

		Program::~Program() {
			if (_code.data != nullptr) {
				delete[](_code.data);
			}
			if (_parameter.data != nullptr) {
				delete[](_parameter.data);
			}
		}

		void Program::Serialize(std::istream &istream) const {
			uint8_t parameterLengthData[64 / 8];
			UInt64SetLE(parameterLengthData, _parameter.length);
			istream >> parameterLengthData;

			istream >> _parameter.data;

			uint8_t attributeLengthData[64 / 8];
			UInt64SetLE(attributeLengthData, _code.length);
			istream >> attributeLengthData;

			istream >> _code.data;
		}

		void Program::Deserialize(std::ostream &ostream) {
			uint8_t parameterLengthData[64 / 8];
			ostream << parameterLengthData;
			_parameter.length = UInt64GetLE(parameterLengthData);

			ostream << _parameter.data;

			uint8_t attributeLengthData[64 / 8];
			ostream << attributeLengthData;
			_code.length = UInt64GetLE(attributeLengthData);

			ostream << _code.data;
		}


	}
}