// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef __ELASTOS_SDK_PROGRAM_H__
#define __ELASTOS_SDK_PROGRAM_H__

#include <boost/shared_ptr.hpp>

#include "CMemBlock.h"
#include "ELAMessageSerializable.h"

namespace Elastos {
	namespace SDK {

		class Program :
			public ELAMessageSerializable {
		public:
			Program();

			Program(const CMBlock &code, const CMBlock &parameter);

			~Program();

			bool isValid();

			virtual void Serialize(ByteStream &ostream) const;

			virtual void Deserialize(ByteStream &istream);

			virtual nlohmann::json toJson();

			virtual void fromJson(const nlohmann::json &jsonData);

		private:
			CMBlock _code;
			CMBlock _parameter;
		};

		typedef boost::shared_ptr<Program> ProgramPtr;

	}
}

#endif //__ELASTOS_SDK_PROGRAM_H__
