// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef __ELASTOS_SDK_IHDPATH_H__
#define __ELASTOS_SDK_IHDPATH_H__

#include <boost/shared_ptr.hpp>

#include "BRInt.h"

#include "Key.h"

namespace Elastos {
	namespace ElaWallet {

		class IHDPath {
		public:
			virtual ~IHDPath() {}

			virtual IHDPath *CreateHDPath() const = 0;

			virtual std::string GetHDPathType() const = 0;

			virtual Key CalculateSubWalletMasterKey(const UInt512 &seed, int coinIndex, UInt256 &chainCode) = 0;
		};

		typedef boost::shared_ptr<IHDPath> HDPathPtr;
	}
}


#endif //__ELASTOS_SDK_IHDPATH_H__
