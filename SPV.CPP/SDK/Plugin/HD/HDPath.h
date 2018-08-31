// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef __ELASTOS_SDK_HDPATH_H__
#define __ELASTOS_SDK_HDPATH_H__

#include "../Interface/IHDPath.h"

namespace Elastos {
	namespace ElaWallet {

		class HDPath : public IHDPath {
		public:
			HDPath();

			virtual Key CalculateSubWalletMasterKey(const UInt512 &seed, int coinIndex, UInt256 &chainCode);

			virtual std::string GetHDPathType() const;

			virtual IHDPath *CreateHDPath() const;
		};

	}
}

#endif //__ELASTOS_SDK_HDPATH_H__
