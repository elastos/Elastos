// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef __ELASTOS_SDK_ICLONABLE_H__
#define __ELASTOS_SDK_ICLONABLE_H__

namespace Elastos {
	namespace ElaWallet {

		template <class T>
		class IClonable {
		public:
			virtual ~IClonable() {}

			virtual T *Clone(const BRMerkleBlock *block, bool manageRaw) const = 0;
		};

	}
}

#endif //__ELASTOS_SDK_ICLONABLE_H__
