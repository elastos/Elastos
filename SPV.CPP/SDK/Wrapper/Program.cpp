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
		}

		void Program::Serialize(ByteStream &ostream) const {
			ostream.putVarUint(_parameter.length);
			ostream.putBytes(_parameter.data, _parameter.length);

			ostream.putVarUint(_code.length);
			ostream.putBytes(_code.data, _code.length);
		}

		void Program::Deserialize(ByteStream &istream) {
			_parameter.length = istream.getVarUint();
			_parameter.data = new uint8_t[_parameter.length];
			istream.getBytes(_parameter.data, _parameter.length);

			_code.length = istream.getVarUint();
			_code.data = new uint8_t[_code.length];
			istream.getBytes(_code.data, _code.length);
		}


	}
}