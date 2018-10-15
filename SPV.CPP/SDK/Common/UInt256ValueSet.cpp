// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "UInt256ValueSet.h"

namespace Elastos {
	namespace ElaWallet {


		void UInt256ValueSet::Insert(const UInt256 &value) {
			_set.insert(value);
		}

		bool UInt256ValueSet::Contains(const UInt256 &value) const {
			return _set.end() != std::find_if(_set.begin(), _set.end(),
											  [&value](const UInt256 &item) { return 0 != UInt256Eq(&value, &item); });
		}
	}
}

