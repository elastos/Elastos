// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef __ELASTOS_SDK_ELAMESSAGESERIALIZABLE_H__
#define __ELASTOS_SDK_ELAMESSAGESERIALIZABLE_H__

#include <istream>
#include <ostream>

namespace Elastos {
	namespace SDK {

		class ELAMessageSerializable {
		public:
			virtual ~ELAMessageSerializable();

			virtual void Serialize(std::istream &istream) const = 0;
			virtual void Deserialize(std::ostream &ostream) = 0;
		};

	}
}

#endif //__ELASTOS_SDK_ELAMESSAGESERIALIZABLE_H__
