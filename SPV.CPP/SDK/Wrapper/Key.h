// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef __ELASTOS_SDK_KEY_H__
#define __ELASTOS_SDK_KEY_H__

#include <boost/shared_ptr.hpp>

#include "BRKey.h"

#include "Wrapper.h"

namespace Elastos {
	namespace SDK {

		class Key :
			public Wrapper<BRKey *> {
		public:

			virtual std::string toString() const;

			virtual BRKey * getRaw() const;
		};

		typedef boost::shared_ptr<Key> KeyPtr;

	}
}

#endif //__ELASTOS_SDK_KEY_H__
