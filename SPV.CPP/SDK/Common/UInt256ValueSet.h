// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef __ELASTOS_SDK_UINT256SET_H__
#define __ELASTOS_SDK_UINT256SET_H__

#include <set>

#include "BRInt.h"

namespace Elastos {
	namespace ElaWallet {

		class UInt256ValueSet {
		public:
			struct UInt256Compare {
				bool operator()(const UInt256 &first, const UInt256 &second) const {
					return UInt256LessThan(&first, &second) == 1;
				}
			};

		public:
			void Insert(const UInt256 &value);

			bool Contains(const UInt256 &value) const;

		private:
			std::set<UInt256, UInt256Compare> _set;
		};

	}
}


#endif //__ELASTOS_SDK_UINT256SET_H__
