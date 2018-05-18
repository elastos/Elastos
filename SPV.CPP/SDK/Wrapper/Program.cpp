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


	}
}