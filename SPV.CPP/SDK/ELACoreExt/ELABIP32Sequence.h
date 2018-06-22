// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.


#ifndef __ELASTOS_SDK_ELABIP32SEQUENCE_H
#define __ELASTOS_SDK_ELABIP32SEQUENCE_H

#include "BRKey.h"
#include "BRInt.h"
#include "BRBIP32Sequence.h"

#include "CMemBlock.h"

namespace Elastos {
	namespace ElaWallet {

		class ELABIP32Sequence {
		public:
			// key used for authenticated API calls, i.e. bitauth: https://github.com/bitpay/bitauth - path m/1H/0
			static void BIP32APIAuthKey(const BRKey *key, const CMBlock &seed);

			static void BIP32PrivKey(BRKey *key, const CMBlock &seed, uint32_t chain, uint32_t index);

			// sets the private key for the specified path to key
			// depth is the number of arguments used to specify the path
			static void BIP32PrivKeyPath(const BRKey *key, const CMBlock &seed, int depth, ...);

			// sets the private key for the path specified by vlist to key
			// depth is the number of arguments in vlist
			static void BIP32vPrivKeyPath(const BRKey *key, const CMBlock &seed, int depth, va_list vlist);
		};
	}
}

#endif //__ELASTOS_SDK_ELABIP32SEQUENCE_H
