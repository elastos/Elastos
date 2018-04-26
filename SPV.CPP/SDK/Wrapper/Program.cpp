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

		void Program::Serialize(ByteStream &ostream) const {
			uint8_t parameterLengthData[64 / 8];
			UInt64SetLE(parameterLengthData, _parameter.length);
			ostream.putBytes(parameterLengthData, sizeof(parameterLengthData));

			ostream.putBytes(_parameter.data, _parameter.length);

			uint8_t attributeLengthData[64 / 8];
			UInt64SetLE(attributeLengthData, _code.length);
			ostream.putBytes(attributeLengthData, sizeof(attributeLengthData));

			ostream.putBytes(_code.data, _code.length);
		}

		void Program::Deserialize(ByteStream &istream) {
			uint8_t parameterLengthData[64 / 8];
			istream.getBytes(parameterLengthData, sizeof(parameterLengthData));
			_parameter.length = UInt64GetLE(parameterLengthData);

			_parameter.data = new uint8_t[_parameter.length];
			istream.getBytes(_parameter.data, _parameter.length);

			uint8_t attributeLengthData[64 / 8];
			istream.getBytes(attributeLengthData, sizeof(attributeLengthData));
			_code.length = UInt64GetLE(attributeLengthData);

			_code.data = new uint8_t[_code.length];
			istream.getBytes(_code.data, _code.length);
		}


	}
}